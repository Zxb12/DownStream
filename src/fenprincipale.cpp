#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#include <QTime>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QDebug>

FenPrincipale::FenPrincipale(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::FenPrincipale), m_fenOptions(NULL), m_auth(new Auth(this)), m_handler(new DownloadHandler(this)),
    m_vitesseTransfert(new VitesseTransfert(this, DOWNLOAD_SPEED_UPDATE_INTERVAL, DOWNLOAD_SPEED_AVERAGE_TIME)),
    m_versionCheck(new VersionCheckThread(this, VERSION_HOST, APP_NAME, VERSION, VERSION_NBR)),
    m_currentDownload(), m_waitTimer(new QTimer(this)), m_updateDownloadTimer(new QTimer(this)), m_waitTime(0), m_isDownloading(false)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME " - v" VERSION);
    m_tray = new QSystemTrayIcon(windowIcon(), this);
    m_tray->show();
    qApp->setActiveWindow(this);
    loadSettings();

    //UI
    connect(ui->adresse, SIGNAL(returnPressed()), this, SLOT(on_btn_ajouter_clicked()));
    connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(waitTimerTick()));
    connect(m_updateDownloadTimer, SIGNAL(timeout()), this, SLOT(updateDownloadTick()));
    connect(m_versionCheck, SIGNAL(update(QString, QString)), this, SLOT(updateAvailable(QString, QString)));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardChange()));
    clipboardChange();  //Remplit tout de suite avec les donn�es du clipboard

    //T�l�chargement
    connect(m_auth, SIGNAL(authed(AuthInfo)), this, SLOT(authSuccess(AuthInfo)));
    connect(m_auth, SIGNAL(authError(AuthError)), this, SLOT(authFail(AuthError)));
    connect(m_handler, SIGNAL(error(DownloadError)), this, SLOT(error(DownloadError)));
    connect(m_handler, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownload(qint64, qint64)));
    connect(m_handler, SIGNAL(finished()), this, SLOT(downloadComplete()));
    connect(m_handler, SIGNAL(waitTime(int, QString)), this, SLOT(waitTimerStart(int, QString)));

    console("Authentification...");
    m_auth->login(m_login, m_password);
    m_updateDownloadTimer->setInterval(DOWNLOAD_SPEED_UPDATE_INTERVAL);
    m_versionCheck->start();
    m_waitTimer->setInterval(1000);

    ui->btn_arreter->hide();
}

FenPrincipale::~FenPrincipale()
{
    delete ui;
}

void FenPrincipale::on_btn_ajouter_clicked()
{
    QString adresse = ui->adresse->text();

    if (m_adresses.contains(adresse, Qt::CaseInsensitive))
        return;

    if (isMegauploadUrl(adresse))
    {
        m_adresses.push_back(adresse);
        ui->liste->addItem(adresse);
        ui->adresse->clear();
        ui->adresse->setFocus();
    }
    else
    {
        QMessageBox::warning(this, APP_NAME, "Le lien que vous avez entr� n'est pas un lien Megaupload valide !");
    }
}

void FenPrincipale::on_btn_supprimer_clicked()
{
    int row = ui->liste->currentRow();
    if (row >= 0)
    {
        QString link = ui->liste->takeItem(row)->text();
        m_adresses.removeOne(link);
    }
}

void FenPrincipale::on_btn_go_clicked()
{
    if (!m_adresses.isEmpty())
        startNextDownload();
}

void FenPrincipale::on_btn_arreter_clicked()
{
    m_isDownloading = false;
    m_handler->stopDownload();
    m_updateDownloadTimer->stop();
    m_waitTimer->stop();

    ui->progression->reset();
    ui->btn_go->show();
    ui->btn_arreter->hide();
}

void FenPrincipale::on_options_clicked()
{
    m_fenOptions = new FenOptions(this, &m_dir, &m_login, &m_password);
    connect(m_fenOptions, SIGNAL(accepted()), this, SLOT(settingsChanged()));
    m_fenOptions->show();
}

void FenPrincipale::console(QString out)
{
    qDebug() << out;
    ui->statusBar->showMessage(out);

    if (!qApp->focusWidget())
        m_tray->showMessage(APP_NAME, out);
}

