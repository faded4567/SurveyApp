#include "settingswidget.h"
#include <QApplication>
#include "settingsmanager.h"


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
    // 这里为了简洁，直接发射信号

    SettingsManager::getInstance().saveToFile();
    
    // 通知主窗口设置已保存
    QApplication::processEvents();
}

void SettingsWidget::onBackClicked()
{
    emit backToMain();
}