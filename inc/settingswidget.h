#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = nullptr);

signals:
    void backToMain();

private slots:
    void onSaveClicked();
    void onBackClicked();

private:
    void loadSettings();
    void saveSettings();

    QVBoxLayout *m_mainLayout;
    QPushButton *m_backButton;
    
    // 答题设置相关控件
    QGroupBox *m_surveySettingsGroup;
    QCheckBox *m_autoRecordCheckBox;
    QCheckBox *m_autoCaptureCheckBox; // 添加自动拍照选项
    QLabel *m_captureIntervalLabel;
    QSpinBox *m_captureIntervalSpinBox; // 添加拍照时间间隔输入框

};

#endif // SETTINGSWIDGET_H