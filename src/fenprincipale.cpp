#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#include <QTime>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>

FenPrincipale::FenPrincipale(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::FenPrincipale), m_fenOptions(NULL), m_auth(new Auth(this)), m_handler(new DownloadHandler(this)),
    m_infoExtractor(new InfoExtractor(this)), m_vitesseTransfert(new VitesseTransfert(this, DOWNLOAD_SPEED_UPDATE_INTERVAL, DOWNLOAD_SPEED_AVERAGE_TIME)),
    m_versionCheck(new VersionCheckThread(this, VERSION_HOST, APP_NAME, VERSION, VERSION_NBR)), m_currentDownload(), m_waitTimer(new QTimer(this)),
    m_updateDownloadTimer(new QTimer(this)), m_waitTime(0), m_isDownloading(false)
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
    connect(m_infoExtractor, SIGNAL(infoAvailable(QString,QString,QString,QString)), this, SLOT(infoAvailable(QString,QString,QString,QString)));
    connect(m_infoExtractor, SIGNAL(infoUnavailable(QString,bool)), this, SLOT(infoUnavailable(QString,bool)));
    connect(m_updateDownloadTimer, SIGNAL(timeout()), this, SLOT(updateDownloadTick()));
    connect(m_versionCheck, SIGNAL(update(QString, QString)), this, SLOT(updateAvailable(QString, QString)));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardChange()));

    //Téléchargement
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
    clipboardChange();  //Initialise la zone d'adresse avec les adresses déjà présentes en mémoire.
    setDetailsVisible(ui->btn_details->isChecked());
    ui->btn_arreter->hide();
    ui->liste->setCurrentRow(0);

    sLog->out(APP_NAME " startup.");
}

FenPrincipale::~FenPrincipale()
{
    delete ui;
    delete m_auth;
    delete m_handler;
    delete m_infoExtractor;
    delete m_vitesseTransfert;
    delete m_versionCheck;
    delete m_updateDownloadTimer;
    delete m_waitTimer;

    sLog->free();
}

void FenPrincipale::on_btn_ajouter_clicked()
{
    QStringList newUrls = ui->adresse->text().simplified().split(' ', QString::SkipEmptyParts);
    QStringList urls;
    foreach(UrlItem info, m_adresses)
    {
        urls << info.url;
    }

    foreach (QString url, newUrls)
    {
        if (urls.contains(url, Qt::CaseInsensitive) || !isMegauploadUrl(url))
            newUrls.removeOne(url);
        else
            addItem(url);
    }

    ui->adresse->clear();
    ui->adresse->setFocus();
}

