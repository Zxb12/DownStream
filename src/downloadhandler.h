#ifndef DOWNLOADHANDLER_H
#define DOWNLOADHANDLER_H

#include "ui_fenprincipale.h"
#include "auth.h"
#include "linkextractor.h"
#include "download.h"
#include "enums.h"

#include <QObject>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QTimer>

class DownloadHandler : public QObject
{
    Q_OBJECT
public:
    DownloadHandler(QObject *parent = 0);

    void setDir(const QDir &dir);
    void setAuthInfo(const AuthInfo &authInfo);
    void download(const QUrl &url);
    void stopDownload();
public slots:
    void restartDownload();

private slots:
    void linkAvailable(QUrl url);
    void restartLinkExtraction();
    void startDownload();
    void recvData(qint64, qint64);
    void downloadFinished();
    void downloadError(DownloadError err);
private:
    void wait(int time, const QString &msg, const char* slot);

private:
    //Compte
    AuthInfo m_authInfo;

    //Télchargement
    QUrl m_url, m_downloadUrl;
    QTimer *m_timer;
    LinkExtractor *m_linkExtractor;
    Download *m_download;

    //Stockage
    QDir m_dir;

signals:
    void error(DownloadError);
    void waitTime(int, QString);
    void downloadProgress(qint64, qint64);
    void finished();
};

#endif // DOWNLOADHANDLER_H
