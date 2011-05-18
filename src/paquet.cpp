#include "paquet.h"

Paquet::Paquet() : m_paquet(), m_stream(&m_paquet, QIODevice::ReadWrite)
{
    //On alloue la place pour la taille du paquet.
    m_stream << (quint16) 0;
}

Paquet::Paquet(QByteArray paquet) : m_paquet(paquet), m_stream(&m_paquet, QIODevice::ReadWrite)
{
}

Paquet::Paquet(const Paquet &paquet) : m_paquet(paquet.m_paquet), m_stream(&m_paquet, QIODevice::ReadWrite)
{
}





Paquet& Paquet::operator<<(const bool &val)
{
    m_stream << val;
    return *this;
}


Paquet& Paquet::operator<<(const qint16 &val)
{
    m_stream << val;
    return *this;
}

Paquet& Paquet::operator<<(const QString &val)
{
    m_stream << val;
    return *this;
}



Paquet& Paquet::operator>>(bool &val)
{
    m_stream >> val;
    return *this;
}

Paquet& Paquet::operator>>(qint16 &val)
{
    m_stream >> val;
    return *this;
}

Paquet& Paquet::operator>>(QString &val)
{
    m_stream >> val;
    return *this;
}

bool Paquet::operator>>(QTcpSocket *socket)
{
    return send(socket);
}

bool Paquet::send(QTcpSocket *socket)
{
    if (socket && socket->isWritable())
    {       //On calcule la taille du paquet et on l'envoie.
        m_stream.device()->seek(0);
        m_stream << (quint16) (m_paquet.size() - sizeof(quint16));
        socket->write(m_paquet);

        return true;
    }
    else    //Impossible d'�crire dans la socket.
        return false;
}

void Paquet::clear()
{
    m_stream.device()->seek(0);
    m_paquet.clear();
    m_stream << (quint16) 0;
}
