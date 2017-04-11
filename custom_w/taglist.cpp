#include "taglist.h"
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QSqlQuery>
#include <QDebug>
#include <QModelIndex>

tagListWidget::tagListWidget( QString dbTagName, QString dbPhotoName, QSqlDatabase dbase,   QListWidget *parent)
  : QListWidget( parent )
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),SLOT(customMenuRequested(QPoint)));
    connect(this,SIGNAL(clicked(QModelIndex)),this,SLOT(tagListWidget_clicked(QModelIndex)));
    this->dbTagTable = dbTagName;
    this->dbPhotoTable = dbPhotoName;
    QSqlDatabase db = dbase;

}
void tagListWidget::mousePressEvent(QMouseEvent *event) {
 if (event->button()==Qt::RightButton)
     this->rightClickedItem = this->itemAt(this->mapFromGlobal(this->cursor().pos()));
 else {
     QListWidget::mousePressEvent(event);
 }
}
void tagListWidget::deleteRow() {
    if (this->rightClickedItem) {
        QString photo = this->rightClickedItem->text();
        QSqlQuery(QString("DELETE FROM %1 WHERE Photo='%2' AND PhotoPath='%3' AND Tag='%4'").arg(dbTagTable).arg(photo).arg(this->photoPathList[photo]).arg(this->currentTag));
        qDebug() << QString("DELETE FROM %1 WHERE Photo='%2' AND PhotoPath='%3' AND Tag='%4'").arg(dbTagTable).arg(photo).arg(this->photoPathList[photo]).arg(this->currentTag);
        delete this->rightClickedItem;
    }
}
void tagListWidget::customMenuRequested(QPoint pos){
    pos=this->mapToGlobal(pos);
QAction* del = new QAction("delete",this);
connect(del,SIGNAL(triggered(bool)),this,SLOT(deleteRow()));
QMenu *menu=new QMenu(this);
menu->addAction(del);
menu->setGeometry(pos.x(),pos.y(),10,10);
menu->exec();
}

void tagListWidget::tagListWidget_clicked(const QModelIndex &index)
{
    QString photo=this->item(index.row())->text();
    qDebug() << QString("SELECT DISTINCT Photo, PhotoPath FROM %1 WHERE Photo='%2'").arg(this->dbTagTable).arg(photo);
    QSqlQuery query(QString("SELECT DISTINCT Photo, PhotoPath FROM %1 WHERE Photo='%2'").arg(this->dbTagTable).arg(photo),db);
    query.next();
    emit fileSend(query.value(1).toString()+"/"+query.value(0).toString());
    QSqlQuery queryCoord(QString("SELECT Lat, Lng FROM %1 WHERE Photo='%2'").arg(dbPhotoTable).arg(query.value(0).toString()));
    queryCoord.next();
    double north=queryCoord.value(0).toDouble();
    double east=queryCoord.value(1).toDouble();
    QString caption = query.value(0).toString();
    qDebug() << caption;
    QString str =
            QString("var newLoc = new google.maps.LatLng(%1, %2); ").arg(north).arg(east) +
            QString("map.setCenter(newLoc);");
    emit(toJavaScript(str));
    qDebug() << "done";

}

void tagListWidget::photoPathsSet(std::map<QString,QString> PathNames)
{
    this->photoPathList = PathNames;
}

void tagListWidget::tagSet(QString tag)
{
    this->currentTag=tag;
}
