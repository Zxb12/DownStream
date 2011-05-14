#ifndef FENOPTIONS_H
#define FENOPTIONS_H

#include <QDialog>
#include <QDir>

namespace Ui
{
    class FenOptions;
}

class FenOptions : public QDialog
{
    Q_OBJECT

public:
    explicit FenOptions(QWidget *parent, QDir *dir, QByteArray *login, QByteArray *password);
    ~FenOptions();

private slots:
    void on_btn_parcourir_clicked();
    void saveOptions();

private:
    Ui::FenOptions *ui;

    QDir *m_dir;
    QByteArray *m_login, *m_password;
};

#endif // FENOPTIONS_H
