#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include "fenoptions.h"
#include "downloadhandler.h"
#include "auth.h"
#include "vitessetransfert.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QClipboard>

namespace Ui
{
class FenPrincipale;
}

class FenPrincipale : public QMainWindow
{
    Q_OBJECT

public:
    explicit FenPrincipale(QWidget *parent = 0);
    ~FenPrincipale();

private slots:
    //UI
    void on_btn_ajouter_clicked();
    void on_btn_supprimer_clicked();
    void on_btn_go_clicked();
    void on_btn_arreter_clicked();
    void on_options_clicked();
    void console(QString);
    void clipboardChange();
    void settingsChanged();

    //Options
    void saveSettings();
    void loadSettings();

    //Auth
    void authSuccess(AuthInfo);
    void authFail(AuthError);

    //Transfert
    void startNextDownload();
    void updateDownload(qint64, qint64);
    void updateDownloadTick();
    void downloadComplete();
    void error(DownloadError error);

    //WaitTimer
    void waitTimerStart(int, QString);
    void waitTimerTick();
private:
    //Fonctions privées
    void closeEvent(QCloseEvent *event);
    bool isMegauploadUrl(const QString &url);
    static QString sizeToString(quint64 size);


private:
    Ui::FenPrincipale *ui;
    FenOptions *m_fenOptions;
    QSystemTrayIcon *m_tray;

    //Données persistantes
    QStringList m_adresses;
    QByteArray m_login, m_password;
    QDir m_dir;

    Auth *m_auth;
    DownloadHandler *m_handler;
    VitesseTransfert *m_vitesseTransfert;

    QString m_currentDownload;
    QTimer *m_waitTimer, *m_updateDownloadTimer;
    int m_waitTime, m_pos, m_total;
    bool m_isDownloading;
};

#endif // FENPRINCIPALE_H
