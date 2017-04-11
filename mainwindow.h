#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFileSystemModel>
#include <QDir>
#include <QDateTime>
#include <QKeyEvent>
#include <QAbstractItemView>
#include "geocode_data_manager.h"
#include <QSqlDatabase>
#include <QListWidgetItem>
#include <QItemDelegate>
#include <QTimer>
#include "button/qslideswitch.h"
#include "button/qslideswitch_p.h"
#include <map>
#include "./custom_w/taglist.h"
struct SMarker
{
    SMarker()
    {
        east = 0;
        north = 0;
        caption = "";
    };
    SMarker(double _east, double _north, QString _caption)
    {
        east = _east; north = _north; caption = _caption;
    };

    double east;
    double north;
    QString caption;
};

class MyClass : public QObject
{
  Q_OBJECT
public:
  MyClass( QObject *parent = 0 );
public slots:

 signals:
  done(double,double);
};



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool dbconnect(QString fileName, bool newFile);
    void updateCbTags();
    bool getRecords();
    bool saveRecords();

private slots:
    void on_lvBackUp_doubleClicked(const QModelIndex &index);
    void on_pushButton_clicked();
    void goClicked();

    void showCoordinates(double east, double north, bool saveMarker = true);
    //set marker to map and save marker in markers list
    void setMarker(double east, double north, QString caption);
    void errorOccured(const QString&);
    void updateMarkers();

    void on_lwMarkers_currentRowChanged(int currentRow);

    void on_pbRemoveMarker_clicked();

    void on_zoomSpinBox_valueChanged(int arg1);
    void on_pushButton_2_clicked();

    void on_comboBox_currentIndexChanged(const QString &arg1);

    void on_lvBackUp_clicked(const QModelIndex &index);

    void on_tagAdd_clicked();

    void on_actionAdd_triggered();

    void on_actionDelete_triggered();

    void on_lwTagPhotos_clicked(const QModelIndex&);

    void on_cbTags_activated(const QString &arg1);

    void updateLwTags();

    void colorlvBackUp(QModelIndex);

    void appStarted();

    void changeMapOrientation(bool);

    void testSlot(double,double);

    void markerFromDir();

    void closeSlot();

    void on_actionNew_triggered();

    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void placeMarkers();

    void on_pushButton_4_clicked();

    void on_tagAddAll_clicked();

    void on_checkBox_clicked();

signals:
    void fileSend(QString fileName);
    void imageAdded();
    void colorLv(QModelIndex);
    void directoryChanged();
    void photoPathsSend(std::map<QString,QString>);
    void tagChanged(QString);
private:
    Ui::MainWindow *ui;
    QFileSystemModel *model;
    GeocodeDataManager m_geocodeDataManager;
    QTimer *timer;
    QSlideSwitch* switcher ;
    QString dbPhotoTable;
    QString dbTagTable;
    tagListWidget* myLW;
    double currLat;
    QString recFileName = "records.txt";
    double currLng;
    QStringList tagList;
    QSqlDatabase db;
    bool markerMode;
    bool appLoaded;
    QString lastDBFile;
    QStringList imageExts;
    std::map<QString,QString> records;
    //markers list
    QList <SMarker*> m_markers;
    void keyPressEvent(QKeyEvent *event);
    void getCoordinates(const QString& address);
};
void contentDifference(QDir &sDir, QDir &bDir, QFileInfoList &diffList);
void recursiveContentList(QDir &dir, QFileInfoList &contentList);




#endif // MAINWINDOW_H
