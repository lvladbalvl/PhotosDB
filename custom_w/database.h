#ifndef DATABASE_H
#define DATABASE_H

#include <QWidget>
#include <QSqlQuery>
class dataBase : public QWidget
{
    Q_OBJECT
public:
    explicit dataBase(QWidget *parent = 0);
private:
    QString DBname;
    QString DBlogin;
    QString DBpass;
    QString table;
public:
signals:

public slots:
    void select(QStringList field, QString table, QString order, QString condition);
    void insert(QSqlQuery query);
    void create_table(QSqlQuery query);
};

#endif // DATABASE_H
