#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "./custom_w/qimagewidjet.h"
#include <QDebug>
#include <QWebFrame>
#include <QWebElement>
#include <QMessageBox>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <QSqlQuery>
#include <QInputDialog>
#include <QListWidgetItem>
#include <vector>
#include <QPushButton>
#include <QFileDialog>
#include "./custom_w/taglist.h"
// Конструктур класса добавляемого в JSON запрос карты, для доступа к маркерам Google Maps из программы по коннекту
MyClass::MyClass(  QObject *parent )
  : QObject( parent )
{
}


// наследование и перегрузка функций рисовки класса QItemDelegate - строк QFileSystemModel. Зеленый окрас для изобрадений имеющихся в базе данных и в списке тегов, а желтый для тех, которые только в тегах
class GreenDelegate: public QItemDelegate
{
public:
GreenDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

public:
virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//drawBackground(painter, option, index);
painter->fillRect(option.rect,QColor(0,200,0));
QItemDelegate::paint(painter, option, index);
}

protected:
virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
painter->fillRect(option.rect, QColor(0, 200, 0));
}
};

class YellowDelegate: public QItemDelegate
{
public:
YellowDelegate(QObject *parent = 0) : QItemDelegate(parent) {}

public:
virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
drawBackground(painter, option, index);
QItemDelegate::paint(painter, option, index);
}

