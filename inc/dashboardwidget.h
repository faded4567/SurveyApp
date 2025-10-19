#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QJsonObject>
#include <QEvent>
#include <QGestureEvent>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QJsonArray>
#include <QGridLayout>
#include "CustomUI.h"
#include "settingswidget.h"

class SurveyListWidget;

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    void setProjects(const QJsonArray& projects);
    void setCurrentUser(const QJsonObject& user);

signals:
    void surveySelected(const QString& surveyId);
    void projectSelected(const QString& projectId, const QString& title);
    void logoutRequested();
    void refreshRequested();

protected:
    bool event(QEvent *event) override;
    bool gestureEvent(QGestureEvent *event);

private slots:
    void onProjectItemClicked(QListWidgetItem *item);
    void onSaveProfileClicked();
    void onRefreshClicked();
    void onLogoutClicked();
    void handleRefreshRequest();

private:
    void setupMyProfileTab();
    void setupConnections();
    void loadSurveyList();
    void loadProjectList();

    QTabWidget *m_tabWidget;
    RefreshableListWidget *m_projectListWidget;
    SettingsWidget *m_settingsWidget;
    QWidget *m_myProfileWidget;
    
    // Toolbar widgets
    QPushButton *m_refreshButton;
    QLabel *m_projectStatusLabel;
    
    // Profile tab widgets
    QLineEdit *m_usernameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_phoneEdit;
    QTextEdit *m_profileEdit;
    QPushButton *m_saveProfileButton;
    QLabel *m_saveStatusLabel;
    QPushButton *m_logoutButton;
    
    // Data
    QJsonArray m_projects;
    QJsonObject m_currentUser;



};

#endif // DASHBOARDWIDGET_H