void FenPrincipale::on_btn_supprimer_clicked()
{
    int row = ui->liste->currentRow();
    if (row >= 0)
        removeItem(row);
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

void FenPrincipale::on_btn_details_toggled(bool visible)
{
    setDetailsVisible(visible);
}

void FenPrincipale::on_options_clicked()
{
    m_fenOptions = new FenOptions(this, &m_dir, &m_login, &m_password);
    connect(m_fenOptions, SIGNAL(accepted()), this, SLOT(settingsChanged()));
    m_fenOptions->show();
}

void FenPrincipale::on_liste_currentRowChanged(int row)
{
    if (row >= 0)
    {
        UrlItem item = m_adresses[row];
        ui->nomFichier->setText(item.name);
        ui->description->setText(item.description);
        ui->taille->setText(item.size);
        ui->lienMegaupload->setText(item.url);
    }
    else
    {
        ui->nomFichier->setText("");
        ui->description->setText("");
        ui->taille->setText("");
        ui->lienMegaupload->setText("");
    }
}

void FenPrincipale::console(QString out)
{
    ui->statusBar->showMessage(out);

    if (!qApp->focusWidget())
        m_tray->showMessage(APP_NAME, out);
}

void FenPrincipale::clipboardChange()
{
    QStringList clipboard = QApplication::clipboard()->text().simplified().split(' ');
    QStringList urls, newUrls;
    foreach(UrlItem info, m_adresses)
    {
        urls << info.url;
    }

    foreach (QString url, clipboard)
    {
        if (isMegauploadUrl(url) && !urls.contains(url, Qt::CaseInsensitive))
        {
            newUrls << url;
        }
    }

    if (!newUrls.isEmpty())
    {
        ui->adresse->setText(newUrls.join(" "));
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
                           "Lien de téléchargement: " + url);
    msgBox.exec();

}

void FenPrincipale::infoAvailable(QString url, QString name, QString description, QString size)
{
    for (int i = 0; i < m_adresses.size(); i++)
    {
        UrlItem &urlItem = m_adresses[i];
        if (urlItem.url == url)
        {
            urlItem.name = name;
            urlItem.description = description;
            urlItem.size = size;
            renameItem(url, name, "Description: " + description + "\nLien: " + url);

            if (ui->lienMegaupload->text() == url)  //Actualisation des infos si les détails de l'item sont affichés
            {
                ui->nomFichier->setText(name);
                ui->description->setText(description);
                ui->taille->setText(size);
            }

            return;
        }
    }
}

void FenPrincipale::infoUnavailable(QString url, bool temporary)
{
    if (temporary)
        renameItem(url, "Fichier temporairement indisponible (" + url + ")");
    else
        renameItem(url, "Fichier supprimé (" + url + ")");
}

void FenPrincipale::saveSettings()
{
    QStringList links;
    if (!m_currentDownload.isEmpty())
        links << m_currentDownload;
    foreach(UrlItem item, m_adresses)
    {
        links << item.url;
    }

    QSettings settings(APP_NAME, APP_ORGANIZATION);
    settings.setValue("links", links);
    settings.setValue("login", m_login);
    settings.setValue("password", m_password);
    settings.setValue("destDir", m_dir.path());
    settings.setValue("details", ui->btn_details->isChecked());
    settings.setValue("firstRun", false);
}

void FenPrincipale::loadSettings()
{
    QSettings settings(APP_NAME, APP_ORGANIZATION);
    QStringList links = settings.value("links").toStringList();
    foreach(QString link, links)
    {
        addItem(link);
    }
    m_login = settings.value("login").toByteArray();
    m_password = settings.value("password").toByteArray();
    m_dir = settings.value("destDir", QDir::home().absolutePath()).toString();
    ui->btn_details->setChecked(settings.value("details", false).toBool());
    if (settings.value("firstRun", true).toBool())
    {
        QMessageBox::information(this, APP_NAME, "Ceci est le premier lancement de l'application. Veuillez entrer vos paramètres !");
        on_options_clicked();
    }
}

void FenPrincipale::authSuccess(AuthInfo authInfo)
{
    console("Authentification réussie");

    ui->btn_go->setEnabled(true);
    m_handler->setAuthInfo(authInfo);
}

void FenPrincipale::authFail(AuthError err)
{
    console("Authenficication échouée");

    switch (err)
    {
    case AUTH_INVALID_LOGIN:
    {
        console("Nom de compte ou mot de passe incorrect.");
        break;
    }
    case AUTH_CONNECTION_REFUSED:
    {
        console("L'hôte distant a refusé la connexion.");
        break;
    }
    case AUTH_REMOTE_HOST_CLOSED:
    {
        console("L'hôte distant a coupé la connexion prématurément");
        break;
    }
    case AUTH_HOST_NOT_FOUND:
    {
        console("Hôte non trouvé");
        break;
    }
    case AUTH_TEMP_NETWORK_FAILURE:
    {
        console("Défaillance temporaire du réseau");
        break;
    }
    case AUTH_NETWORK_ERROR:
    {
        console("Défaillance du réseau");
        break;
    }
    case AUTH_PROTOCOL_FAILURE:
    {
        console("Erreur de protocole");
        break;
    }
    case AUTH_UNDEFINED_ERROR:
    {
        console("Erreur indéfinie");
        break;
    }
    }
}

void FenPrincipale::startNextDownload()
{
    if (!m_adresses.isEmpty() || !m_currentDownload.isEmpty())
    {
        //Téléchargement suivant seulement si aucun téléchargement n'est en cours.
        if (m_currentDownload.isEmpty())
        {
            m_currentDownload = m_adresses.first().url;
            removeItem(0);
            on_liste_currentRowChanged(ui->liste->currentRow());    //Corrige l'affichage d'informations de fichier incorrectes
        }

        ui->btn_go->hide();
        ui->btn_arreter->show();

        m_handler->setDir(m_dir);
        m_handler->download(QUrl(m_currentDownload));
        m_vitesseTransfert->start();
        console("Téléchargement de: " + m_currentDownload);
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
        console("Téléchargements terminés !");
    }
}

void FenPrincipale::updateDownload(qint64 pos, qint64 total)
{
    if (m_isDownloading)  //Evite les vitesses très élevées affichées lors de la reprise d'un téléchargement
        *m_vitesseTransfert << (pos - ui->progression->value()); //Différence = nbr d'octets écrits
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
        console("Erreur: erreur de connexion pour récupérer le lien pour " + m_currentDownload);
        break;
    }
    case LINK_NOT_FOUND:
    {
        console("Erreur: lien de téléchargement non trouvé (lien invalide ou supprimé) pour " + m_currentDownload);
        startNextDownload();
        break;
    }
    case FILE_COULD_NOT_BE_OPENED:
    {
        console("Erreur: le fichier de destination n'a pas pu être ouvert !");
        break;
    }
    case FILE_CORRUPT_RESTART_DOWNLOAD:
    {
        console("Erreur: le fichier de destination est corrompu, redémarrage du téléchargement !");
        break;
    }
    case DOWNLOAD_LIMIT_REACHED:
    {
        console("Erreur: la limite de transferts a été atteinte !");
        break;
    }
    case DOWNLOAD_NETWORK_ERROR:
    {
        console("Erreur: téléchargement interrompu !");
        break;
    }
    case DOWNLOAD_EMPTY:
    {
        console("Erreur: téléchargement vide ! (erreur de connexion)");
        break;
    }
    case DOWNLOAD_SOCKET_TIMEOUT:
    {
        console("Erreur: timeout de la connexion !");
        break;
    }
    default:
    {
        console("Erreur: " + pNbr(error));
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

void FenPrincipale::addItem(const QString &url)
{
    QListWidgetItem *item = new QListWidgetItem(url);

    UrlItem urlItem;
    urlItem.url = url;
    urlItem.name = QString();
    urlItem.description = QString();
    urlItem.size = QString();
    urlItem.item = item;

    m_adresses.push_back(urlItem);
    ui->liste->addItem(item);
    m_infoExtractor->queue(url);
}

void FenPrincipale::removeItem(const int &row)
{
    Q_ASSERT(row >= 0);
    QListWidgetItem *item = m_adresses[row].item;
    ui->liste->removeItemWidget(item);
    m_adresses.removeAt(row);
    delete item;
}

void FenPrincipale::renameItem(const QString &url, const QString &label, const QString &tip)
{
    QListWidgetItem *item = NULL;
    foreach(UrlItem itr, m_adresses)
    {
        if (itr.url == url)
        {
            item = itr.item;
            break;
        }
    }

    Q_ASSERT(item);
    item->setText(label);
    item->setToolTip(tip);
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

void FenPrincipale::setDetailsVisible(bool visible)
{
    if (visible)
    {
        ui->labelNomDeFichier->show();
        ui->labelDescription->show();
        ui->labelTaille->show();
        ui->labelLienMegaupload->show();
        ui->nomFichier->show();
        ui->description->show();
        ui->taille->show();
        ui->lienMegaupload->show();
    }
    else
    {
        ui->labelNomDeFichier->hide();
        ui->labelDescription->hide();
        ui->labelTaille->hide();
        ui->labelLienMegaupload->hide();
        ui->nomFichier->hide();
        ui->description->hide();
        ui->taille->hide();
        ui->lienMegaupload->hide();
    }
}

//Convertit une taille de fichier en une chaîne avec l'extention o, ko, Mo, Go, ...
QString FenPrincipale::sizeToString(quint64 nbr)
{
    quint8 exp = 0;
    while (nbr > 1024)
    {
        exp++;
        nbr /= 1024;
    }

    QString ret = pNbr(nbr);

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
