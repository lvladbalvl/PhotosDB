#ifndef TAGLIST_H
#define TAGLIST_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSqlQuery>


class tagListWidget : public QListWidget
{
    Q_OBJECT
public:
    tagListWidget( QString dbTagName, QString dbPhotoName, QSqlDatabase dbase, QListWidget *parent=0);
    public slots:
        void customMenuRequested(QPoint pos);
        void mousePressEvent (QMouseEvent *event);
        void deleteRow ();
        void tagListWidget_clicked(const QModelIndex &index);
        void photoPathsSet(std::map<QString,QString> PathNames);
        void tagSet(QString tag);
    signals:
        void fileSend(QString);
        void toJavaScript(QString);
private:
        QListWidgetItem* rightClickedItem;
        QString dbTagTable;
        QString dbPhotoTable;
        QSqlDatabase db;
        std::map<QString,QString> photoPathList;
        QString currentTag;
};

#endif // TAGLIST_H
