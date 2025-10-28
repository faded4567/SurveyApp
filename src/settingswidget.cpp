#include "settingswidget.h"
#include <QApplication>
#include "settingsmanager.h"
#include <QMessageBox>
#include <QTextBrowser>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QScrollBar>
#include <QScreen>
#include <QMouseEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QRect>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QTimer>

// 自定义对话框实现
ChangelogDialog::ChangelogDialog(QWidget *parent) : QDialog(parent)
{
    // 设置窗口标志，无边框和置顶
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setModal(true);
    
    // 设置背景色
    setStyleSheet("QDialog { background-color: white; border: 1px solid gray; border-radius: 10px; }");
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 创建标题栏
    QWidget *titleBar = new QWidget;
    titleBar->setStyleSheet("background-color: #4A90E2; border-radius: 5px;");
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 5, 10, 5);
    
    QLabel *titleLabel = new QLabel("更新日志");
    titleLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    titleLayout->addWidget(titleLabel);
    
    QPushButton *closeButton = new QPushButton("×");
    closeButton->setFixedSize(30, 30);
    closeButton->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   color: white;"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 0.2);"
        "   border-radius: 15px;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    titleLayout->addWidget(closeButton);
    
    mainLayout->addWidget(titleBar);
    
    // 创建内容区域
    m_contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建标签页控件显示不同版本的更新日志
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: 1px solid #ccc;"
        "   border-radius: 5px;"
        "}"
        "QTabBar::tab {"
        "   background: #f0f0f0;"
        "   padding: 8px 12px;"
        "   margin: 2px;"
        "   border-radius: 3px;"
        "}"
        "QTabBar::tab:selected {"
        "   background: #4A90E2;"
        "   color: white;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "   background: #ddd;"
        "}"
    );
    
    
    // v0.1.0 版本更新日志
    QTextBrowser *v010Log = new QTextBrowser;
    v010Log->setOpenExternalLinks(true);
    v010Log->setStyleSheet("QTextBrowser { font-size: 14px; }");
    v010Log->setHtml(
        "<h3 style='color: #2c3e50; margin-top: 10px;'>v1.0.0 (2025-10-26)</h3>"
        "<h4 style='color: #3498db;'>🆕 新增功能</h4>"
        "<ul>"
        "<li>添加问卷基本功能，适配基本的问卷题型</li>"
        "<li>用户登陆功能</li>"
        "<li>适配每页一题</li>"
        "<li>问卷列表界面支持下拉刷新</li>"
        "<li>添加基本的逻辑跳转功能，适配中途结束逻辑</li>"
        "<li>支持选项题中有自定义文本上传</li>"
        "<li>添加用户信息显示页面</li>"
        "<li>添加题干说明显示</li>"
        "<li>支持默认隐藏题目</li>"
        "<li>添加问卷设置页面</li>"
        "<li>支持问卷设置开启自动录音以及照相（可设置时间间隔），提交时将缓存上传</li>"
        "<li>添加应用的定位功能，上传时也会上传位置的经纬度</li>"
        "</ul>"
    );

    // v0.1.1 版本更新日志
    QTextBrowser *v011Log = new QTextBrowser;
    v011Log->setOpenExternalLinks(true);
    v011Log->setStyleSheet("QTextBrowser { font-size: 14px; }");
    v011Log->setHtml(
        "<h3 style='color: #2c3e50; margin-top: 10px;'>v1.0.0 (2025-10-26)</h3>"
        "<h4 style='color: #3498db;'>🆕 新增功能</h4>"
        "<ul>"
        "<li>增加用户操作以及网络请求记录日志功能</li>"
        "</ul>"
        "<h4 style='color: #3498db;'>⛑ 问题修复</h4>"
        "<ul>"
        "</ul>"
        );

    tabWidget->addTab(v010Log, "v0.1.0");
    
    contentLayout->addWidget(tabWidget);
    
    mainLayout->addWidget(m_contentWidget);
    
    // 设置合适的大小
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    int width = qMin(600, screenSize.width() - 20);
    int height = qMin(500, screenSize.height() - 20);
    resize(width, height);
    
    // 居中显示
    centerOnScreen();
}

void ChangelogDialog::mousePressEvent(QMouseEvent *event)
{
    // 如果点击的是对话框外部，则关闭对话框
    if (!m_contentWidget->geometry().contains(event->pos())) {
        accept();
    } else {
        QDialog::mousePressEvent(event);
    }
}

void ChangelogDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // 调整大小时重新居中
    centerOnScreen();
}

void ChangelogDialog::centerOnScreen()
{
    // 获取屏幕大小
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(screenGeometry.topLeft() + QPoint(x, y));
}


SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    
    // 答题设置组
    m_surveySettingsGroup = new QGroupBox("答题设置");
    
    QGridLayout *surveySettingsLayout = new QGridLayout(m_surveySettingsGroup);
    

    // 自动录音选项
    m_autoRecordCheckBox = new QCheckBox("答题时自动录音");
    
    // 自动拍照选项
    m_autoCaptureCheckBox = new QCheckBox("答题时自动拍照");
    
    // 拍照时间间隔设置
    m_captureIntervalLabel = new QLabel("拍照时间间隔（秒）:");
    m_captureIntervalSpinBox = new QSpinBox;
    m_captureIntervalSpinBox->setRange(10, 300); // 10秒到5分钟
    m_captureIntervalSpinBox->setValue(30); // 默认30秒
    m_captureIntervalSpinBox->setSuffix(" 秒");

    
    surveySettingsLayout->addWidget(m_autoRecordCheckBox,0,0,1,3);
    surveySettingsLayout->addWidget(m_autoCaptureCheckBox,1,0,1,3);
    surveySettingsLayout->addWidget(m_captureIntervalLabel,2,0);
    surveySettingsLayout->addWidget(m_captureIntervalSpinBox,2,1,1,2);
    
    m_mainLayout->addWidget(m_surveySettingsGroup);
    
    // 更新日志按钮
    m_changelogButton = new QPushButton("查看更新日志");
    m_changelogButton->setStyleSheet("QPushButton {"
                             "background-color: #4A90E2;"
                             "color: white;"
                             "border: none;"
                             "border-radius: 8px;"
                             "padding: 12px 30px;"
                             "font-size: 15px;"
                             "font-weight: bold;"
                             "margin: 10px;"
                             "}"
                             "QPushButton:hover {"
                             "background-color: #5fa0f0;"
                             "}"
                             "QPushButton:pressed {"
                             "background-color: #3a7bc8;"
                             "}");
    connect(m_changelogButton, &QPushButton::clicked, this, &SettingsWidget::onShowChangelogClicked);
    
    QHBoxLayout *changelogLayout = new QHBoxLayout;
    changelogLayout->addStretch();
    changelogLayout->addWidget(m_changelogButton);
    changelogLayout->addStretch();
    m_mainLayout->addLayout(changelogLayout);
    
    // 保存按钮
    QPushButton *saveButton = new QPushButton("保存设置");
    saveButton->setStyleSheet("QPushButton {"
                             "background-color: #4A90E2;"
                             "color: white;"
                             "border: none;"
                             "border-radius: 8px;"
                             "padding: 12px 30px;"
                             "font-size: 15px;"
                             "font-weight: bold;"
                             "margin: 10px;"
                             "}"
                             "QPushButton:hover {"
                             "background-color: #5fa0f0;"
                             "}"
                             "QPushButton:pressed {"
                             "background-color: #3a7bc8;"
                             "}");
    connect(saveButton, &QPushButton::clicked, this, &SettingsWidget::onSaveClicked);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addStretch();

    SettingsManager::getInstance().loadFromFile();
    
    // 加载设置
    loadSettings();
}

void SettingsWidget::loadSettings()
{
    m_autoRecordCheckBox->setChecked(SettingsManager::getInstance().getValue("survey/autoRecord").toBool());
    m_autoCaptureCheckBox->setChecked(SettingsManager::getInstance().getValue("survey/autoCapture").toBool());
    m_captureIntervalSpinBox->setValue(SettingsManager::getInstance().getValue("survey/captureInterval", 30).toInt());
}

void SettingsWidget::saveSettings()
{
    SettingsManager::getInstance().setValue("survey/autoRecord", m_autoRecordCheckBox->isChecked());
    SettingsManager::getInstance().setValue("survey/autoCapture", m_autoCaptureCheckBox->isChecked());
    SettingsManager::getInstance().setValue("survey/captureInterval", m_captureIntervalSpinBox->value());
}

void SettingsWidget::onSaveClicked()
{
    saveSettings();
    
    // 显示保存成功的提示（可以使用 QMessageBox 或其他方式）
    QMessageBox::information(this, "设置保存", "设置已成功保存！");
    
    SettingsManager::getInstance().saveToFile();
    
    // 通知主窗口设置已保存
    QApplication::processEvents();
}

void SettingsWidget::onBackClicked()
{
    emit backToMain();
}

void SettingsWidget::onShowChangelogClicked()
{
    ChangelogDialog *dialog = createChangelogDialog();
    dialog->exec();
    delete dialog;
}

ChangelogDialog* SettingsWidget::createChangelogDialog()
{
    ChangelogDialog *dialog = new ChangelogDialog(this);
    return dialog;
}
