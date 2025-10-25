#include "mainwindow.h"
#include "logindialog.h"
#include "surveylistwidget.h"
#include "surveyformwidget.h"
#include "networkmanager.h"
#include "logfilemanager.h"
#include "dashboardwidget.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtCore/private/qandroidextras_p.h>
#include <QFuture>
#include <QFutureWatcher>
#include <QThread>
#include <QScrollArea>
#include <QScroller>
#include <QScrollerProperties>
#include <QGesture>
#include <QPanGesture>
#include "permissionmanager.h"
// 定义应用名称常量
static const QString APP_NAME = "SurveyKing客户端";


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    InitLog();
    requestInitialPermissions();
    setupUi();
    setupConnections();
    
    // 设置整体样式表
    setStyleSheet("QMainWindow { background-color: #f0f8ff; font-family: 'Segoe UI', Arial, sans-serif; }"
                  "QMenuBar { background-color: #4A90E2; color: white; border-bottom: 1px solid #3a7bc8; }"
                  "QMenuBar::item { background-color: transparent; padding: 8px 12px; }"
                  "QMenuBar::item:selected { background-color: #5fa0f0; }"
                  "QMenuBar::item:pressed { background-color: #3a7bc8; }"
                  "QMenu { background-color: white; border: 1px solid #b0d4e3; border-radius: 4px; }"
                  "QMenu::item { padding: 6px 20px; }"
                  "QMenu::item:selected { background-color: #e1f0fa; }"
                  "QStatusBar { background-color: #e1f0fa; border-top: 1px solid #b0d4e3; }"
                  "QStatusBar QLabel { color: #555555; }"
                  "QMessageBox { background-color: white; }");

    // 启用触摸手势支持
    grabGesture(Qt::PanGesture);
    setAttribute(Qt::WA_AcceptTouchEvents);
    
    // 启用 Qt Scroller 支持，改善移动端触摸体验
    QScroller *scroller = QScroller::scroller(this);
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    sp.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    sp.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.05); // 更平滑的拖拽体验
    sp.setScrollMetric(QScrollerProperties::MinimumVelocity, 0.0);
    sp.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.8); // 提高最大速度
    sp.setScrollMetric(QScrollerProperties::AcceleratingFlickMaximumTime, 0.5); // 更长的滑动时间
    sp.setScrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor, 2.0); // 更快的加速
    sp.setScrollMetric(QScrollerProperties::SnapPositionRatio, 0.02); // 更精确的位置停靠
    sp.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, 0);
    sp.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);
    sp.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.1); // 减少点击延迟
    sp.setScrollMetric(QScrollerProperties::ScrollMetricCount, 1.0); // 启用惯性滚动
    sp.setScrollMetric(QScrollerProperties::AxisLockThreshold, 0.5); // 轴锁定阈值
    scroller->setScrollerProperties(sp);
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUi()
{
    setWindowTitle(APP_NAME);
    resize(800, 600);

    // 创建菜单栏
    QMenu *fileMenu = menuBar()->addMenu("文件");

    QAction *logoutAction = fileMenu->addAction("退出登录");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);

    QAction *exitAction = fileMenu->addAction("退出");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // 创建状态栏
    statusBar()->showMessage("准备就绪");
    m_statusLabel = new QLabel;
    statusBar()->addPermanentWidget(m_statusLabel);

    // 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    NetworkManager &nm =  NetworkManager::instance();
    QThread *thread = new QThread(this);
    nm.moveToThread(thread);

    // 创建登录页面
    m_loginDialog = new LoginDialog;
    
    // 创建仪表板页面
    m_dashboardWidget = new DashboardWidget;

    // 连接网络管理器的信号
    connect(&nm, &NetworkManager::loginSuccess, m_loginDialog, &LoginDialog::onLoginSuccess);
    connect(&nm, &NetworkManager::loginFailed, m_loginDialog, &LoginDialog::onLoginFailed);
    connect(&nm, &NetworkManager::registerSuccess, m_loginDialog, &LoginDialog::onRegisterSuccess);
    connect(&nm, &NetworkManager::registerFailed, m_loginDialog, &LoginDialog::onRegisterFailed);
    connect(&nm, &NetworkManager::captchaReceived, m_loginDialog, &LoginDialog::onCaptchaReceived);
    connect(thread, &QThread::started, &nm, &NetworkManager::InitManager);
    connect(m_loginDialog, &LoginDialog::captchaRefreshRequested, &nm, &NetworkManager::onRefreshCaptcha);

    // 连接网络管理器的信号
    connect(&nm, &NetworkManager::loginFailed, [this](const QString& error) {
        QMessageBox::warning(this, "登录失败", error);
    });

    connect(&nm, &NetworkManager::registerSuccess, this, &MainWindow::onLoginSuccess);
    connect(&nm, &NetworkManager::registerFailed, [this](const QString& error) {
        QMessageBox::warning(this, "注册失败", error);
    });

    connect(&nm, &NetworkManager::surveySchemaReceived, this, &MainWindow::onSurveySchemaReceived);
    connect(&nm, &NetworkManager::submitSuccess, this, &MainWindow::onSubmitSuccess);
    connect(&nm, &NetworkManager::submitFailed, this, &MainWindow::onSubmitFailed);
    connect(&nm, &NetworkManager::networkError, this, &MainWindow::onNetworkError);
    
    // 连接仪表板相关的信号
    connect(&nm, &NetworkManager::currentUserReceived, this, &MainWindow::onLoginSuccess);
    connect(&nm, &NetworkManager::projectListReceived, this, &MainWindow::onProjectListReceived);

    connect(this, &MainWindow::Signallogin,&nm, &NetworkManager::login);
    connect(this, &MainWindow::SignalregisterUser,&nm, &NetworkManager::registerUser);
    connect(this, &MainWindow::SignalgetSurveySchema,&nm, &NetworkManager::getSurveySchema);
    connect(this, &MainWindow::SignalsubmitResponse,&nm, &NetworkManager::submitResponse);
    connect(this, &MainWindow::SignalSetAuth,&nm, &NetworkManager::setAuthToken);
    connect(this, &MainWindow::SignalgetProjectList, &nm, &NetworkManager::getProjectList);
    connect(this, &MainWindow::SignalgetCurrentUser, &nm, &NetworkManager::getCurrentUser);

    thread->start();

    // 初始显示登录页面
    showLoginDialog();
}

