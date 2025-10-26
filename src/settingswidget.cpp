#include "settingswidget.h"
#include <QApplication>
#include "settingsmanager.h"
#include <QMessageBox>
#include <QTextBrowser>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QScrollBar>
#include <QScreen>

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
    QDialog *dialog = createChangelogDialog();
    dialog->exec();
    delete dialog;
}

QDialog* SettingsWidget::createChangelogDialog()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("更新日志");
    dialog->resize(600, 500);
    
    // 居中显示在屏幕中央
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - dialog->width()) / 2;
    int y = (screenGeometry.height() - dialog->height()) / 2;
    dialog->move(x, y);
    
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    
    // 创建标签页控件显示不同版本的更新日志
    QTabWidget *tabWidget = new QTabWidget;
    
    // v1.0.0 版本更新日志
    QTextBrowser *v100Log = new QTextBrowser;
    v100Log->setHtml(
        "<h3>v1.0.0 (2025-10-26)</h3>"
        "<h4>🆕 新增功能</h4>"
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

    tabWidget->addTab(v100Log, "v1.0.0");
    
    layout->addWidget(tabWidget);
    
    // 添加关闭按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::close);
    layout->addWidget(buttonBox);
    
    return dialog;
}
