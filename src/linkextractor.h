#ifndef LINKEXTRACTOR_H
#define LINKEXTRACTOR_H

#include "enums.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QUrl>

class LinkExtractor : public QObject
{
    Q_OBJECT
public:
    explicit LinkExtractor(QObject *parent = 0);

    void setCookieJar(QNetworkCookieJar *jar);

    void extractLinkFrom(const QUrl &url);
    void stop();

signals:
    void linkAvailable(QUrl);
    void error(DownloadError);

private slots:
    void reply();

private:
    //Réseau
    QNetworkAccessManager *m_accessManager;
    QNetworkReply *m_reply;
};

#endif // LINKEXTRACTOR_H