void FenPrincipale::clipboardChange()
{
    QString url = QApplication::clipboard()->text().simplified();
    if (isMegauploadUrl(url))
    {
        ui->adresse->setText(url);
        ui->adresse->setFocus();
        ui->adresse->selectAll();
    }
}

void FenPrincipale::settingsChanged()
{
    console("Authentification...");
    m_auth->login(m_login, m_password);

    saveSettings();
}

void FenPrincipale::updateAvailable(QString version, QString url)
{
    QMessageBox msgBox(this);
    msgBox.setText("Une nouvelle version de DownStream est disponible !");
    msgBox.setDetailedText("Nouvelle version: " + version + "\r\n"
                           "Lien de t�l�chargement: " + url);
    msgBox.exec();

}

void FenPrincipale::saveSettings()
{
    QStringList links;
    if (!m_currentDownload.isEmpty())
        links << m_currentDownload << m_adresses;
    else
        links << m_adresses;


    QSettings settings(APP_NAME, APP_ORGANIZATION);
    settings.setValue("links", links);
    settings.setValue("login", m_login);
    settings.setValue("password", m_password);
    settings.setValue("destDir", m_dir.path());
    settings.setValue("firstRun", false);
}

void FenPrincipale::loadSettings()
{
    QSettings settings(APP_NAME, APP_ORGANIZATION);
    m_adresses = settings.value("links").toStringList();
    ui->liste->addItems(m_adresses);
    m_login = settings.value("login").toByteArray();
    m_password = settings.value("password").toByteArray();
    m_dir = settings.value("destDir", QDir::home().absolutePath()).toString();
    if (settings.value("firstRun", true).toBool())
    {
        QMessageBox::information(this, APP_NAME, "Ceci est le premier lancement de l'application. Veuillez entrer vos param�tres !");
        on_options_clicked();
    }
}

void FenPrincipale::authSuccess(AuthInfo authInfo)
{
    console("Authentification r�ussie");

    ui->btn_go->setEnabled(true);
    m_handler->setAuthInfo(authInfo);
}

void FenPrincipale::authFail(AuthError err)
{
    console("Authenficication �chou�e");

    switch (err)
    {
    case AUTH_INVALID_LOGIN:
    {
        console("Nom de compte ou mot de passe incorrect.");
        break;
    }
    case AUTH_CONNECTION_REFUSED:
    {
        console("L'h�te distant a refus� la connexion.");
        break;
    }
    case AUTH_REMOTE_HOST_CLOSED:
    {
        console("L'h�te distant a coup� la connexion pr�matur�ment");
        break;
    }
    case AUTH_HOST_NOT_FOUND:
    {
        console("H�te non trouv�");
        break;
    }
    case AUTH_TEMP_NETWORK_FAILURE:
    {
        console("D�faillance temporaire du r�seau");
        break;
    }
    case AUTH_NETWORK_ERROR:
    {
        console("D�faillance du r�seau");
        break;
    }
    case AUTH_PROTOCOL_FAILURE:
    {
        console("Erreur de protocole");
        break;
    }
    case AUTH_UNDEFINED_ERROR:
    {
        console("Erreur ind�finie");
        break;
    }
    }
}

void FenPrincipale::startNextDownload()
{
    if (!m_adresses.isEmpty() || !m_currentDownload.isEmpty())
    {
        //T�l�chargement suivant seulement si aucun t�l�chargement n'est en cours.
        if (m_currentDownload.isEmpty())
        {
            m_currentDownload = m_adresses.takeFirst();
            ui->liste->takeItem(0);
        }

        ui->btn_go->hide();
        ui->btn_arreter->show();

        m_handler->setDir(m_dir);
        m_handler->download(QUrl(m_currentDownload));
        m_vitesseTransfert->start();
        console("T�l�chargement de: " + m_currentDownload);
    }
    else
    {
        ui->statusBar->clearMessage();
        ui->btn_arreter->hide();
        ui->btn_go->show();
        ui->progression->setValue(0);
        ui->progression->setFormat("");

        m_vitesseTransfert->stop();
        m_updateDownloadTimer->stop();
        console("T�l�chargements termin�s !");
    }
}

