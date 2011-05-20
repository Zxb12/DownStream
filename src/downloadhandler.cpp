#include "downloadhandler.h"

#include <QDebug>

DownloadHandler::DownloadHandler(QObject *parent) :
    QObject(parent), m_authInfo(), m_url(), m_downloadUrl(), m_timer(new QTimer(this)),
    m_linkExtractor(new LinkExtractor(this)), m_download(new Download(this)), m_dir()
{
    m_timer->setSingleShot(true);

    connect(m_linkExtractor, SIGNAL(linkAvailable(QUrl)), this, SLOT(linkAvailable(QUrl)));
    connect(m_linkExtractor, SIGNAL(error(DownloadError)), this, SLOT(downloadError(DownloadError)));
    connect(m_download, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(recvData(qint64, qint64)));
    connect(m_download, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(m_download, SIGNAL(error(DownloadError)), this, SLOT(downloadError(DownloadError)));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(startDownload()));
}

void DownloadHandler::setDir(const QDir &dir)
{
    m_dir = dir;

    m_download->setDir(dir);
}

void DownloadHandler::setAuthInfo(const AuthInfo &authInfo)
{
    m_authInfo = authInfo;

    m_linkExtractor->setCookieJar(m_authInfo.cookieJar);
    m_download->setCookieJar(m_authInfo.cookieJar);
}

void DownloadHandler::download(const QUrl &url)
{
    m_url = url;

    m_linkExtractor->extractLinkFrom(m_url);
}

void DownloadHandler::stopDownload()
{
    m_timer->stop();
    m_linkExtractor->stop();
    m_download->stopDownload();
}

void DownloadHandler::linkAvailable(QUrl url)
{
    qDebug() << "Lien de téléchargement trouvé !" << url;

    m_downloadUrl = url;
    wait(m_authInfo.level, BEFORE_LINK_AVAILABLE, SLOT(startDownload()));
}

void DownloadHandler::restartLinkExtraction()
{
    m_linkExtractor->extractLinkFrom(m_url);
}

void DownloadHandler::startDownload()
{
    m_download->startDownload(m_downloadUrl);
}

void DownloadHandler::restartDownload()
{
    m_download->startDownload();
}

void DownloadHandler::recvData(qint64 pos, qint64 size)
{
    emit downloadProgress(pos, size);
}

void DownloadHandler::downloadFinished()
{
    emit finished();
}

void DownloadHandler::downloadError(DownloadError err)
{
    emit error(err);

    switch (err)
    {
    case DOWNLOAD_NETWORK_ERROR:
    case DOWNLOAD_SOCKET_TIMEOUT:
        wait(RETRY_TIMER, BEFORE_NEXT_TRY, SLOT(restartDownload()));
        break;
    case DOWNLOAD_EMPTY:
        restartLinkExtraction();
        break;
    case DOWNLOAD_LIMIT_REACHED:
        wait(LIMIT_REACHED_TIMER, BEFORE_NEXT_TRY, SLOT(restartLinkExtraction()));
        break;
    case LINK_NETWORK_ERROR:
        wait(RETRY_TIMER, BEFORE_NEXT_TRY, SLOT(restartLinkExtraction()));

    default:
        break;
    }
}

void DownloadHandler::wait(const int& time, const QString &msg, const char *slot)
{
    m_timer->stop();
    m_timer->disconnect(this);  //Déconnexion de tous les slots
    connect(m_timer, SIGNAL(timeout()), this, slot);    //Reconnexion au nouveau slot
    m_timer->start(time * IN_MILLISECONDS);

    emit waitTime(time, msg);
}
