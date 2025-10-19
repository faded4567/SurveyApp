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
    

};

#endif // SETTINGSWIDGET_H
