#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "enums.h"

#include <QObject>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Download : public QObject
{
    Q_OBJECT
public:
    explicit Download(QObject *parent = 0);

    void startDownload(const QUrl &url = QUrl());
    void stopDownload();

    void setCookieJar(QNetworkCookieJar *cookieJar);
    void setDir(const QDir &dir);

private:
    //Téléchargement
    bool containsError(const QByteArray &data);
    QString extractFileName(const QUrl &url);
    void writeBuffer();
    QUrl m_url;
    qint64 m_startPos, m_pos, m_size;

    //Réseau
    QNetworkAccessManager *m_accessManager;
    QNetworkReply *m_reply;

    //Stockage
    QDir m_dir;
    QFile m_file;

private slots:
    void recvData(qint64, qint64);
    void downloadFinished();

signals:
    void error(DownloadError err);
    void downloadProgress(qint64, qint64);
    void finished();
};

#endif // DOWNLOAD_H
