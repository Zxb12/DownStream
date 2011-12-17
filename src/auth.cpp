#include "auth.h"

#include <QStringList>
#include <QNetworkRequest>

Auth::Auth(QObject *parent) :
    QObject(parent), m_authLevel(GUEST), m_cookieJar(new QNetworkCookieJar(this)), m_accessManager(new QNetworkAccessManager(this)), m_reply(NULL)
{
}

void Auth::login(const QByteArray &login, const QByteArray &password)
{
    //Réinitialisation
    m_authLevel = GUEST;
    m_cookieJar->setCookiesFromUrl(QList<QNetworkCookie>(), MEGAUPLOAD);
    if (m_reply)
    {
        m_reply->abort();
        m_reply = NULL;
    }

    if (login.isEmpty())
    {
        AuthInfo authInfo = {m_authLevel, m_cookieJar};
        emit authed(authInfo);
        return;
    }


    m_reply = m_accessManager->post(QNetworkRequest(MEGAUPLOAD), "login=1&username=" + login + "&password=" + password);
    connect(m_reply, SIGNAL(finished()), this, SLOT(reply()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(replyError(QNetworkReply::NetworkError)));
}

void Auth::reply()
{
    //Vérification d'erreurs
    if (m_reply->error())
        return;

    //Extraction des cookies
    QList<QNetworkCookie> liste = QNetworkCookie::parseCookies(m_reply->rawHeader("set-cookie"));

    //Recherche du cookie user indiquant une authentification réussie
    foreach (QNetworkCookie cookie, liste)
    {
        if (cookie.name() == "user")
        {
            m_authLevel = USER;
            break;
        }
    }

    //Renvoi d'un message d'erreur si l'authentification a échoué
    if (m_authLevel == GUEST)
    {
        emit authError(AUTH_INVALID_LOGIN);
        return;
    }

    //Préparation de la structure
    m_cookieJar->setCookiesFromUrl(liste, MEGAUPLOAD);
    AuthInfo authInfo = {m_authLevel, m_cookieJar};

    emit authed(authInfo);

    m_reply->close();
    m_reply = NULL;
}

void Auth::replyError(QNetworkReply::NetworkError err)
{
    switch (err)
    {
    case QNetworkReply::ConnectionRefusedError:
    {
        emit authError(AUTH_CONNECTION_REFUSED);
        break;
    }
    case QNetworkReply::RemoteHostClosedError:
    {
        emit authError(AUTH_REMOTE_HOST_CLOSED);
        break;
    }
    case QNetworkReply::HostNotFoundError:
    {
        emit authError(AUTH_HOST_NOT_FOUND);
        break;
    }
    case QNetworkReply::TemporaryNetworkFailureError:
    {
        emit authError(AUTH_TEMP_NETWORK_FAILURE);
        break;
    }
    case QNetworkReply::UnknownNetworkError:
    {
        emit authError(AUTH_NETWORK_ERROR);
        break;
    }
    case QNetworkReply::ProtocolFailure:
    {
        emit authError(AUTH_PROTOCOL_FAILURE);
        break;
    }
    case QNetworkReply::OperationCanceledError:
    {
        //Erreur émise lors de l'appel à abort()
        return;
    }
    default:
    {
        emit authError(AUTH_UNDEFINED_ERROR);
        break;
    }
    }

    if (m_reply)
    {
        m_reply->abort();
        m_reply = NULL;
    }
}
