#ifndef VERSIONCHECK_H
#define VERSIONCHECK_H

#define PORT    50181

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QDebug>

class VersionCheckThread : public QThread
{
    Q_OBJECT
public:

    VersionCheckThread(QObject *parent, QString host, QString prog, QString version, qint16 versionNbr);

private:
    void run();

private slots:
    void connected();
    void recvData();

signals:
    void update(QString, QString);

private:
    QTcpSocket *m_socket;
    quint16 m_taillePaquet;

    QString m_host, m_prog, m_version;
    qint16 m_versionNbr;
};

#endif // VERSIONCHECK_H
