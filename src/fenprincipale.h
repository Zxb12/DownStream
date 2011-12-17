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
#include <QMenu>
#include <QClipboard>

namespace Ui
{
class FenPrincipale;
}

struct DownloadInfo
{
    QString url, name, description, size;

    DownloadInfo(const QString &_url = QString()) : url(_url), name(QString()), description(QString()), size(QString()) {}
    QString printableName() const { return name.isEmpty() ? url : name; }
    void clear()
    {
        url.clear();
        name.clear();
        description.clear();
        size.clear();
    }
};

struct DownloadItem : DownloadInfo
{
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
    void on_btn_monter_clicked();
    void on_btn_descendre_clicked();
    void on_btn_go_clicked();
    void on_btn_arreter_clicked();
    void on_btn_details_toggled(bool);
    void on_btn_options_clicked();
    void on_liste_currentRowChanged(int);
    void console(QString);
    void clipboardChange();
    void settingsChanged();
    void updateAvailable(QString, QString);
    void infoAvailable(QString, QString, QString, QString);
    void infoUnavailable(QString, ExtractionError);
    void trayClicked(QSystemTrayIcon::ActivationReason);
    void retablir();

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
    //Fonctions priv�es
    void addItem(const DownloadInfo &info);
    void removeItem(const int &row);
    void renameItem(const QString &url, const QString &label, const QString &tip = QString());
    void moveItem(int i, int j);

    void closeEvent(QCloseEvent *event);
    bool isMegauploadUrl(const QString &url);
    void setDetailsVisible(bool visible);
    static QString sizeToString(quint64 size);


private:
    Ui::FenPrincipale *ui;
    FenOptions *m_fenOptions;
    QSystemTrayIcon *m_tray;
    QMenu *m_menu;
    QAction *m_retablirAction, *m_startAction, *m_stopAction, *m_quitterAction;
    bool m_trayWarningShown;

    //Donn�es persistantes
    QList<DownloadItem> m_adresses;
    QByteArray m_login, m_password;
    QDir m_dir;

    Auth *m_auth;
    DownloadHandler *m_handler;
    InfoExtractor *m_infoExtractor;
    VitesseTransfert *m_vitesseTransfert;
    VersionCheckThread *m_versionCheck;

    DownloadInfo m_currentDownload;
    QTimer *m_waitTimer, *m_updateDownloadTimer;
    int m_waitTime, m_pos, m_total;
    bool m_isDownloading;
};

#endif // FENPRINCIPALE_H
