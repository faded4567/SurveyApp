#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QObject>
#include <QCoreApplication>
#include <QPainter>
#include <QPixmap>
#include <QTimer>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

signals:
    void loginRequested(const QString& username, const QString& password);
    void registerRequested(const QString& username, const QString& password);
    void captchaRefreshRequested();

public slots:
    void InitManager();
    void onLoginClicked();
    void onRegisterClicked();
    void onCaptchaImageClicked();
    void onLoginSuccess(const QJsonObject& userInfo);
    void onLoginFailed(const QString& error);
    void onRegisterSuccess(const QJsonObject& userInfo);
    void onRegisterFailed(const QString& error);
    void onCaptchaReceived(const QString& captchaId, const QPixmap& captchaImage);


protected:
    bool eventFilter(QObject *watched, QEvent *event);

private slots:

private:
    Ui::LoginDialog *ui;
    QString m_captchaId;


};

#endif // LOGINDIALOG_H
