#include "fenoptions.h"
#include "ui_fenoptions.h"
#include "enums.h"

#include <QFileDialog>

FenOptions::FenOptions(QWidget *parent, QDir *dir, QByteArray *login, QByteArray *password) :
    QDialog(parent), ui(new Ui::FenOptions), m_dir(dir), m_login(login), m_password(password)
{
    ui->setupUi(this);
    setWindowTitle(APP_NAME);

    connect(this, SIGNAL(accepted()), this, SLOT(saveOptions()));

    ui->dossier->setText(dir->path());
    ui->login->setText(*m_login);
    ui->password->setText(*m_password);
}

FenOptions::~FenOptions()
{
    delete ui;
}

void FenOptions::on_btn_parcourir_clicked()
{
    QString dossier = QFileDialog::getExistingDirectory(this, "Sélectionnez le dossier de destination", ui->dossier->text());
    if (!dossier.isEmpty())
        ui->dossier->setText(dossier);
}

void FenOptions::saveOptions()
{
    *m_dir = ui->dossier->text();
    *m_login = ui->login->text().toUtf8();
    if (!m_login->isEmpty())
        *m_password = ui->password->text().toUtf8();
    else
        *m_password = "";
}