protected:
virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
painter->fillRect(option.rect, QColor(250, 200, 0));
}
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    imageExts << "bmp" << "gif" << "jpg" << "png" << "pbm" << "pgm" << "ppm" << "xbm" << "xpm" << "svg" << "dds" << "icns" << "jp2" << "mng" << "tga" << "tiff" << "wbmp" << "webp";
    db = QSqlDatabase::addDatabase("QSQLITE");
    appLoaded=false; // указывает на то, запущено ли приложение. Нужно поскольку порядок строк QFileSystemModel  во время запуска приложения другой, чем при работе программы
    markerMode=false; // переменная указывающая на положение кнопки switcher. В зависимости от положения маркеры рисуются на основании списка изображений выбранного тега или на основании списка файлов открытой папки файл менеджера
    switcher = new QSlideSwitch(this);
    switcher->setCheckable(true);
    getRecords();
    bool ok = dbconnect(records["dbFileName"],false);
    if (!ok)
        bool ok = dbconnect("tmp.sqlite",true);
    dbPhotoTable = "photoList";
    dbTagTable= "Tags";
    ui->setupUi(this);
    model = new QFileSystemModel(this);
    model->setFilter(QDir::AllDirs);
    ui->lvBackUp->setModel(model);
    QImageWidjet* image = new QImageWidjet();
    ui->verticalLayout->addWidget(image);
    switcher->setMinimumSize(20,20);
    ui->verticalLayout_2->addWidget(switcher);
    myLW = new tagListWidget(dbTagTable,dbPhotoTable, db);
    ui->gridLayout->addWidget(myLW,1,2);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->lePostalAddress->setText("");
    ui->webView->setUrl(QUrl("qrc:/html/google_maps.html"));
    //ui->webView->setUrl(QUrl("http://maps.google.com/maps/api/js?sensor=false"));
    this->updateCbTags();
    ui->lvBackUp->setRootIndex(model->setRootPath(records["lastDir"]));
    QTimer::singleShot(2000,this,SLOT(appStarted()));
    //connect(ui->horizontalSlider, SIGNAL(valueChanged(int)),image,SLOT(setMargin(int)));
    connect(this,SIGNAL(fileSend(QString)),image,SLOT(imageLoad(QString)));
    connect(ui->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
    connect(ui->lePostalAddress, SIGNAL(returnPressed()), this, SLOT(goClicked()));
    connect(&m_geocodeDataManager, SIGNAL(coordinatesReady(double,double)), this, SLOT(showCoordinates(double,double)));
    connect(&m_geocodeDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_geocodeDataManager, SIGNAL(addressReady(QString)), ui->lePostalAddress, SLOT(setText(QString)));
    connect(ui->webView,SIGNAL(loadFinished(bool)),this,SLOT(updateLwTags())); // обновляет список тагов после загрузки карты, чтобы сразу отобразились все точки
    connect(this,SIGNAL(colorLv(QModelIndex)),SLOT(colorlvBackUp(QModelIndex)));
    connect(this,SIGNAL(imageAdded()),this,SLOT(updateLwTags()));
    connect(switcher,SIGNAL(clicked(bool)),this,SLOT(changeMapOrientation(bool)));
    connect(ui->actionExit,SIGNAL(triggered(bool)),this,SLOT(close()));
    //connect(model,SIGNAL(directoryLoaded(QString)),this,SLOT(appStarted()));
    connect(switcher,SIGNAL(toggled(bool)),this,SLOT(changeMapOrientation(bool)));
    connect(qApp,SIGNAL(lastWindowClosed()),this,SLOT(closeSlot()));
    connect(this,SIGNAL(directoryChanged()),this,SLOT(placeMarkers()));
    connect(switcher,SIGNAL(clicked(bool)),this,SLOT(placeMarkers()));
    connect(this,SIGNAL(photoPathsSend(std::map<QString,QString>)),myLW,SLOT(photoPathsSet(std::map<QString,QString>)));
    connect(myLW,SIGNAL(fileSend(QString)),image,SLOT(imageLoad(QString)));
    connect(this,SIGNAL(tagChanged(QString)),myLW,SLOT(tagSet(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::dbconnect(QString fileName, bool newFile)
{
    // подключение базы данных, содержащей таблицы photoList и Tags
    if (db.isOpen())
        db.close();
    if (!newFile) {
        if (QFile("tmp.sqlite").exists())
            QFile::remove("tmp.sqlite");
        QFile::copy(fileName,"tmp.sqlite");
    }
    else
        QFile::remove("tmp.sqlite");
    db.setDatabaseName("tmp.sqlite");
    db.open();
    if (newFile) {
        QSqlQuery(QString("CREATE TABLE \"Tags\" (\"Tag\" TEXT, \"Photo\" TEXT, \"PhotoPath\" TEXT)"));
        QSqlQuery(QString("CREATE TABLE \"photoList\" (\"Photo\" VARCHAR, \"Place\" VARCHAR, \"Lat\" FLOAT, \"Lng\" FLOAT, \"Date\" VARCHAR, \"Description\" TEXT, \"PhotoPath\" VARCHAR)"));
        //this->updateLwTags();
    }
    return db.isOpen();
}

void MainWindow::updateCbTags()
{
    if (!tagList.isEmpty()) {
        tagList.clear();
        ui->cbTags->clear();
    }
    QSqlQuery query(QString("SELECT DISTINCT Tag FROM %1").arg(dbTagTable),db);
    while (query.next()) {
             tagList.append(query.value(0).toString());
             qDebug() << query.value(0).toString();
    }
    ui->cbTags->addItems(tagList);
}

bool MainWindow::getRecords()
{
    QFile lastRecFile(recFileName);
    if (lastRecFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&lastRecFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          if (line.contains("dbFileName")) {
              QStringList tmpList=line.split(' ', QString::SkipEmptyParts);
              records["dbFileName"]=tmpList.last();
          }
          if (line.contains("lastDir")) {
              QStringList tmpList=line.split(' ', QString::SkipEmptyParts);
              records["lastDir"]=tmpList.last();
          }
       }
       lastRecFile.close();
    }
    return true;
}

bool MainWindow::saveRecords()
{
    QFile lastRecFile(recFileName);
    if (lastRecFile.open(QIODevice::WriteOnly))
    {
       QTextStream in(&lastRecFile);
       for (std::map<QString,QString>::iterator it=records.begin();it!=records.end();it++) {
       in << it->first << " " << it->second << "\r\n" ;
       }
       lastRecFile.close();
    }
}


void MainWindow::on_lvBackUp_doubleClicked(const QModelIndex &index)
{

    QListView* listView = (QListView*) sender();
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.fileName() == "..") {
        QDir dir = fileInfo.dir();
        dir.cdUp();
        listView->setRootIndex(model->index(dir.absolutePath()));
                model->setRootPath(dir.absolutePath());
                qDebug()<< ui->lvBackUp->rootIndex().data().toString();
            emit(colorLv(ui->lvBackUp->rootIndex()));
                emit(directoryChanged());
    }
    else if (fileInfo.fileName() == ".") {
        listView->setRootIndex(model->index(fileInfo.dir().absolutePath()));
            emit(colorLv(ui->lvBackUp->rootIndex()));
    }
    else if (fileInfo.isDir()) {
        qDebug() <<"dir";
        listView->setRootIndex(index);
        model->setRootPath(fileInfo.absoluteFilePath());
        //qDebug() << ui->lvBackUp->currentIndex().data().toString();
            emit(colorLv(ui->lvBackUp->currentIndex()));
        emit(directoryChanged());

    }
    else if (fileInfo.isFile()) {
        emit fileSend(fileInfo.absoluteFilePath());
    }
    records["lastDir"]=model->rootDirectory().absolutePath();
}



void recursiveContentList(QDir &dir, QFileInfoList &contentList)
{
    foreach(QFileInfo info, dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst)){
        contentList.append(info);
        if (info.isDir() && dir.cd(info.fileName())) {
            recursiveContentList(dir, contentList);
            dir.cdUp();
        }
    }
}


void contentDifference(QDir &sDir, QDir &bDir, QFileInfoList &diffList)
{
    foreach(QFileInfo sInfo, sDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst)) {
        bool fileExists = false;
        foreach(QFileInfo dInfo, bDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::DirsFirst)) {
            if (sInfo.fileName() == dInfo.fileName()) {
                if (sInfo.isDir() || sInfo.lastModified() <= dInfo.lastModified())
                    fileExists = true;
                break;
            }
        }
        if (!fileExists)
            diffList.append(sInfo);
        if (sInfo.isFile())
            continue;
        if (fileExists) {
            sDir.cd(sInfo.fileName());
            bDir.cd(sInfo.fileName());
            contentDifference(sDir,bDir,diffList);
            sDir.cdUp();
            bDir.cdUp();
        }
        else {
            sDir.cd(sInfo.fileName());
            recursiveContentList(sDir, diffList);
            sDir.cdUp();
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
//    QDir sDir = QDir(model->filePath(ui->lvSource->rootIndex()));
//    QDir bDir = QDir(model->filePath(ui->lvBackUp->rootIndex()));

//    QFileInfoList diffList = QFileInfoList();
//    contentDifference(sDir, bDir, diffList);

//    foreach(QFileInfo diffInfo, diffList) {
//        QString backUpPath = diffInfo.filePath().replace(sDir.absolutePath(),bDir.absolutePath());
//        if (diffInfo.isFile()) {
//            QFile::remove(backUpPath);
//            QFile::copy(diffInfo.absoluteFilePath(),backUpPath);
//        }
//        if (diffInfo.isDir()) {
//            bDir.mkdir(backUpPath);
//        }
//    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Right) {
        QKeyEvent *eventVirt = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
        QCoreApplication::sendEvent (ui->lvBackUp, eventVirt);
       // QModelIndex index1 = ui->lvBackUp->indexAt(QPoint(0,0));
       // ui->label->setText();
        //ui->lvBackUp->setFocus(Qt::FastTransformation);
        QFileInfo fileInfo = model->fileInfo(ui->lvBackUp->currentIndex());
        emit fileSend(fileInfo.absoluteFilePath());
    }
    else if (event->key() == Qt::Key_Left) {
        QKeyEvent *eventVirt = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
        QCoreApplication::sendEvent (ui->lvBackUp, eventVirt);
       // QModelIndex index1 = ui->lvBackUp->indexAt(QPoint(0,0));
       // ui->label->setText();
        //ui->lvBackUp->setFocus(Qt::FastTransformation);
        QFileInfo fileInfo = model->fileInfo(ui->lvBackUp->currentIndex());
        emit fileSend(fileInfo.absoluteFilePath());
    }
}

