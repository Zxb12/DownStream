#ifndef AUTH_H
#define AUTH_H

#include "enums.h"

#include <QObject>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct AuthInfo
{
    AuthLevel level;
    QNetworkCookieJar *cookieJar;
};

class Auth : public QObject
{
    Q_OBJECT
public:
    explicit Auth(QObject *parent = 0);

    void login(const QByteArray& login, const QByteArray& password);

private slots:
    void reply();
    void replyError(QNetworkReply::NetworkError);

signals:
    void authed(AuthInfo);
    void authError(AuthError);

private:
    AuthLevel m_authLevel;
    QNetworkCookieJar *m_cookieJar;

    //Réseau
    QNetworkAccessManager *m_accessManager;
    QNetworkReply *m_reply;
};

#endif // AUTH_H
