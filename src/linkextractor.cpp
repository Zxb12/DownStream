#include "linkextractor.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>

#include <QDebug>

LinkExtractor::LinkExtractor(QObject *parent) :
    QObject(parent), m_accessManager(new QNetworkAccessManager(this)), m_reply(NULL)
{
}

void LinkExtractor::setCookieJar(QNetworkCookieJar *jar)
{
    m_accessManager->setCookieJar(jar);
}

void LinkExtractor::extractLinkFrom(const QUrl &url)
{
    m_reply = m_accessManager->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(reply()));
}

void LinkExtractor::stop()
{
    if (m_reply && m_reply->isOpen())
    {
        m_reply->abort();
    }
}

void LinkExtractor::reply()
{
    //Vérification des erreurs
    if (QNetworkReply::NetworkError err = m_reply->error())
    {
        if (err == QNetworkReply::OperationCanceledError)
        {
            m_reply = NULL;
            return;
        }
        emit error(LINK_NETWORK_ERROR);
    }
    //Extraction des données de la réponse
    QByteArray data = m_reply->readAll();
    m_reply->close();
    m_reply = NULL;

    //Extraction du lien
    QRegExp regexp("http://www[0-9]*.megaupload.com/files/[^\"]*");
    int pos = regexp.indexIn(data);
    if (pos == -1)
    {
        emit error(LINK_NOT_FOUND);
        return;
    }

    //Envoi du lien
    QUrl fileLink = regexp.cap();
    emit linkAvailable(fileLink);

}

