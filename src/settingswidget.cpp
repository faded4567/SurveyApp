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

    
    surveySettingsLayout->addWidget(m_autoRecordCheckBox,0,0,1,3);
    
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
}

void SettingsWidget::saveSettings()
{
    SettingsManager::getInstance().setValue("survey/autoRecord", m_autoRecordCheckBox->isChecked());

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
