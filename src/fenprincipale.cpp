#include "fenprincipale.h"
#include "ui_fenprincipale.h"

#include <QTime>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QShortcut>

FenPrincipale::FenPrincipale(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::FenPrincipale), m_fenOptions(NULL), m_trayWarningShown(false), m_auth(new Auth(this)), m_handler(new DownloadHandler(this)),
    m_infoExtractor(new InfoExtractor(this)), m_vitesseTransfert(new VitesseTransfert(this, DOWNLOAD_SPEED_UPDATE_INTERVAL, DOWNLOAD_SPEED_AVERAGE_TIME)),
    m_versionCheck(new VersionCheckThread(this, VERSION_HOST, APP_NAME, VERSION, VERSION_NBR)), m_currentDownload(), m_waitTimer(new QTimer(this)),
    m_updateDownloadTimer(new QTimer(this)), m_waitTime(0), m_isDownloading(false)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME " - v" VERSION);

    //Icône système
    m_menu = new QMenu(this);
    m_retablirAction = new QAction("Rétablir", m_menu);
    m_startAction = new QAction("Télécharger", m_menu);
    m_stopAction = new QAction("Arrêter", m_menu);
    m_quitterAction = new QAction("Quitter", m_menu);
    m_tray = new QSystemTrayIcon(windowIcon(), this);

    m_menu->addAction(m_retablirAction);
    m_menu->addAction(m_startAction);
    m_menu->addAction(m_stopAction);
    m_menu->addAction(m_quitterAction);
    m_retablirAction->setVisible(false);
    m_stopAction->setVisible(false);
    m_tray->setContextMenu(m_menu);
    m_tray->show();

    qApp->setActiveWindow(this);
    loadSettings();

    //UI
    connect(ui->adresse, SIGNAL(returnPressed()), this, SLOT(on_btn_ajouter_clicked()));
    connect(m_waitTimer, SIGNAL(timeout()), this, SLOT(waitTimerTick()));
    connect(m_infoExtractor, SIGNAL(infoAvailable(QString,QString,QString,QString)), this, SLOT(infoAvailable(QString,QString,QString,QString)));
    connect(m_infoExtractor, SIGNAL(infoUnavailable(QString,ExtractionError)), this, SLOT(infoUnavailable(QString,ExtractionError)));
    connect(m_updateDownloadTimer, SIGNAL(timeout()), this, SLOT(updateDownloadTick()));
    connect(m_versionCheck, SIGNAL(update(QString, QString)), this, SLOT(updateAvailable(QString, QString)));
    connect(m_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    connect(m_retablirAction, SIGNAL(triggered()), this, SLOT(retablir()));
    connect(m_startAction, SIGNAL(triggered()), this, SLOT(on_btn_go_clicked()));
    connect(m_stopAction, SIGNAL(triggered()), this, SLOT(on_btn_arreter_clicked()));
    connect(m_quitterAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardChange()));

    //Raccourcis
    connect(new QShortcut(QKeySequence("Ctrl+Q"), this), SIGNAL(activated()), qApp, SLOT(quit()));
    connect(new QShortcut(QKeySequence("Ctrl+D"), this), SIGNAL(activated()), ui->btn_details, SLOT(toggle()));
    connect(new QShortcut(QKeySequence("Ctrl+Up"), this), SIGNAL(activated()), this, SLOT(on_btn_monter_clicked()));
    connect(new QShortcut(QKeySequence("Ctrl+Shift+Up"), this), SIGNAL(activated()), this, SLOT(on_btn_monter_clicked()));
    connect(new QShortcut(QKeySequence("Ctrl+Down"), this), SIGNAL(activated()), this, SLOT(on_btn_descendre_clicked()));
    connect(new QShortcut(QKeySequence("Ctrl+Shift+Down"), this), SIGNAL(activated()), this, SLOT(on_btn_descendre_clicked()));
    connect(new QShortcut(QKeySequence("Del"), this), SIGNAL(activated()), this, SLOT(on_btn_supprimer_clicked()));

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
    saveSettings();
    m_handler->stopDownload();
    if (m_versionCheck->isRunning())
        m_versionCheck->terminate();

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
    foreach(DownloadInfo info, m_adresses)
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

    //Sauvegarde des liens
    saveSettings();
}

