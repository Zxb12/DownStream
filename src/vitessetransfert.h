#ifndef VITESSETRANSFERT_H
#define VITESSETRANSFERT_H

#include "enums.h"

#include <QObject>
#include <QTimer>
#include <QString>
#include <QMap>
#include <QTime>
#include <QtGlobal>

#define PRECISION 1     //Nombre de chiffres après la virgule pour la vitesse de transfert

class VitesseTransfert : public QObject
{
    Q_OBJECT
public:
    VitesseTransfert(QObject *parent, int intervalle, int duree);

    void start() { m_timer.start(); }
    void stop() { m_timer.stop(); }
    void operator<< (quint32 nbrOctets) { m_octetsEcrits.insert(QTime::currentTime(), nbrOctets); }
    const QString& getVitesse() const { return m_vitesseTransfert; }

public slots:
    void updateVitesse();

signals:
    void update();

protected:
    //Vitesse de transfert;
    QTimer m_timer;
    QString m_vitesseTransfert;
    int m_intervalle, m_duree;
    QMap<QTime, quint32> m_octetsEcrits;

};

#endif // VITESSETRANSFERT_H
