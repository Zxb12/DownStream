#include "versioncheck.h"
#include "paquet.h"

#include <QMessageBox>

VersionCheckThread::VersionCheckThread(QObject *parent, QString host, QString prog, QString version, qint16 versionNbr) :
    QThread(parent), m_taillePaquet(0), m_host(host), m_prog(prog), m_version(version), m_versionNbr(versionNbr)
{
    m_socket = new QTcpSocket(this);
}

void VersionCheckThread::run()
{
    connect(m_socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(recvData()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(terminate()));

    m_socket->connectToHost(m_host, PORT);

    sleep(15);
}

void VersionCheckThread::connected()
{

    Paquet out;
    out << m_prog << m_version << m_versionNbr;
    out >> m_socket;
}

void VersionCheckThread::recvData()
{
    QDataStream stream(m_socket);

    //Récupération de la taille du paquet
    if (!m_taillePaquet)
    {
        if (m_socket->bytesAvailable() < sizeof m_taillePaquet)
            return;

        stream >> m_taillePaquet;
    }

    //Récupération du reste du paquet
    if (m_socket->bytesAvailable() < m_taillePaquet)
        return;

    //On lit la socket pour la taille d'un paquet et on stocke.
    Paquet *in = new Paquet(m_socket->read(m_taillePaquet));

    m_taillePaquet = 0;

    m_socket->close();

    bool updateAvailable;
    *in >> updateAvailable;

    if (updateAvailable)
    {
        QString version, url;
        *in >> version >> url;
        emit update(version, url);
    }

    delete in;

    terminate();
}