void MainWindow::showCoordinates(double east, double north, bool saveMarker)
{
    qDebug() << "Form, showCoordinates" << east << north;

    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);") +
            QString("map.setZoom(%1);").arg(ui->zoomSpinBox->value());

     qDebug() << str;

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

    if (saveMarker)
        setMarker(east, north, ui->lePostalAddress->text());
}

void MainWindow::setMarker(double east, double north, QString caption)
{
    for (int i=0; i<m_markers.size(); i++)
    {
        if (m_markers[i]->caption == caption) return;
    }

    QString str =
            QString("var marker = marker || []; marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("map: map,") +
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});");

    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);


    SMarker *_marker = new SMarker(east, north, caption);
    m_markers.append(_marker);
    currLat=north;
    currLng=east;
    //adding capton to ListWidget
    //ui->lwMarkers->addItem(caption);
}

void MainWindow::goClicked()
{
    QString address = ui->lePostalAddress->text();
    m_geocodeDataManager.getCoordinates(address.replace(" ", "+"));
}



void MainWindow::errorOccured(const QString& error)
{
    QMessageBox::warning(this, tr("Geocode Error"), error);
}

void MainWindow::updateMarkers()
{
    QSqlQuery query(QString("SELECT DISTINCT Place FROM %1").arg(dbPhotoTable));
       while (query.next()) {
            QSqlQuery queryCount(QString("SELECT COUNT(*) FROM %2 WHERE Place='%3'").arg(dbPhotoTable).arg(query.value(0).toString()));

       }
}