void MainWindow::setupConnections()
{
    connect(m_loginDialog, &LoginDialog::loginRequested, this, &MainWindow::onLoginRequested);
    connect(m_loginDialog, &LoginDialog::registerRequested, this, &MainWindow::onRegisterRequested);
    
    // 连接仪表板的信号
    connect(m_dashboardWidget, &DashboardWidget::projectSelected, this, &MainWindow::onSurveySelected);
    connect(m_dashboardWidget, &DashboardWidget::refreshRequested, this, [this]() {
        emit SignalgetProjectList();
    });
    connect(m_dashboardWidget, &DashboardWidget::logoutRequested, this, &MainWindow::onLogout);
}

void MainWindow::showLoginDialog()
{
    m_stackedWidget->addWidget(m_loginDialog);
    m_stackedWidget->setCurrentWidget(m_loginDialog);
    statusBar()->showMessage("请登录");
    // 初始刷新验证码
    emit m_loginDialog->captchaRefreshRequested();
}

void MainWindow::showDashboard()
{
    m_stackedWidget->addWidget(m_dashboardWidget);
    m_stackedWidget->setCurrentWidget(m_dashboardWidget);
    
    // 设置当前用户信息
    m_dashboardWidget->setCurrentUser(m_currentUser);
    
    // 获取项目列表
    emit SignalgetProjectList();
    statusBar()->showMessage("正在加载项目列表...");
}

