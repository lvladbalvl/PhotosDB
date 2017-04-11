#include "geocode_data_manager.h"
#include <QDebug>
#include "json.h"
using QtJson::JsonObject;
using QtJson::JsonArray;



GeocodeDataManager::GeocodeDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));


}

void GeocodeDataManager::getCoordinates(const QString& address)
{
    QString url = QString("http://maps.google.com/maps/api/geocode/json?address=%1&sensor=false&language=en").arg(address);
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(url)));
}


void GeocodeDataManager::replyFinished(QNetworkReply* reply)
{
    QString json = reply->readAll();
    qDebug() << "Reply = " << json;
    qDebug() << "URL = " << reply->url();
    QString strUrl = reply->url().toString();
    bool ok;
    JsonObject result = QtJson::parse(json, ok).toMap();

    // json is a QString containing the data to convert
    //QVariant result = parser.parse (json.toLatin1(), &ok);
    if(!ok)
    {
        emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
        return;
    }

    QString code = result["status"].toString();
    qDebug() << "Code" << code;
    if(code != "OK")
    {
        emit errorOccured(QString("Code of request is: %1").arg(code));
        return;
    }

    QVariantList results = result["results"].toList();
    if(results.count() == 0)
    {
        emit errorOccured(QString("Cannot find any locations"));
        return;
    }

    double east  = results[0].toMap()["geometry"].toMap()["location"].toMap()["lng"].toDouble();
    double north = results[0].toMap()["geometry"].toMap()["location"].toMap()["lat"].toDouble();

    emit coordinatesReady(east, north);

    QString address = results[0].toMap()["formatted_address"].toString();
    emit addressReady(address);
}
