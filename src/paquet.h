#ifndef PAQUET_H
#define PAQUET_H

#include <QtGlobal>
#include <QTcpSocket>

class Paquet
{
public:
    Paquet();
    Paquet(QByteArray);
    Paquet(const Paquet&);

    //Manipulation
    Paquet& operator<<(const bool&);
    Paquet& operator<<(const qint16&);
    Paquet& operator<<(const QString&);

    Paquet& operator>>(bool&);
    Paquet& operator>>(qint16&);
    Paquet& operator>>(QString&);

    bool operator>>(QTcpSocket*);

    //Envoie le paquet préparé à la socket.
    //Doit calculer la taille du paquet avant.
    bool send(QTcpSocket*);
    void clear();

private:
    QByteArray m_paquet;
    QDataStream m_stream;
};

#endif // PAQUET_H
