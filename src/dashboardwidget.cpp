#include "dashboardwidget.h"
#include "surveylistwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QFormLayout>
#include <QDateTime>
#include <QTimer>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QScroller>
#include <QScrollerProperties>
#include <QGesture>
#include <QPanGesture>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent)
{
    m_isResumingFromBackground = false;
    

    QGridLayout *mainlayout = new QGridLayout(this);

    // 创建工具栏
    // m_refreshButton = new QPushButton("刷新");
    // mainlayout->addWidget(m_refreshButton,0,0,1,1);

    // m_projectStatusLabel = new QLabel("项目加载中...");
    // mainlayout->addWidget(m_projectStatusLabel,0,0,1,1);

    // 创建标签页
    m_tabWidget = new QTabWidget;
    mainlayout->addWidget(m_tabWidget, 0, 0, 4, 4);

    m_projectListWidget = new RefreshableListWidget;
    // 连接刷新信号到槽函数
    connect(m_projectListWidget, &RefreshableListWidget::refreshRequested,
            this, &DashboardWidget::refreshRequested);

    m_myProfileWidget = new QWidget;
    // 创建设置页面
    m_settingsWidget = new SettingsWidget;

    // 连接设置页面的信号
    connect(m_settingsWidget, &SettingsWidget::backToMain, this, [this]() {
        m_tabWidget->setCurrentWidget(m_projectListWidget);
    });

    m_tabWidget->addTab(m_projectListWidget, "问卷列表");
    m_tabWidget->addTab(m_myProfileWidget, "个人设置");
    m_tabWidget->addTab(m_settingsWidget, "问卷设置");

    setupMyProfileTab();
    setupConnections(); // 确保连接信号槽
    
    // 连接应用程序状态变化信号
    connect(qApp, &QApplication::applicationStateChanged, 
            this, &DashboardWidget::handleApplicationStateChanged);
}

void DashboardWidget::setupMyProfileTab()
{
    // 创建表单布局
    QFormLayout *formLayout = new QFormLayout;

    // 创建输入控件
    m_usernameEdit = new QLineEdit;
    m_usernameEdit->setReadOnly(true);
    m_emailEdit = new QLineEdit;
    m_phoneEdit = new QLineEdit;
    m_profileEdit = new QTextEdit;
    m_profileEdit->setMaximumHeight(120);

    // 添加到表单布局
    formLayout->addRow("用户名:", m_usernameEdit);
    formLayout->addRow("邮箱:", m_emailEdit);
    formLayout->addRow("电话:", m_phoneEdit);
    formLayout->addRow("个人简介:", m_profileEdit);

    // 创建保存按钮和状态标签
    m_saveProfileButton = new QPushButton("保存");
    m_saveStatusLabel = new QLabel;
    m_logoutButton = new QPushButton("退出登录");

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_saveProfileButton);
    buttonLayout->addWidget(m_saveStatusLabel);
    buttonLayout->addWidget(m_logoutButton);
    buttonLayout->addStretch();

    // 创建主布局并设置给 m_myProfileWidget
    QVBoxLayout *mainLayout = new QVBoxLayout(m_myProfileWidget);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch(); // 使内容靠上

    // 连接信号槽
    connect(m_saveProfileButton, &QPushButton::clicked, this, &DashboardWidget::onSaveProfileClicked);
}

void DashboardWidget::setupConnections()
{
    // connect(m_refreshButton, &QPushButton::clicked, this, &DashboardWidget::onRefreshClicked);
    connect(m_projectListWidget, &QListWidget::itemClicked, this, &DashboardWidget::onProjectItemClicked);
    connect(m_saveProfileButton, &QPushButton::clicked, this, &DashboardWidget::onSaveProfileClicked);
    connect(m_logoutButton, &QPushButton::clicked, this, &DashboardWidget::onLogoutClicked);
}


void DashboardWidget::setCurrentUser(const QJsonObject& userInfo)
{
    m_currentUser = userInfo;

    QJsonObject obj = m_currentUser["data"].toObject();

    // 填充用户信息到表单
    m_usernameEdit->setText(obj["name"].toString());
    m_emailEdit->setText(obj["email"].toString());
    m_phoneEdit->setText(obj["phone"].toString());
    m_profileEdit->setPlainText(obj["profile"].toString());
}

