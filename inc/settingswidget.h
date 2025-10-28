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
#include <QDialog>
#include <QTextEdit>
#include <QTabWidget>

// 自定义对话框类，用于显示更新日志
class ChangelogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangelogDialog(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void centerOnScreen();
    QWidget *m_contentWidget;
};

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
    void onShowChangelogClicked();

private:
    void loadSettings();
    void saveSettings();
    ChangelogDialog* createChangelogDialog();

    QVBoxLayout *m_mainLayout;
    QPushButton *m_backButton;
    
    // 答题设置相关控件
    QGroupBox *m_surveySettingsGroup;
    QCheckBox *m_autoRecordCheckBox;
    QCheckBox *m_autoCaptureCheckBox; // 添加自动拍照选项
    QLabel *m_captureIntervalLabel;
    QSpinBox *m_captureIntervalSpinBox; // 添加拍照时间间隔输入框
    
    // 更新日志按钮
    QPushButton *m_changelogButton;
};

#endif // SETTINGSWIDGET_H