void FenPrincipale::on_btn_supprimer_clicked()
{
    QList<QListWidgetItem *> selection = ui->liste->selectedItems();
    for (int i = m_adresses.size() - 1; i >= 0; i--)
    {
        if (selection.contains(m_adresses[i].item))
            removeItem(i);
    }

    //Sauvegarde des liens
    saveSettings();
}

void FenPrincipale::on_btn_monter_clicked()
{
    QList<QListWidgetItem *> selection = ui->liste->selectedItems();
    QList<int> rows;
    for (int i = 0; i < m_adresses.size(); i++)
    {
        if (selection.contains(m_adresses[i].item))
            rows << i;
    }

    ui->liste->hide();  //On cache la liste pour améliorer les performances du traitement
    if (!rows.isEmpty() && rows.first() > 0)
    {
        foreach(int row, rows)
        {
            moveItem(row, row - 1);
            ui->liste->setCurrentRow(row - 1, QItemSelectionModel::Select);
        }
    }
    ui->liste->show();
}

void FenPrincipale::on_btn_descendre_clicked()
{
    QList<QListWidgetItem *> selection = ui->liste->selectedItems();
    QList<int> rows;
    for (int i = m_adresses.size() - 1; i >= 0; i--)
    {
        if (selection.contains(m_adresses[i].item))
            rows << i;
    }

    ui->liste->hide();  //On cache la liste pour améliorer les performances du traitement
    if (!rows.isEmpty() && rows.first() < m_adresses.size() - 1)
    {
        foreach(int row, rows)
        {
            moveItem(row, row + 1);
            ui->liste->setCurrentRow(row + 1, QItemSelectionModel::Select);
        }
    }
    ui->liste->show();
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
    m_startAction->setVisible(true);
    m_stopAction->setVisible(false);

    ui->progression->reset();
    ui->btn_go->show();
    ui->btn_arreter->hide();
}

void FenPrincipale::on_btn_details_toggled(bool visible)
{
    setDetailsVisible(visible);
}

void FenPrincipale::on_btn_options_clicked()
{
    m_fenOptions = new FenOptions(this, &m_dir, &m_login, &m_password);
    connect(m_fenOptions, SIGNAL(accepted()), this, SLOT(settingsChanged()));
    m_fenOptions->show();
}