void DashboardWidget::setProjects(const QJsonArray& projects)
{
    int totalCount = m_projectListWidget->count(); // 获取总项数
    // 从最后一项开始，倒序删除直到索引1（第二项）
    for (int i = totalCount - 1; i > 0; --i) {
        QListWidgetItem *item = m_projectListWidget->takeItem(i); // 从列表中移除项并获取指针
        delete item; // 删除该项对象，释放内存[1,3](@ref)
    }

    // if (projects.isEmpty()) {
    //     m_projectStatusLabel->setText("暂无问卷");
    //     return;
    // }

    // m_projectStatusLabel->setText(QString("共找到 %1 个问卷").arg(projects.size()));

    for (const auto& projectValue : projects) {
        QJsonObject project = projectValue.toObject();
        QString title = project["name"].toString();
        int status = project["status"].toInt();
        QString lastUpdateStr = "未知";

        if (project.contains("updateAt") && !project["updateAt"].isNull()) {
            lastUpdateStr = project["updateAt"].toString();
        }

        // 状态翻译
        QString statusText;
        if (status == 1) statusText = "已发布";
        else if (status == 0) statusText = "未发布";
        else statusText = QString::number(status);

        // 创建列表项
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(QString("%1\n状态: %2  更新时间: %3")
                      .arg(title)
                      .arg(statusText)
                      .arg(lastUpdateStr));
        item->setData(Qt::UserRole, project["id"].toString());
        item->setData(Qt::UserRole + 1, title);

        m_projectListWidget->addItem(item);
    }
    m_projectListWidget->endRefresh();

}

void DashboardWidget::onRefreshClicked()
{
    emit refreshRequested();
}

void DashboardWidget::onProjectItemClicked(QListWidgetItem *item)
{
    QString projectId = item->data(Qt::UserRole).toString();
    QString title = item->data(Qt::UserRole + 1).toString();
    emit projectSelected(projectId, title);
}

void DashboardWidget::onSaveProfileClicked()
{
    // 这里应该调用API保存用户信息
    // 为简化起见，我们只显示一个提示信息
    m_saveStatusLabel->setText("保存成功");
    QTimer::singleShot(3000, [this]() {
        m_saveStatusLabel->setText("");
    });
}

void DashboardWidget::onLogoutClicked()
{
    emit logoutRequested();
}

void DashboardWidget::handleRefreshRequest()
{
    emit refreshRequested();
}

bool DashboardWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QWidget::event(event);
}

bool DashboardWidget::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pan = event->gesture(Qt::PanGesture)) {
        QPanGesture *panGesture = static_cast<QPanGesture *>(pan);
        Q_UNUSED(panGesture);
        // 处理平移手势
        return true;
    }
    return false;
}

void DashboardWidget::handleApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive) {
        // 当应用变为活跃状态时，标记为从后台恢复
        m_isResumingFromBackground = true;
        // 使用单次定时器来重置布局，确保在事件循环之后执行
        QTimer::singleShot(100, this, &DashboardWidget::resetLayoutOnAppResume);
    }
}

void DashboardWidget::resetLayoutOnAppResume()
{
    if (!m_isResumingFromBackground) {
        return;
    }
    
    // 强制更新布局
    m_tabWidget->updateGeometry();
    m_tabWidget->update();
    
    if (m_projectListWidget) {
        m_projectListWidget->updateGeometry();
        m_projectListWidget->update();
    }
    
    if (m_settingsWidget) {
        m_settingsWidget->updateGeometry();
        m_settingsWidget->update();
    }
    
    if (m_myProfileWidget) {
        m_myProfileWidget->updateGeometry();
        m_myProfileWidget->update();
    }
    
    update();
    
    // 重置标志
    m_isResumingFromBackground = false;
}

void DashboardWidget::showEvent(QShowEvent *event)
{
    // 当widget显示时，如果是从后台恢复，则重置布局
    if (m_isResumingFromBackground) {
        QTimer::singleShot(50, this, &DashboardWidget::resetLayoutOnAppResume);
    }
    
    QWidget::showEvent(event);
}
