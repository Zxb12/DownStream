#ifndef FENPRINCIPALE_H
#define FENPRINCIPALE_H

#include "fenoptions.h"
#include "auth.h"
#include "downloadhandler.h"
#include "infoextractor.h"
#include "vitessetransfert.h"
#include "versioncheck.h"

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QClipboard>

namespace Ui
{
class FenPrincipale;
}

struct UrlItem
{
    QString url;
    QListWidgetItem *item;
};

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
    void updateAvailable(QString, QString);
    void infoAvailable(QString, QString, QString, QString);
    void infoUnavailable(QString, bool);

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
    void addItem(const QString &url);
    void removeItem(const int &row);
    void renameItem(const QString &url, const QString &label, const QString &tip = QString());

    void closeEvent(QCloseEvent *event);
    bool isMegauploadUrl(const QString &url);
    static QString sizeToString(quint64 size);


private:
    Ui::FenPrincipale *ui;
    FenOptions *m_fenOptions;
    QSystemTrayIcon *m_tray;

    //Données persistantes
    QList<UrlItem> m_adresses;
    QByteArray m_login, m_password;
    QDir m_dir;

    Auth *m_auth;
    DownloadHandler *m_handler;
    InfoExtractor *m_infoExtractor;
    VitesseTransfert *m_vitesseTransfert;
    VersionCheckThread *m_versionCheck;

    QString m_currentDownload;
    QTimer *m_waitTimer, *m_updateDownloadTimer;
    int m_waitTime, m_pos, m_total;
    bool m_isDownloading;
};

#endif // FENPRINCIPALE_H
