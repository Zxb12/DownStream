#include "download.h"

#include <QNetworkRequest>

Download::Download(QObject *parent) :
    QObject(parent), m_url(), m_startPos(0), m_pos(0), m_size(0), m_timer(new QTimer(this)),m_accessManager(new QNetworkAccessManager(this)),
    m_reply(NULL), m_dir(), m_file()
{
    m_timer->setSingleShot(true);
    m_timer->setInterval(DOWNLOAD_NO_RECV_TIMER * IN_MILLISECONDS);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(socketTimeout()));
}

void Download::setCookieJar(QNetworkCookieJar *cookieJar)
{
    m_accessManager->setCookieJar(cookieJar);
}

void Download::setDir(const QDir &dir)
{
    m_dir = dir;
}

QString Download::extractFileName(const QUrl &url)
{
    QString tmp(url.path()), fileName;

    for (int i = tmp.size() - 1; i > 0; i--)
    {
        if (tmp[i] != '/')
            fileName.push_front(tmp[i]);
        else
            break;
    }

    return fileName;
}

void Download::writeBuffer()
{
    QByteArray data = m_reply->read(m_reply->bytesAvailable());
    m_file.write(data);
}

/* Lors du lancement de cette fonction pour un nouveau téléchargement, m_startPos, m_pos et m_size
   sont supposés être 0, sinon le téléchargement reprend.
   m_file est supposé être fermé.
   Echec silencieux si un téléchargement est déjà en cours */
void Download::startDownload(const QUrl &url)
{
    if (m_reply && m_reply->isOpen())
        return;

    if (!url.isEmpty())
        m_url = url;

    if (m_file.isOpen())
        m_file.close();

    sLog->out("startDownload() m_pos = %1, m_startPos = %2, m_size = %3", pNbr(m_pos), pNbr(m_startPos), pNbr(m_size));

    Q_ASSERT(!m_url.isEmpty());

    //Ouverture du fichier
    sLog->out("Fichier téléchargé : %1", extractFileName(m_url));
    m_file.setFileName(m_dir.path() + '/' + extractFileName(m_url));
    m_file.open(QIODevice::ReadWrite);

    sLog->out("Taille du fichier ouvert : %1", pNbr(m_file.size()));

    if (m_file.error())
    {
        emit error(FILE_COULD_NOT_BE_OPENED);
        return;
    }
    if (m_size && m_size < m_file.size())
    {
        emit error(FILE_CORRUPT_RESTART_DOWNLOAD);
        m_startPos = 0;
        m_file.resize(0);
    }
    else
    {
        m_startPos = m_file.size();
        m_file.seek(m_startPos);
    }
    //Requête réseau
    QNetworkRequest req(m_url);
    req.setRawHeader("Range", "bytes=" + QByteArray::number(m_startPos) + "-");
    m_reply = m_accessManager->get(req);

    connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(recvData(qint64, qint64)));
    connect(m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void Download::stopDownload()
{
    if (m_reply && m_reply->isRunning())
    {
        m_reply->abort();
        m_reply = NULL;
        m_timer->stop();
    }
}
void Download::recvData(qint64 pos, qint64 size)
{
    if (size == -1 || m_reply->error()) //Erreur
        return;

    if (!size)  //size == 0 dans certains cas d'erreur invisibles par m_reply->error()
    {
        emit error(DOWNLOAD_EMPTY);
        m_reply->abort();
        return;
    }


    if (!m_size)
    {
        sLog->out("Taille du téléchargement : %1", pNbr(size));
        m_size = size + m_startPos;
    }

    m_pos = m_startPos + pos;

    if (m_reply->bytesAvailable() >= DOWNLOAD_BUFFER)
        writeBuffer();

    m_timer->start();

    emit downloadProgress(m_pos, m_size);
}

void Download::downloadFinished()
{
    sLog->out("downloadFinished() m_pos = %1, m_startPos = %2, m_size =  %3, m_file.pos() = %4, errors: %5 (%6)",
              pNbr(m_pos), pNbr(m_startPos), pNbr(m_size), pNbr(m_file.pos()), pNbr(m_reply->error()), m_reply->errorString());

    m_startPos = m_file.pos();
    m_timer->stop();

    //Traitement des erreurs
    switch (m_reply->error())
    {
    case QNetworkReply::OperationCanceledError: //Téléchargement annulé
    {
        sLog->out("Téléchargement annulé");
        break;
    }
    case QNetworkReply::ProtocolUnknownError:   //Download limit exceeded
    {
        sLog->out("Download limit exceeded");
        emit error(DOWNLOAD_LIMIT_REACHED);
        break;
    }
    case QNetworkReply::UnknownContentError:    //Téléchargement déjà terminé
    {
        if (m_reply->errorString().contains("Requested Range Not Satisfiable", Qt::CaseInsensitive))
        {
            sLog->out("Téléchargement déjà terminé");
            m_startPos = 0, m_pos = 0, m_size = 0;
            emit finished();
        }
        else
        {
            sLog->out("Unknown download error");
            emit error(DOWNLOAD_NETWORK_ERROR);
        }
        break;
    }
    case QNetworkReply::NoError:
    {
        writeBuffer();

        if (m_file.pos() == m_startPos)     //Téléchargement vide
        {
            sLog->out("Téléchargement vide");
            emit error(DOWNLOAD_EMPTY);
            break;
        }

        if (m_pos != m_size)                //Fin de téléchargement prématurée
        {
            sLog->out("Fin de téléchargement prématurée");
            emit error(DOWNLOAD_NETWORK_ERROR);
            break;
        }

        //Téléchargement terminé !
        m_startPos = 0, m_pos = 0, m_size = 0;
        emit finished();
        break;
    }
    default:
    {
        sLog->out("Erreur de téléchargement : %1 (%2)", pNbr(m_reply->error()), m_reply->errorString());
        emit error(DOWNLOAD_NETWORK_ERROR);
        break;
    }
    }

    m_file.close();
    m_reply->close();
    m_reply = NULL;
}

void Download::socketTimeout()
{
    m_reply->abort();
    emit error(DOWNLOAD_SOCKET_TIMEOUT);
}

bool Download::containsError(const QByteArray &data)
{
    //Recherche de message d'erreur
    if (data.contains(DOWNLOAD_LIMIT_EXCEEDED))
    {
        emit error(DOWNLOAD_LIMIT_REACHED);
        return false;
    }

    return true;
}

