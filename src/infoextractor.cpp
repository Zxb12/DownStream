#include "infoextractor.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QStringList>
#include <QRegExp>
#include <QTextDocumentFragment>

InfoExtractor::InfoExtractor(QObject *parent) :
    QObject(parent), m_queue(), m_url(), m_replyTimer(new QTimer(this)),
    m_accessManager(new QNetworkAccessManager(this)), m_reply(NULL)
{
    //Définition du cookie pour avoir la page en anglais
    QNetworkCookieJar *jar = new QNetworkCookieJar(this);
    QNetworkCookie cookie("l", "en");
    QList<QNetworkCookie> liste;
    liste << cookie;
    jar->setCookiesFromUrl(liste, MEGAUPLOAD);
    m_accessManager->setCookieJar(jar);

    m_replyTimer->setSingleShot(true);
    m_replyTimer->setInterval(INFO_EXTRACTION_TIMEOUT);
    connect(m_replyTimer, SIGNAL(timeout()), this, SLOT(replyTimeout()));
}

InfoExtractor::~InfoExtractor()
{
    if (m_reply)
        m_reply->abort();
}

void InfoExtractor::queue(const QString &url)
{
    m_queue.enqueue(url);
    if (!m_reply)
        extractInfo();
}

void InfoExtractor::remove(const QString &url)
{
    m_queue.removeOne(url);
}

void InfoExtractor::extractInfo()
{
    Q_ASSERT(!m_queue.isEmpty());

    m_url = m_queue.dequeue();

    m_reply = m_accessManager->get(QNetworkRequest(m_url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(reply()));

    m_replyTimer->start();
}

void InfoExtractor::reply()
{
    m_replyTimer->stop();

    //Vérification des erreurs
    if (QNetworkReply::NetworkError err = m_reply->error())
    {
        if (err != QNetworkReply::OperationCanceledError)
        {
            sLog->out("InfoExtractor::reply() erreur: %1", m_reply->errorString());
        }
        emit infoUnavailable(m_url, true);
        m_reply->close();
        m_reply = NULL;
        if (!m_queue.isEmpty())
            extractInfo();
        return;
    }

    //Extraction des données de la réponse
    QString data = QTextDocumentFragment::fromHtml(m_reply->readAll()).toPlainText(); //Interprétation HTML
    m_reply->close();
    m_reply = NULL;

    if (data.isEmpty())
    {
        sLog->out("InfoExtractor::reply() réponse de taille nulle");
        m_queue.enqueue(m_url);
        extractInfo();
        return;
    }

    //Extraction des données
    QString fileName, fileDescription, fileSize;
    QStringList splitData = data.split('\n', QString::SkipEmptyParts);
    foreach(QString lineData, splitData)
    {
        if (lineData.startsWith(FILE_NAME))
            fileName = lineData.remove(FILE_NAME);
        else if (lineData.startsWith(FILE_DESCRIPTION))
            fileDescription = lineData.remove(FILE_DESCRIPTION);
        else if (lineData.startsWith(FILE_SIZE))
            fileSize = lineData.remove(FILE_SIZE);
        else if (lineData.startsWith(FILE_TEMPORARILY_UNAVAILABLE))
        {
            emit infoUnavailable(m_url, true);
            m_queue.enqueue(m_url);
            extractInfo();
            return;
        }
    }

    if (fileName.isEmpty())
    {
        sLog->out("InfoExtractor::reply() Lien non trouvé. Données: %1", data);
        emit infoUnavailable(m_url, false);
        return;
    }

    emit infoAvailable(m_url, fileName, fileDescription, fileSize);
    if (!m_queue.isEmpty())
        extractInfo();
}

void InfoExtractor::replyTimeout()
{
    sLog->out("InfoExtractor::replyTimeout()");
    m_replyTimer->stop();

    if (m_reply)
        m_reply->abort();
}