void FenPrincipale::on_liste_currentRowChanged(int row)
{
    if (row >= 0 && row < m_adresses.size())
    {
        DownloadInfo item = m_adresses[row];
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
    m_tray->setToolTip(APP_NAME "\n"
                       "Dernier message: " + out);

    if (!qApp->focusWidget())
        m_tray->showMessage(APP_NAME, out);
}

void FenPrincipale::clipboardChange()
{
    QStringList clipboard = QApplication::clipboard()->text().simplified().split(' ');
    QStringList urls, newUrls;
    foreach(DownloadInfo info, m_adresses)
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
        DownloadInfo &downloadInfo = m_adresses[i];
        if (downloadInfo.url == url)
        {
            downloadInfo.name = name;
            downloadInfo.description = description;
            downloadInfo.size = size;
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

void FenPrincipale::infoUnavailable(QString url, ExtractionError erreur)
{
    switch(erreur)
    {
    case FILE_DELETED:
        renameItem(url, "Fichier supprimé (" + url + ")");
        break;
    case INVALID_DATA:
        renameItem(url, "Données reçues invalides (" + url + ") - fichier protégé par mot de passe ?");
        break;
    case NETWORK_ERROR:
        renameItem(url, "Erreur du réseau (" + url + ")");
        break;
    default:
        renameItem(url, "Impossible d'extraire les infos (" + url + ")");
        break;
    }

}

void FenPrincipale::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        retablir();
    }
}

void FenPrincipale::retablir()
{
    show();
    m_retablirAction->setVisible(false);
}

void FenPrincipale::saveSettings()
{
    QStringList links;
    if (!m_currentDownload.url.isEmpty())
        links << m_currentDownload.url;
    foreach(DownloadInfo info, m_adresses)
    {
        links << info.url;
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
        on_btn_options_clicked();
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
    if (!m_adresses.isEmpty() || !m_currentDownload.url.isEmpty())
    {
        //Téléchargement suivant seulement si aucun téléchargement n'est en cours.
        if (m_currentDownload.url.isEmpty())
        {
            m_currentDownload = m_adresses.first();
            removeItem(0);
            on_liste_currentRowChanged(ui->liste->currentRow());    //Corrige l'affichage d'informations de fichier incorrectes
        }

        ui->btn_go->hide();
        ui->btn_arreter->show();
        m_startAction->setVisible(false);
        m_stopAction->setVisible(true);

        m_handler->setDir(m_dir);
        m_handler->download(m_currentDownload.url);
        m_vitesseTransfert->start();
        console("Téléchargement de: " + m_currentDownload.printableName());
    }
    else
    {
        ui->statusBar->clearMessage();
        ui->btn_arreter->hide();
        ui->btn_go->show();
        ui->progression->setValue(0);
        ui->progression->setFormat("");
        m_startAction->setVisible(true);
        m_stopAction->setVisible(false);

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

    //Sauvegarde des liens
    saveSettings();
}

void FenPrincipale::error(DownloadError error)
{
    m_isDownloading = false;
    switch (error)
    {
    case LINK_NETWORK_ERROR:
    {
        console("Erreur: erreur de connexion pour récupérer le lien pour " + m_currentDownload.printableName());
        break;
    }
    case LINK_NOT_FOUND:
    {
        console("Erreur: lien de téléchargement non trouvé (lien invalide ou supprimé) pour " + m_currentDownload.printableName());
        addItem(m_currentDownload.url);
        m_currentDownload.url.clear();
        startNextDownload();
        break;
    }
    case PASSWORD_REQUIRED:
    {
        console("Erreur: le fichier à télécharger est protégé par mot de passe !");
        addItem(m_currentDownload.url);
        m_currentDownload.url.clear();
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

    DownloadItem downloadItem;
    downloadItem.url = url;
    downloadItem.name = QString();
    downloadItem.description = QString();
    downloadItem.size = QString();
    downloadItem.item = item;

    m_adresses.push_back(downloadItem);
    ui->liste->addItem(item);
    m_infoExtractor->queue(url);
}

void FenPrincipale::removeItem(const int &row)
{
    Q_ASSERT(row >= 0);
    QListWidgetItem *item = m_adresses[row].item;
    ui->liste->removeItemWidget(item);
    m_infoExtractor->remove(m_adresses[row].url);
    m_adresses.removeAt(row);
    delete item;
}

void FenPrincipale::renameItem(const QString &url, const QString &label, const QString &tip)
{
    QListWidgetItem *item = NULL;
    foreach(DownloadItem itr, m_adresses)
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

void FenPrincipale::moveItem(int i, int j)
{
    DownloadItem downloadItem = m_adresses.takeAt(i);
    ui->liste->takeItem(i);

    m_adresses.insert(j, downloadItem);
    ui->liste->insertItem(j, downloadItem.item);
}

void FenPrincipale::closeEvent(QCloseEvent *event)
{
    if (QApplication::keyboardModifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
        qApp->quit();

    if (!m_trayWarningShown)
    {
        m_tray->showMessage(APP_NAME, "DownStream a été réduit dans la barre des tâches.\n"
                            "Faites un clic droit sur l'icône pour ouvrir le menu.");
        m_trayWarningShown = true;
    }

    m_retablirAction->setVisible(true);
    hide();

    event->ignore();
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