void MainWindow::on_lwMarkers_currentRowChanged(int currentRow)
{
    if (currentRow < 0) return;
    QSqlQuery query(QString("SELECT Photo, PhotoPath FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(ui->lwMarkers->currentItem()->text()));
    query.next();
    emit fileSend(query.value(1).toString()+"/"+query.value(0).toString());
}

void MainWindow::on_pbRemoveMarker_clicked()
{
    if (ui->lwMarkers->currentRow() < 0) return;

    QString str =
            QString("mgr.removeMarker(mgr.markers[%1]);").arg(ui->lwMarkers->currentRow());
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
    //deleteing caption from markers list
    delete m_markers.takeAt(ui->lwMarkers->currentRow());
    //deleteing caption from ListWidget
    delete ui->lwMarkers->takeItem(ui->lwMarkers->currentRow());

}

void MainWindow::on_zoomSpinBox_valueChanged(int arg1)
{
    QString str =
            QString("map.setZoom(%1);").arg(arg1);
    qDebug() << str;
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
}

void MainWindow::on_pushButton_4_clicked() {

}


void MainWindow::on_pushButton_2_clicked()
{
    QString address=ui->lePostalAddress->text();
    QFileInfo fileInfo = model->fileInfo(ui->lvBackUp->currentIndex());
    QString description = ui->descrText->text();
    QDateTime date=ui->dateEdit->dateTime();
    QString name = fileInfo.fileName();
    //qDebug() << QString("INSERT into %1 VALUES (%2,%3,%4,%5,%6,%7)").arg(dbPhotoTable).arg(name).arg(address).arg(currLat).arg(currLng).arg(date.date().toString(Qt::ISODate)).arg(description);
    QSqlQuery(QString("INSERT into %1 VALUES ('%2','%3',%4,%5,'%6','%7','%8')").arg(dbPhotoTable).arg(name).arg(address).arg(currLat).arg(currLng).arg(date.date().toString(Qt::ISODate)).arg(description).arg(fileInfo.absolutePath()),db);
    //ui->lvBackUp->setItemDelegateForRow(ui->lvBackUp->currentIndex().row(),&deleg);
    //qDebug() << model->setData(ui->lvBackUp->currentIndex(),QColor(255,0,0),Qt::BackgroundColorRole);
    QSqlQuery queryTag(QString("SELECT * FROM %1 WHERE Photo='%2'").arg(dbTagTable).arg(name));
        if(queryTag.next()) {
            ui->lvBackUp->setItemDelegateForRow(ui->lvBackUp->currentIndex().row(),new GreenDelegate(ui->lvBackUp));
        }
        else {
            ui->lvBackUp->setItemDelegateForRow(ui->lvBackUp->currentIndex().row(),new YellowDelegate(ui->lvBackUp));
        }
    QKeyEvent *eventVirt = new QKeyEvent ( QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QCoreApplication::sendEvent (ui->lvBackUp, eventVirt);
    emit(imageAdded());
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &hash)
{

}

void MainWindow::on_lvBackUp_clicked(const QModelIndex &index)
{
    QString dateString;
    QFileInfo fileInfo = model->fileInfo(index);
    if (fileInfo.isFile()) {
        QSqlQuery query(QString("SELECT * FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(fileInfo.fileName()),db);
    if (query.next()) {
        ui->lePostalAddress->setText(query.value(1).toString());
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(query.value(2).toString()).arg(query.value(3).toString()) +
            QString("map.setCenter(newLoc);");
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
    dateString = query.value(4).toString();
    }
    QDate imageDate= QDate::fromString(dateString,Qt::ISODate);
    if (imageDate.isNull()) {imageDate=fileInfo.created().date();}
    ui->dateEdit->setDate(imageDate);
    ui->descrText->setText(query.value(5).toString());
    }
}


void MainWindow::on_cbTags_activated(const QString &Tag)
{
    emit(tagChanged(Tag));
    for (int i=myLW->count(); i>-1;i--) {
        delete myLW->takeItem(i);
    }
    std::vector<int> tagPhotos;
    std::map<QString,QString> photoPathList;
    QSqlQuery query(QString("SELECT Photo, PhotoPath FROM %1 WHERE Tag='%2'").arg(dbTagTable).arg(Tag),db);
    int k=0;
    while (query.next()) {
            myLW->addItem(query.value(0).toString());
             photoPathList[query.value(0).toString()]=query.value(1).toString();
             emit(photoPathsSend(photoPathList));
             QSqlQuery queryCoord(QString("SELECT Lat, Lng FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(query.value(0).toString()));
             if (!queryCoord.next()) { tagPhotos.push_back(k); continue;}
             ++k;
    }
    for (std::vector<int>::iterator it=tagPhotos.begin(); it!=tagPhotos.end(); it++) {
        myLW->item(*it)->setBackgroundColor(QColor(120,0,0));
   }
    placeMarkers();
}
void MainWindow::updateLwTags()
{
    this->on_cbTags_activated(ui->cbTags->currentText());
    int blue, red, green;
    QString color;
    for (int i=0; i<512;i++) {
        if (i<=256) {
             red = 255-i;
             green=i;
             blue=0;
        }
        else {
             red=0;
             green=511-i;
             blue=i-256;
        }
 color=QString("%1%2%3").arg(red,2,16,QChar('0')).arg(green,2,16,QChar('0')).arg(blue,2,16,QChar('0'));
 //qDebug() << color;
    }
}



void MainWindow::colorlvBackUp(QModelIndex index)
{
    int i=0;
    int dirnum=0;
    if (!appLoaded) {
    //qDebug() << model->fileName(index.sibling(0,0));
           // qDebug() << ui->lvBackUp->rootIndex().data().toString();
          //  qDebug() << model->hasIndex(i,0,index);
//            qDebug() << model->index(1,0,index).data().toString();
    while (model->hasIndex(i,0,index)) {
       if (model->fileInfo(model->index(i,0,index)).isDir()) {
            dirnum++;
       }
       i++;
    }
    i=0;
    }
    qDebug() << dirnum;
    while (model->hasIndex(i,0,index)) {
        qDebug() << model->filePath(model->index(i,0,index));
        QSqlQuery query(QString("SELECT * FROM %1 WHERE Photo='%2' AND PhotoPath='%3'").arg(dbPhotoTable).arg(model->fileName(model->index(i,0,index))).arg(model->fileInfo(model->index(i,0,index)).absolutePath()));
        if (query.next()) {
            QSqlQuery queryTag(QString("SELECT * FROM %1 WHERE Photo='%2' AND PhotoPath='%3'").arg(dbTagTable).arg(model->fileName((model->index(i,0,index)))).arg(model->fileInfo(model->index(i,0,index)).absolutePath()));
            if(queryTag.next()) {
                if (appLoaded) {
                    ui->lvBackUp->setItemDelegateForRow(i,new GreenDelegate(ui->lvBackUp));


                }
                else {
                    ui->lvBackUp->setItemDelegateForRow(i,new GreenDelegate(ui->lvBackUp));
                    qDebug() << ui->lvBackUp->model()->rowCount();
                    qDebug() << model->rowCount();

                }
            }
            else {
                if (appLoaded) {
                    ui->lvBackUp->setItemDelegateForRow(i,new YellowDelegate(ui->lvBackUp));
                }
                else {
                    ui->lvBackUp->setItemDelegateForRow(i,new YellowDelegate(ui->lvBackUp));
                    qDebug() << model->rowCount();
                }
            }
        }
        else {
            //qDebug() << model->fileName(model->index(i,0,ui->lvBackUp->currentIndex().sibling(0,0)));
            ui->lvBackUp->setItemDelegateForRow(i, new QItemDelegate(ui->lvBackUp));
        }
        ++i;
    }
}

void MainWindow::appStarted()
{
       if (!appLoaded) {
       ui->lvBackUp->setRootIndex(model->setRootPath(records["lastDir"]));
        emit(colorLv(ui->lvBackUp->rootIndex()));
       }
       appLoaded=true;
}

void MainWindow::changeMapOrientation(bool clicked)
{
markerMode=clicked;
qDebug() << markerMode;
if (markerMode) {
    this->on_cbTags_activated(ui->cbTags->currentText());
}
else {
    this->markerFromDir();
}
}


void MainWindow::testSlot(double east, double north)
{
    ui->lwMarkers->clear();
    QSqlQuery query(QString("SELECT Photo FROM %1 WHERE (Lat<%2 AND Lat>%3 AND Lng>%4 AND Lng<%5)").arg(dbPhotoTable).arg(east+0.1).arg(east-0.1).arg(north-0.1).arg(north+0.1));
    while (query.next()) {
        ui->lwMarkers->addItem(query.value(0).toString());
    }
    query.exec(QString("SELECT Place FROM %1 WHERE (Lat=%2 AND Lng=%3)").arg(dbPhotoTable).arg(east).arg(north));
    if (query.next())
    ui->lePostalAddress->setText(query.value(0).toString());
}

void MainWindow::markerFromDir()
{
    QModelIndex index = ui->lvBackUp->currentIndex();
    int i=0;
    while (model->hasIndex(i,0,index)) {
        model->filePath(model->index(i,0,index));
        ++i;
    }
}

void MainWindow::closeSlot()
{
    db.close();
    QFile::remove("tmp.sqlite");
    saveRecords();
}

void MainWindow::on_tagAdd_clicked()
{
    QFileInfo fileInfo = model->fileInfo(ui->lvBackUp->currentIndex());
    QString tag = ui->cbTags->currentText();
    QSqlQuery query(QString("SELECT Photo FROM %1 WHERE Tag='%2'").arg(dbTagTable).arg(tag));
    query.next();
    if (query.value(0).toString().isEmpty())
    {
        QSqlQuery(QString("UPDATE %1 SET Photo='%2',PhotoPath='%3' WHERE Tag='%4'").arg(dbTagTable).arg(fileInfo.fileName()).arg(fileInfo.absolutePath()).arg(tag));
    }
    else {
        QSqlQuery(QString("INSERT INTO %1 VALUES('%2','%3','%4')").arg(dbTagTable).arg(tag).arg(fileInfo.fileName()).arg(fileInfo.absolutePath()));
    }
    emit(imageAdded());
}

void MainWindow::on_actionAdd_triggered()
{
    QString tag = QInputDialog::getText(this, "Adding tag to a list", "Enter Tag Name:");
    if (tagList.contains(tag)) {
        QMessageBox::warning(this,"Warning","Name already exists");
        return;
    }
    ui->cbTags->addItem(tag);
    tagList.append(tag);
    QSqlQuery(QString("INSERT INTO %1 VALUES('%2','','')").arg(dbTagTable).arg(tag));
}

void MainWindow::on_actionDelete_triggered()
{
    QString tag = QInputDialog::getItem(this, "Delete", "Choose tag to delete",tagList,0,false);
    QSqlQuery(QString("DELETE FROM %1 WHERE Tag='%2'").arg(dbTagTable).arg(tag),db);
    int i = ui->cbTags->findText(tag);
    if (i!=-1) {
        ui->cbTags->removeItem(i);
    }
}

void MainWindow::on_lwTagPhotos_clicked(const QModelIndex &index)
{
    QString photo=myLW->item(index.row())->text();
    QSqlQuery query(QString("SELECT DISTINCT Photo, PhotoPath FROM %1 WHERE Photo='%2'").arg(dbTagTable).arg(photo));
    query.next();
    emit fileSend(query.value(1).toString()+"/"+query.value(0).toString());
    QSqlQuery queryCoord(QString("SELECT Lat, Lng FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(query.value(0).toString()));
    queryCoord.next();
    double north=queryCoord.value(0).toDouble();
    double east=queryCoord.value(1).toDouble();
    QString caption = query.value(0).toString();
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);");
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);

}



void MainWindow::on_actionNew_triggered()
{
    bool ok = dbconnect("tmp.sqlite",true);
    emit(colorLv(ui->lvBackUp->rootIndex()));
    this->updateCbTags();
    this->updateLwTags();
}

void MainWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Save DataBase"), "",tr("SqlLite DataBase (*.sqlite);;All Files (*)"));
    QFile::copy("tmp.sqlite",fileName);
    records["dbFileName"] = fileName;
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load DataBase"),"",tr("SqlLite DataBase (*.sqlite);;All Files (*)"));
    bool ok = dbconnect(fileName,false);
    emit(colorLv(ui->lvBackUp->rootIndex()));
    this->updateCbTags();
    this->updateLwTags();
    records["dbFileName"] = fileName;

}

void MainWindow::placeMarkers()
{
    int i=0;
    QSqlQuery query(db);
    QString markersDelete=QString("var markers = markers || [];   for (var i = 0; i < markers.length; i++) {markers[i].setMap(null);} markers=[];")+
            QString("var circle = {")+
            QString("path: 'M-5,0a5,5 0 1,0 10,0a5,5 0 1,0 -10,0',")+
            QString("fillOpacity: 0.8,")+
            QString("scale: 1,")+
            QString("};");
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(markersDelete);
    MyClass* marker = new MyClass(this);
    connect(marker,SIGNAL(done(double,double)),this,SLOT(testSlot(double,double)));
    ui->webView->page()->mainFrame()->addToJavaScriptWindowObject(QString("markerObj"),marker);
    if (markerMode){
        query.exec(QString("SELECT Photo FROM %1 WHERE Tag='%2'").arg(dbTagTable).arg(ui->cbTags->currentText()));
        qDebug() << "tag";
    }
    else {
        query.exec(QString("SELECT Photo FROM %1 WHERE PhotoPath='%2'").arg(dbPhotoTable).arg(model->rootDirectory().absolutePath()));
        qDebug() << QString("SELECT Photo FROM %1 WHERE PhotoPath='%2'").arg(dbPhotoTable).arg(model->rootDirectory().absolutePath());
    }
    while (query.next()) {
        qDebug() << query.value(0).toString();
        QSqlQuery queryCoord(QString("SELECT Lat, Lng FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(query.value(0).toString()));
        if (!queryCoord.next()) continue;
        if (!queryCoord.value(0).toString().isEmpty()) {
        double north=queryCoord.value(0).toDouble();
        double east=queryCoord.value(1).toDouble();
        QString caption = query.value(0).toString();
        QString str =
            QString("circle.fillColor='#%1%2%3';").arg(qrand()%256,0,16,QChar('0')).arg(qrand()%256,0,16,QChar('0')).arg(qrand()%256,0,16,QChar('0'))+
            QString("var marker = new google.maps.Marker({") +
            QString("position: new google.maps.LatLng(%1, %2),").arg(north).arg(east) +
            QString("icon: circle,")+
            QString("map: map,") +
            QString("title: %1").arg("\""+caption+"\"") +
            QString("});")+
            QString("marker.addListener('click', function() {markerObj.done(%1,%2);});").arg(north).arg(east)+
            QString("markers.push(marker);");
        ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(str);
        //qDebug()<<ui->webView->page()->currentFrame()->documentElement().toPlainText();
        }
    }
    QString setCenter = QString("var bounds = new google.maps.LatLngBounds(); for (var i = 0; i < markers.length; i++) {bounds.extend(markers[i].getPosition());}map.fitBounds(bounds);");
    ui->webView->page()->currentFrame()->documentElement().evaluateJavaScript(setCenter);
}


void MainWindow::on_tagAddAll_clicked()
{
    QModelIndex index = ui->lvBackUp->rootIndex();
    int i=0;
    QString tag = ui->cbTags->currentText();
    while (model->hasIndex(i,0,index)) {
        if (model->fileInfo(model->index(i,0,index)).isDir()) {++i; continue;}
        if (!imageExts.contains(model->fileInfo(model->index(i,0,index)).completeSuffix())) {++i; continue;}
        QSqlQuery query(QString("SELECT * FROM %1 WHERE Photo='%2' AND PhotoPath='%3'").arg(dbTagTable).arg(model->fileName((model->index(i,0,index)))).arg(model->fileInfo(model->index(i,0,index)).absolutePath()));
        query.next();
        if (!query.value(0).toString().isEmpty()) {++i; continue;}
        query.exec(QString("SELECT Photo FROM %1 WHERE Tag='%2'").arg(dbTagTable).arg(tag));
        query.next();
        if (query.value(0).toString().isEmpty())
        {
            QSqlQuery(QString("UPDATE %1 SET Photo='%2',PhotoPath='%3' WHERE Tag='%4'").arg(dbTagTable).arg(model->fileInfo(model->index(i,0,index)).fileName()).arg(model->fileInfo(model->index(i,0,index)).absolutePath()).arg(tag));
        }
        else {
            QSqlQuery(QString("INSERT INTO %1 VALUES('%2','%3','%4')").arg(dbTagTable).arg(tag).arg(model->fileInfo(model->index(i,0,index)).fileName()).arg(model->fileInfo(model->index(i,0,index)).absolutePath()));
        }
        ++i;
    }
}

void MainWindow::on_checkBox_clicked()
{

}