void FenPrincipale::updateDownload(qint64 pos, qint64 total)
{
    if (m_isDownloading)  //Evite les vitesses tr�s �lev�es affich�es lors de la reprise d'un t�l�chargement
        *m_vitesseTransfert << (pos - ui->progression->value()); //Diff�rence = nbr d'octets �crits
    else
        m_isDownloading = true;

    m_pos = pos;
    m_total = total;

    if (!m_updateDownloadTimer->isActive())
        m_updateDownloadTimer->start();


    ui->progression->setMaximum(m_total);
    ui->progression->setValue(m_pos);
    updateDownloadTick();
}

void FenPrincipale::updateDownloadTick()
{
    ui->progression->setFormat("%p% - " + sizeToString(m_pos) + "/" + sizeToString(m_total) + " - " + m_vitesseTransfert->getVitesse());
}

void FenPrincipale::downloadComplete()
{
    m_isDownloading = false;
    m_currentDownload.clear();
    startNextDownload();
}

void FenPrincipale::error(DownloadError error)
{
    m_isDownloading = false;
    switch (error)
    {
    case LINK_NETWORK_ERROR:
    {
        console("Erreur: erreur de connexion pour r�cup�rer le lien pour " + m_currentDownload);
        break;
    }
    case LINK_NOT_FOUND:
    {
        console("Erreur: lien de t�l�chargement non trouv� (lien invalide ou supprim�) pour " + m_currentDownload);
        startNextDownload();
        break;
    }
    case FILE_COULD_NOT_BE_OPENED:
    {
        console("Erreur: le fichier de destination n'a pas pu �tre ouvert !");
        break;
    }
    case FILE_CORRUPT_RESTART_DOWNLOAD:
    {
        console("Erreur: le fichier de destination est corrompu, red�marrage du t�l�chargement !");
        break;
    }
    case DOWNLOAD_LIMIT_REACHED:
    {
        console("Erreur: la limite de transferts a �t� atteinte !");
        break;
    }
    case DOWNLOAD_NETWORK_ERROR:
    {
        console("Erreur: t�l�chargement interrompu !");
        break;
    }
    case DOWNLOAD_EMPTY:
    {
        console("Erreur: t�l�chargement vide ! (erreur de connexion)");
        break;
    }
    case DOWNLOAD_SOCKET_TIMEOUT:
    {
        console("Erreur: timeout de la connexion !");
        break;
    }
    default:
    {
        console("Erreur: " + QString::number(error));
        break;
    }
    }
}

void FenPrincipale::waitTimerStart(int time, QString msg)    //time = secondes
{
    m_updateDownloadTimer->stop();
    ui->progression->setFormat("%v/%m " + msg);
    ui->progression->setRange(0, time);
    ui->progression->setValue(0);
    m_waitTime = time;
    m_waitTimer->start();
}

void FenPrincipale::waitTimerTick()
{
    int time = ui->progression->value();
    if (time < m_waitTime)
        ui->progression->setValue(time + 1);
    else
        m_waitTimer->stop();
}

void FenPrincipale::closeEvent(QCloseEvent *event)
{
    saveSettings();
    m_handler->stopDownload();
    event->accept();
    if (m_versionCheck->isRunning())
        m_versionCheck->terminate();
}

bool FenPrincipale::isMegauploadUrl(const QString &url)
{
    QRegExp regexp("(http://)?www.megaupload.com/\\?d=[a-z0-9]{8}");
    regexp.setCaseSensitivity(Qt::CaseInsensitive);

    return regexp.exactMatch(url);
}

//Convertit une taille de fichier en une cha�ne avec l'extention o, ko, Mo, Go, ...
QString FenPrincipale::sizeToString(quint64 nbr)
{
    quint8 exp = 0;
    while (nbr > 1024)
    {
        exp++;
        nbr /= 1024;
    }

    QString ret = QString::number(nbr);

    switch (exp)
    {
    case 0:
        ret += "o";
        return ret;
    case 1:
        ret += "ko";
        return ret;
    case 2:
        ret += "Mo";
        return ret;
    case 3:
        ret += "Go";
        return ret;
    case 4:
        ret += "To";
        return ret;
    default:
        return "<Overflow>";
    }
}
