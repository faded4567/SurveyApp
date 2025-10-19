#include "surveylistwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QIcon>
#include <QScroller>
#include <QScrollerProperties>
#include <QSpacerItem>

SurveyListWidget::SurveyListWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
    setupConnections();
}

void SurveyListWidget::setupUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // 标题
    m_titleLabel = new QLabel("SurveyKing 问卷列表");
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);

    // 刷新按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_refreshButton = new QPushButton("刷新");
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png")); // 如果有刷新图标
    m_refreshButton->setStyleSheet("padding: 5px 10px;");
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    m_mainLayout->addLayout(buttonLayout);

    // 创建包含下拉刷新功能的滚动区域
    m_scrollArea = new PullToRefreshScrollArea(this);
    m_scrollArea->setStyleSheet(
        "PullToRefreshScrollArea { border: none; }"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #f0f0f0;"
        "    width: 10px;"
        "    margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    border-radius: 5px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}");
    
    // 问卷列表
    m_listWidget = new QListWidget;
    m_listWidget->setStyleSheet(
        "QListWidget {"
        "   border: 1px solid #ddd;"
        "   border-radius: 4px;"
        "   background-color: white;"
        "}"
        "QListWidget::item {"
        "   border-bottom: 1px solid #eee;"
        "   padding: 15px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #e3f2fd;"
        "   color: #1976d2;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #f5f5f5;"
        "}"
        );
    
    // 设置列表始终可滚动，即使内容不足
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_listWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    
    // 启用触摸手势支持
    m_listWidget->grabGesture(Qt::PanGesture);
    m_listWidget->setAttribute(Qt::WA_AcceptTouchEvents);
    
    // 启用 Qt Scroller 支持，改善移动端触摸体验
    QScroller *scroller = QScroller::scroller(m_listWidget->viewport());
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    sp.setScrollMetric(QScrollerProperties::HorizontalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    sp.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 0.05); // 更平滑的拖拽体验
    sp.setScrollMetric(QScrollerProperties::MinimumVelocity, 0.0);
    sp.setScrollMetric(QScrollerProperties::MaximumVelocity, 0.8); // 提高最大速度
    sp.setScrollMetric(QScrollerProperties::AcceleratingFlickMaximumTime, 0.5); // 更长的滑动时间
    sp.setScrollMetric(QScrollerProperties::AcceleratingFlickSpeedupFactor, 2.0); // 更快的加速
    sp.setScrollMetric(QScrollerProperties::SnapPositionRatio, 0.02); // 更精确的位置停靠
    sp.setScrollMetric(QScrollerProperties::MaximumClickThroughVelocity, 0);
    sp.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);
    sp.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.1); // 减少点击延迟
    sp.setScrollMetric(QScrollerProperties::ScrollMetricCount, 1.0); // 启用惯性滚动
    sp.setScrollMetric(QScrollerProperties::AxisLockThreshold, 0.5); // 轴锁定阈值
    scroller->setScrollerProperties(sp);
    
    // 将列表添加到滚动区域
    m_scrollArea->setWidget(m_listWidget);
    
    m_mainLayout->addWidget(m_scrollArea, 1);

    // 状态标签
    m_statusLabel = new QLabel("暂无问卷");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("color: #7f8c8d;");
    m_mainLayout->addWidget(m_statusLabel);
    
    // 连接刷新信号
    connect(m_scrollArea, &PullToRefreshScrollArea::refreshRequested, this, &SurveyListWidget::onRefreshClicked);
}

void SurveyListWidget::setupConnections()
{
    connect(m_listWidget, &QListWidget::itemClicked, this, &SurveyListWidget::onItemClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &SurveyListWidget::onRefreshClicked);
}

void SurveyListWidget::setSurveys(const QJsonArray& surveys)
{
    m_listWidget->clear();

    if (surveys.isEmpty()) {
        m_statusLabel->setText("暂无可用问卷");
        return;
    }

    m_statusLabel->setText(QString("共找到 %1 份问卷").arg(surveys.size()));

    for (const auto& surveyValue : surveys) {
        QJsonObject survey = surveyValue.toObject();
        QString title = survey["name"].toString(); // SurveyKing使用name字段而不是title
        QString status = survey["status"].toString(); // SurveyKing直接使用status字段
        qint64 createDate = survey["createAt"].toVariant().toLongLong(); // SurveyKing使用createAt字段

        // 格式化日期
        QDateTime dateTime;
        dateTime.setMSecsSinceEpoch(createDate);
        QString dateStr = dateTime.toString("yyyy-MM-dd hh:mm");

        // 状态翻译
        QString statusText;
        if (status == "1") statusText = "已发布";
        else if (status == "0") statusText = "未发布";
        else statusText = status;

        // 创建列表项
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(QString("%1\n状态: %2  创建时间: %3")
                      .arg(title)
                      .arg(statusText)
                      .arg(dateStr));
        item->setData(Qt::UserRole, survey["id"].toString()); // SurveyKing使用id字段
        item->setData(Qt::UserRole + 1, title);

        m_listWidget->addItem(item);
    }
    
    // 添加一个大的空白项以确保始终可以滚动
    QListWidgetItem *spacerItem = new QListWidgetItem();
    spacerItem->setFlags(Qt::NoItemFlags);  // 使其不可选择和不可点击
    spacerItem->setSizeHint(QSize(0, 1000)); // 设置一个足够大的高度
    m_listWidget->addItem(spacerItem);
}

void SurveyListWidget::clearSurveys()
{
    m_listWidget->clear();
    m_statusLabel->setText("暂无问卷");
}

void SurveyListWidget::onItemClicked(QListWidgetItem *item)
{
    // 忽略 spacer item
    if (item->flags() == Qt::NoItemFlags) {
        return;
    }
    
    QString surveyId = item->data(Qt::UserRole).toString();
    QString title = item->data(Qt::UserRole + 1).toString();
    emit surveySelected(surveyId, title);
}

void SurveyListWidget::onRefreshClicked()
{
    emit refreshRequested();
}

bool SurveyListWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QWidget::event(event);
}

bool SurveyListWidget::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pan = event->gesture(Qt::PanGesture)) {
        // 可在此处进一步处理平移手势逻辑（如自定义滚动行为）
        return true;
    }
    return false;
}
