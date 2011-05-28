#include "vitessetransfert.h"

VitesseTransfert::VitesseTransfert(QObject *parent, int intervalle, int duree) :
    QObject(parent), m_timer(this), m_vitesseTransfert(), m_intervalle(intervalle),
    m_duree(qMax(duree, intervalle)), m_octetsEcrits()
{
    m_timer.setInterval(m_intervalle);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateVitesse()));
}

void VitesseTransfert::updateVitesse()
{
    m_vitesseTransfert.clear();

    //On garde uniquement les données dans l'intervalle de temps étudié, et l'on regarde combien d'octets cela représente.
    float octetsEcrits = 0;
    foreach (QTime i_time, m_octetsEcrits.keys())
    {
        if (i_time.msecsTo(QTime::currentTime()) > m_duree)
            m_octetsEcrits.remove(i_time);
        else
            octetsEcrits += m_octetsEcrits.value(i_time);
    }

    double octetsParSeconde = octetsEcrits / m_duree * IN_MILLISECONDS; // Conversion de ms en s

    quint8 exp = 0;
    while (octetsParSeconde > 1024)
    {
        exp++;
        octetsParSeconde /= 1024;
    }

    m_vitesseTransfert = pNbr(octetsParSeconde, 'f', PRECISION);

    switch (exp)
    {
    case 0:
        m_vitesseTransfert += " o/s";
        break;
    case 1:
        m_vitesseTransfert += " ko/s";
        break;
    case 2:
        m_vitesseTransfert += " Mo/s";
        break;
    case 3:
        m_vitesseTransfert += " Go/s";
        break;
    case 4:
        m_vitesseTransfert += " To/s";
        break;
    default:
        m_vitesseTransfert = "<Taille trop importante pour etre affichée>";
        break;
    }

    emit update();
}
