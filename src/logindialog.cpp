#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMouseEvent>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    m_captchaId("")
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("SurveyKing");

    // 设置验证码标签可点击
    ui->captchaImageLabel->installEventFilter(this);
    ui->captchaImageLabel->setCursor(Qt::PointingHandCursor);

    // 连接按钮信号
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    // 隐藏验证码相关
    ui->captchaImageLabel->setVisible(false);
    ui->captchaEdit->setVisible(false);
    ui->captchaLabel->setVisible(false);

    // 隐藏注册 暂时关闭注册功能
    ui->registerButton->setVisible(false);


}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::InitManager()
{
    // 获取
}


bool LoginDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->captchaImageLabel && event->type() == QEvent::MouseButtonPress) {
        onCaptchaImageClicked();
        return true;
    }
    return QDialog::eventFilter(watched, event);
}

void LoginDialog::onLoginClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
            ui->statusLabel->setText("请输入用户名、密码");
    }

    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    ui->statusLabel->setText("正在登录...");

    emit loginRequested(username, password);
}

void LoginDialog::onRegisterClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
            ui->statusLabel->setText("请输入用户名、密码");
    }

    if (password.length() < 6) {
            ui->statusLabel->setText("密码长度至少6位");
        return;
    }

    ui->loginButton->setEnabled(false);
    ui->registerButton->setEnabled(false);
    ui->statusLabel->setText("正在注册...");

    emit registerRequested(username, password);
}

void LoginDialog::onCaptchaImageClicked()
{
    // emit captchaRefreshRequested();
}

void LoginDialog::onLoginSuccess(const QJsonObject& userInfo)
{
    accept(); // 关闭对话框并返回Accepted
    LogFileManager::instance().logUserAction("Login", "Login successful");
}

void LoginDialog::onLoginFailed(const QString& error)
{
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->statusLabel->setText(error);
    // emit captchaRefreshRequested();
}

void LoginDialog::onRegisterSuccess(const QJsonObject& userInfo)
{
    accept(); // 关闭对话框并返回Accepted
}

void LoginDialog::onRegisterFailed(const QString& error)
{
    ui->loginButton->setEnabled(true);
    ui->registerButton->setEnabled(true);
    ui->statusLabel->setText(error);

    // emit captchaRefreshRequested();
}

void LoginDialog::onCaptchaReceived(const QString &captchaId, const QPixmap &captchaImage)
{
    m_captchaId = captchaId;
    
    // Check if the captcha image is valid
    if (captchaImage.isNull()) {
        // Set a default placeholder or clear the label
        ui->captchaImageLabel->setText("刷新验证码");
        ui->captchaImageLabel->setAlignment(Qt::AlignCenter);
    } else {
        ui->captchaImageLabel->setPixmap(captchaImage);
    }
}


