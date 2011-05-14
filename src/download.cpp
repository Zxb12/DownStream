#include "download.h"

#include <QNetworkRequest>
#include <QDebug>

Download::Download(QObject *parent) :
        QObject(parent), m_url(), m_startPos(0), m_pos(0), m_size(0), m_accessManager(new QNetworkAccessManager(this)), m_reply(NULL), m_dir(), m_file()
{
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

    for (int i = tmp.size(); i > 0; i--)
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

    Q_ASSERT(!m_url.isEmpty());

    //Ouverture du fichier
    qDebug() << m_dir.path() + '/' + extractFileName(m_url);
    m_file.setFileName(m_dir.path() + '/' + extractFileName(m_url));
    m_file.open(QIODevice::ReadWrite);

    qDebug() << "Taille du fichier ouvert : " << m_file.size();

    if (m_file.error())
    {
        emit error(FILE_COULD_NOT_BE_OPENED);
        return;
    }
    if (m_size > m_file.size())
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
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
}

void Download::stopDownload()
{
    if (m_reply && m_reply->isRunning())
    {
        m_reply->abort();
        m_reply = NULL;
    }
}
void Download::recvData(qint64 pos, qint64 size)
{
    if (!m_size)
    {
        qDebug() << "Taille du téléchargement : " << size;
        m_size = size + m_startPos;
    }

    m_pos = m_startPos + pos;

    if (m_reply->bytesAvailable() >= DOWNLOAD_BUFFER)
        writeBuffer();

    emit downloadProgress(m_pos, m_size);
}

void Download::downloadFinished()
{
    qDebug() << "void Download::downloadFinished() " << m_reply->error() << m_reply->errorString() << ", m_pos = " << m_pos
            << ", m_startPos = " << m_startPos << ", m_size = " << m_size << ", m_file.pos() = " << m_file.pos();

    m_startPos = m_file.pos();

    //Traitement des erreurs
    switch (m_reply->error())
    {
    case QNetworkReply::OperationCanceledError: //Téléchargement annulé
        {
            qDebug() << "Téléchargement annulé";
            break;
        }
    case QNetworkReply::ProtocolUnknownError:   //Download limit exceeded
        {
            qDebug() << "Download limit exceeded";
            emit error(DOWNLOAD_LIMIT_REACHED);
            break;
        }
    case QNetworkReply::NoError:
        {
            writeBuffer();

            if (m_pos != m_size)                //Fin de téléchargement prématurée
            {
                qDebug() << "Fin de téléchargement prématurée";
                emit error(DOWNLOAD_NETWORK_ERROR);
                break;
            }

            if (m_file.pos() == m_startPos)     //Téléchargement vide
            {
                qDebug() << "Téléchargement vide";
                emit error(DOWNLOAD_EMPTY);
                break;
            }

            //Téléchargement terminé !
            m_startPos = 0, m_pos = 0, m_size = 0;
            emit finished();
            break;
        }
    default:
        {
            qDebug() << "Erreur de téléchargement !" << m_reply->error() << m_reply->errorString();
            emit error(DOWNLOAD_NETWORK_ERROR);
            break;
        }
    }

    m_file.close();
    m_reply->close();
    m_reply = NULL;
}

void Download::downloadError(QNetworkReply::NetworkError err)
{
    qDebug() << "void Download::downloadError() " << err << m_reply->errorString();
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

