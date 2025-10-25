#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>
#include <QGestureEvent>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include "settingswidget.h"

class DashboardWidget;
class SurveyFormWidget;
class NetworkManager;
class LoginDialog;
class SurveyListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void Signallogin(const QString& username, const QString& password);
    void SignalregisterUser(const QString& username, const QString& password);
    void SignalgetSurveyList();
    void SignalgetSurveySchema(const QString& surveyId);
    void SignalsubmitResponse(const QString& surveyId, const QJsonObject& data, qint64 diifTime);
    void SignalSetAuth(const QString& token);
    void SignalgetProjectList();
    void SignalgetCurrentUser();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


protected:
    // 添加事件处理函数
    bool event(QEvent *event) override;
    bool gestureEvent(QGestureEvent *event);
    void keyPressEvent(QKeyEvent *event) override;  // 添加这一行

private slots:
    void onSurveySelected(const QString& surveyId, const QString& title);
    void onLoginSuccess(const QJsonObject& userInfo);
    void onLogout();
    void onRegisterRequested(const QString& username, const QString& password);
    void onLoginRequested(const QString& username, const QString& password);
    void onProjectListReceived(const QJsonArray& projects);
    void onSurveySchemaReceived(const QJsonObject& schema);
    void onSubmitSuccess();
    void onSubmitFailed(const QString& error);
    void onNetworkError(const QString& error);
    void showLoginDialog();
    void showDashboard();
    void onSubmitResponse(const QJsonObject& data);
    void onBackToSurveyList();

private:
    enum PageIndex {
        SurveyListPage = 0,
        SurveyFormPage = 1,
        SettingsPage = 2
    };
    void setupUi();
    void setupConnections();
    void createMenus();
    void InitLog();
    void requestInitialPermissions();

    QStackedWidget *m_stackedWidget;
    DashboardWidget *m_dashboardWidget;
    SurveyFormWidget *m_surveyFormWidget = nullptr;
    NetworkManager *m_networkManager;
    LoginDialog *m_loginDialog;
    SurveyListWidget *m_surveyListWidget;
    
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_helpMenu;
    QAction *m_dashboardAction;
    QAction *m_logoutAction;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    
    QLabel *m_statusLabel;
    
    // Data
    QJsonObject m_currentUser;
    QString m_currentSurveyId;
    QString m_currentSurveyTitle;
};

#endif // MAINWINDOW_H
