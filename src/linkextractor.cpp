#include "linkextractor.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegExp>
#include <QTextDocumentFragment>
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
    //V�rification des erreurs
    if (QNetworkReply::NetworkError err = m_reply->error())
    {
        if (err == QNetworkReply::OperationCanceledError)
        {
            m_reply = NULL;
            return;
        }
        sLog->out("LinkExtractor::reply() erreur: %1", m_reply->errorString());
        emit error(LINK_NETWORK_ERROR);
        m_reply->close();
        m_reply = NULL;
        return;
    }

    //Extraction des donn�es de la r�ponse
    QByteArray data = m_reply->readAll();
    m_reply->close();
    m_reply = NULL;

    //Extraction du lien
    QRegExp regexp(LINK_EXTRACTION_REGEXP);
    if (regexp.indexIn(data) == -1)
    {
        if (data.contains(LINK_EXTRACTION_NEED_PASSWORD))
        {
            emit error(PASSWORD_REQUIRED);
        }
        else if (data.contains(PREMIUM_ACCOUNT_NEEDED))
        {
            emit error(NEED_PREMIUM);
        }
        else
        {
            emit error(LINK_NOT_FOUND);
        }
        return;
    }

    //Interpr�tation des caract�res sp�ciaux (HTML) dans le lien
    QUrl fileLink = QTextDocumentFragment::fromHtml(regexp.cap()).toPlainText();

    emit linkAvailable(fileLink);
}

