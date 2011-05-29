#ifndef INFOEXTRACTOR_H
#define INFOEXTRACTOR_H

#include "enums.h"

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QUrl>

#define FILE_NAME        "File name: "
#define FILE_DESCRIPTION "File description: "
#define FILE_SIZE        "File size: "

class InfoExtractor : public QObject
{
    Q_OBJECT
public:
    explicit InfoExtractor(QObject *parent = 0);
    ~InfoExtractor();

    void queue(const QString &url);

signals:
    void infoAvailable(QString, QString, QString, QString);
    void infoUnavailable(QString, bool);

private slots:
    void extractInfo();
    void reply();
    void replyTimeout();

private:
    QQueue<QString> m_queue;
    QString m_url;
    QTimer *m_replyTimer;

    //Réseau
    QNetworkAccessManager *m_accessManager;
    QNetworkReply *m_reply;
};

#endif // INFOEXTRACTOR_H