void MainWindow::onLoginRequested(const QString& username, const QString& password)
{
    emit Signallogin(username, password);
    statusBar()->showMessage("正在登录...");
}

void MainWindow::onRegisterRequested(const QString& username, const QString& password)
{
    emit SignalregisterUser(username, password);
    statusBar()->showMessage("正在注册...");
}

void MainWindow::onLoginSuccess(const QJsonObject& userInfo)
{
    m_currentUser = userInfo;
    showDashboard();
}

void MainWindow::onProjectListReceived(const QJsonArray& projects)
{
    m_dashboardWidget->setProjects(projects);
    statusBar()->showMessage(QString("已加载 %1 个项目").arg(projects.size()));
}

void MainWindow::onSurveySelected(const QString& surveyId, const QString& title)
{
    m_currentSurveyId = surveyId;
    m_currentSurveyTitle = title;

    if (m_surveyFormWidget != nullptr) {
        m_stackedWidget->removeWidget(m_surveyFormWidget);
        delete m_surveyFormWidget;
        m_surveyFormWidget = nullptr;
    }

    m_surveyFormWidget = new SurveyFormWidget;
    m_stackedWidget->addWidget(m_surveyFormWidget);
    m_stackedWidget->setCurrentWidget(m_surveyFormWidget);

    // 连接 NetworkManager 的文件上传信号
    connect(m_surveyFormWidget, &SurveyFormWidget::submitSurvey, this, &MainWindow::onSubmitResponse);
    connect(m_surveyFormWidget, &SurveyFormWidget::UploadFile, &NetworkManager::instance(), &NetworkManager::uploadFile);
    connect(&NetworkManager::instance(), &NetworkManager::fileUploadSuccess, m_surveyFormWidget, &SurveyFormWidget::handleUploadSuccsee);
    connect(&NetworkManager::instance(), &NetworkManager::fileUploadFailed, m_surveyFormWidget, &SurveyFormWidget::handleUploadFailed);
    
    // 连接返回问卷列表信号
    connect(m_surveyFormWidget, &SurveyFormWidget::backToSurveyList, this, &MainWindow::onBackToSurveyList);

    connect(m_surveyFormWidget, &SurveyFormWidget::startSurvey, this, [this]() {
        statusBar()->showMessage(QString("正在填写问卷: %1").arg(m_currentSurveyTitle));
    });

    emit SignalgetSurveySchema(surveyId);
    statusBar()->showMessage(QString("正在加载问卷: %1").arg(title));
}

void MainWindow::onSurveySchemaReceived(const QJsonObject& schema)
{
    if (m_surveyFormWidget) {
        m_surveyFormWidget->setSurveySchema(schema);
        statusBar()->showMessage(QString("正在填写问卷: %1").arg(m_currentSurveyTitle));
    }
}

void MainWindow::onSubmitResponse(const QJsonObject& data)
{
    emit SignalsubmitResponse(m_currentSurveyId, data, m_surveyFormWidget->GetDiffTime());
    statusBar()->showMessage("正在提交问卷...");
}

void MainWindow::onSubmitSuccess()
{
    // 创建一个无按钮的提示对话框
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setWindowTitle("提交成功");
    msgBox->setText("问卷提交成功！");
    msgBox->setIcon(QMessageBox::Information);
    msgBox->setStandardButtons(QMessageBox::NoButton); // 不显示任何按钮

    // 显示对话框
    msgBox->show();

    // 2秒后自动关闭对话框并返回仪表板
    QTimer::singleShot(2000, this, [this, msgBox]() {
        msgBox->close();
        delete msgBox;

        // 返回仪表板
        m_stackedWidget->setCurrentWidget(m_dashboardWidget);
        statusBar()->showMessage("问卷提交成功！");
    });
}

void MainWindow::onSubmitFailed(const QString& error)
{
    QMessageBox::warning(this, "提交失败", error);
    statusBar()->showMessage("问卷提交失败");
}

