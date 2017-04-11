#include "database.h"
#include <QSqlDatabase>
dataBase::dataBase(QWidget *parent) : QWidget(parent)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
       db.setDatabaseName("test.sqlite");
       bool ok = db.open();
}