void MainWindow::onNetworkError(const QString& error)
{
    QMessageBox::warning(this, "网络错误", error);
    statusBar()->showMessage("网络连接错误");
}

void MainWindow::onLogout()
{
    // 清除认证信息
    emit SignalSetAuth("");

    // 清除用户信息
    m_currentUser = QJsonObject();

    // 返回登录界面
    showLoginDialog();
}

void MainWindow::InitLog()
{

    // QString requiredPermissions;
    // requiredPermissions = "android.permission.READ_EXTERNAL_STORAGE";
    // auto future = QtAndroidPrivate::requestPermission(requiredPermissions);
    // QFutureWatcher<QtAndroidPrivate::PermissionResult> *watcher = new QFutureWatcher<QtAndroidPrivate::PermissionResult>();
    // connect(watcher, &QFutureWatcher<QtAndroidPrivate::PermissionResult>::finished, this, [this, watcher]() {
    //     QtAndroidPrivate::PermissionResult result = watcher->result();
    //     if (result == QtAndroidPrivate::Authorized) {
    //         // 权限被授予
    //         qDebug() << "Storage permission granted";
    //         // ... 执行需要权限的操作，例如初始化你的日志模块 ...
    //         // 初始化日志系统
    //         LogFileManager::instance().init();
    //         // 设置日志文件最大为5MB，保留30天
    //         LogFileManager::instance().setMaxSize(5 * 1024 * 1024);
    //         LogFileManager::instance().setMaxDays(30);

    //         qDebug() << "Application started";
    //         qInfo() << "This is an info message";
    //         qWarning() << "This is a warning message";
    //     } else {
    //         // 权限被拒绝
    //         qWarning() << "Storage permission denied";
    //         // ... 处理权限被拒绝的情况，可能需要对用户进行提示 ...
    //     }
    //     watcher->deleteLater();
    // });
    // watcher->setFuture(future);
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QMainWindow::event(event);
}

bool MainWindow::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pan = event->gesture(Qt::PanGesture)) {
        // 处理平移手势
        return true;
    }
    return false;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
#ifdef Q_OS_ANDROID
    if (event->key() == Qt::Key_Back) {
        // 处理返回键事件
        if (m_stackedWidget->currentIndex() == 1) { // 如果当前在问卷列表页面
            // 显示退出确认对话框
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "退出确认", "您确定要退出应用吗？",
                                          QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QMainWindow::close(); // 退出应用
            }
            // 如果用户选择"No"，则不执行任何操作，保持应用运行
        } else if (m_stackedWidget->currentIndex() == 2) { // 如果当前在问卷填写页面
            // 返回问卷列表页面
            m_stackedWidget->setCurrentIndex(1);
        } else {
            QMainWindow::keyPressEvent(event); // 其他情况使用默认处理
        }
        return;
    }
#endif
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onBackToSurveyList()
{
    // 返回问卷列表页面 (仪表板页面)
    m_stackedWidget->setCurrentWidget(m_dashboardWidget);
    statusBar()->showMessage("已返回问卷列表");
}

void MainWindow::requestInitialPermissions()
{
    // 请求录音权限
    PermissionManager::instance().requestAudioRecordingPermission([this](bool granted) {
        if (granted) {
            qDebug() << "Initial audio recording permission granted";
        } else {
            qDebug() << "Initial audio recording permission denied";
        }
        
        // 在录音权限请求完成后，再请求相机权限
        PermissionManager::instance().requestCameraPermission([this](bool granted) {
            if (granted) {
                qDebug() << "Initial camera permission granted";
            } else {
                qDebug() << "Initial camera permission denied";
            }
            
            // 在相机权限请求完成后，再请求定位权限
            PermissionManager::instance().requestLocationPermission([](bool granted) {
                if (granted) {
                    qDebug() << "Initial location permission granted";
                } else {
                    qDebug() << "Initial location permission denied";
                }
            });
        });
    });
}
