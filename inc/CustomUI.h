#ifndef CUSTOMUI_H
#define CUSTOMUI_H

#include <QtWidgets>
#include <QScroller>
#include <QStackedWidget>

// 例如，如果你的 QScrollArea 直接 setWidget(stackedWidget);
class CustomStackedWidget : public QStackedWidget {
    Q_OBJECT

public:
    explicit CustomStackedWidget(QWidget *parent = nullptr){}
protected:
    QSize sizeHint() const override {
        if (currentWidget()) {
            return currentWidget()->sizeHint(); // 返回当前页面的推荐大小
            // 或者根据实际情况返回 currentWidget()->size() 或 minimumSizeHint()
        }
        return QStackedWidget::sizeHint();
    }
};

class RefreshableListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit RefreshableListWidget(QWidget *parent = nullptr)
        : QListWidget(parent),
        m_refreshThreshold(60),   // 触发刷新的下拉阈值（像素）
        m_refreshEnabled(false),
        m_refreshing(false),
        m_lastDragY(0),
        m_pullDistance(0),
        m_refreshIndicatorItem(nullptr)
    {
        // 1. 设置滚动条策略：始终显示垂直滚动条
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // [1](@ref)

        // 2. 创建刷新指示项（初始隐藏）
        // 初始化刷新指示项
        m_refreshIndicatorItem = new QListWidgetItem("下拉刷新");
        m_refreshIndicatorItem->setTextAlignment(Qt::AlignCenter);
        m_refreshIndicatorItem->setBackground(QBrush(Qt::lightGray));
        insertItem(0, m_refreshIndicatorItem);
        m_refreshIndicatorItem->setHidden(true); // 初始隐藏
        m_refreshIndicatorItem->setSizeHint(QSize(0, 0)); // 初始高度为0

        // 3. 启用触摸事件和手势识别（对移动端至关重要）
        setAttribute(Qt::WA_AcceptTouchEvents);
        QScroller::grabGesture(this, QScroller::TouchGesture); // [7](@ref)

        // 4. （可选）针对移动端调整滚动条样式，使其更细更美观
        verticalScrollBar()->setStyleSheet(
            "QScrollBar:vertical { width: 10px; background: transparent; margin: 0px; }"
            "QScrollBar::handle:vertical { background: #c0c0c0; border-radius: 5px; min-height: 20px; }"
            "QScrollBar::handle:vertical:hover { background: #a0a0a0; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; }"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
            ); // [3](@ref)
    }

signals:
    void refreshRequested(); // 刷新请求信号

protected:
    // 处理鼠标按下事件（主要用于桌面端）
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            m_lastDragY = event->pos().y();
            m_pullDistance = 0; // 重置下拉距离
            m_dragStartPos = event->pos(); // 记录拖动起始点
        }
        QListWidget::mousePressEvent(event);
    }

    // 处理鼠标移动事件（主要用于桌面端）
    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_refreshing) {
            QListWidget::mouseMoveEvent(event);
            return;
        }

        // 检查是否在列表顶部且向下拖动
        if (verticalScrollBar()->value() == 0 && event->buttons() & Qt::LeftButton) {
            int deltaY = event->pos().y() - m_lastDragY;

            // 添加一个小的起始阈值，避免轻微滑动误触发
            QPoint delta = event->pos() - m_dragStartPos;
            if (delta.manhattanLength() < QApplication::startDragDistance()) {
                QListWidget::mouseMoveEvent(event);
                return;
            }

            if (deltaY > 0) { // 向下拖动
                m_pullDistance = deltaY;

                // 显示刷新提示项（如果隐藏）
                if (m_refreshIndicatorItem->isHidden()) {
                    m_refreshIndicatorItem->setHidden(false);
                }

                // 动态设置指示项的高度，产生“下拉”视觉效果
                int indicatorHeight = qMin(m_pullDistance / 2, 40); // 限制最大高度
                m_refreshIndicatorItem->setSizeHint(QSize(0, indicatorHeight));

                // 根据下拉距离更新提示文本
                if (m_pullDistance >= m_refreshThreshold) {
                    m_refreshIndicatorItem->setText("松开刷新");
                    m_refreshEnabled = true;
                } else {
                    m_refreshIndicatorItem->setText("下拉刷新");
                    m_refreshEnabled = false;
                }
            }
        }
        QListWidget::mouseMoveEvent(event);
    }

    // 处理鼠标释放事件（主要用于桌面端）
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (m_refreshEnabled && !m_refreshing) {
                startRefresh();
            } else {
                // 未达到刷新条件，添加回弹动画
                QPropertyAnimation *animation = new QPropertyAnimation(this, "pullDistance");
                animation->setDuration(300); // 动画持续时间300ms
                animation->setStartValue(m_pullDistance);
                animation->setEndValue(0);
                animation->setEasingCurve(QEasingCurve::OutQuad); // 使用缓动曲线使动画更自然
                animation->start(QAbstractAnimation::DeleteWhenStopped);

                // 动画结束后隐藏指示项
                connect(animation, &QPropertyAnimation::finished, this, [this]() {
                    m_refreshIndicatorItem->setHidden(true);
                    m_refreshIndicatorItem->setSizeHint(QSize(0, 0)); // 重置高度
                });
            }
            m_refreshEnabled = false;
            m_pullDistance = 0;
        }
        QListWidget::mouseReleaseEvent(event);
    }

    // 处理触摸事件（用于移动端，增强兼容性）
    bool event(QEvent *event) override
    {
        return QListWidget::event(event);
        // switch (event->type()) {
        // case QEvent::TouchBegin: {
        //     QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        //     if (!touchEvent->touchPoints().isEmpty()) {
        //         m_lastDragY = touchEvent->touchPoints().first().pos().y();
        //         m_pullDistance = 0;
        //         m_isTouching = true;
        //     }
        //     break;
        // }
        // case QEvent::TouchUpdate: {
        //     if (m_refreshing || !m_isTouching) break;
        //     QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        //     if (touchEvent->touchPoints().isEmpty()) break;

        //     QTouchEvent::TouchPoint touchPoint = touchEvent->touchPoints().first();
        //     int currentY = touchPoint.pos().y();
        //     int deltaY = currentY - m_lastDragY;

        //     // 检查是否在列表顶部且向下拖动
        //     if (verticalScrollBar()->value() == 0 && deltaY > 0) {
        //         m_pullDistance = deltaY;

        //         if (m_refreshIndicatorItem->isHidden()) {
        //             m_refreshIndicatorItem->setHidden(false);
        //         }

        //         if (m_pullDistance >= m_refreshThreshold) {
        //             m_refreshIndicatorItem->setText("松开刷新");
        //             m_refreshEnabled = true;
        //         } else {
        //             m_refreshIndicatorItem->setText("下拉刷新");
        //             m_refreshEnabled = false;
        //         }
        //     }
        //     break;
        // }
        // case QEvent::TouchEnd:
        // case QEvent::TouchCancel: {
        //     m_isTouching = false;
        //     handleRelease();
        //     break;
        // }
        // default:
        //     break;
        // }
        // return QListWidget::event(event);
    }

    // 自定义属性，用于动画
    Q_PROPERTY(int pullDistance READ pullDistance WRITE setPullDistance)
    int pullDistance() const { return m_pullDistance; }
    void setPullDistance(int distance) {
        m_pullDistance = distance;
        if (!m_refreshIndicatorItem->isHidden()) {
            int indicatorHeight = qMin(m_pullDistance / 2, 40);
            m_refreshIndicatorItem->setSizeHint(QSize(0, indicatorHeight));
            if (m_pullDistance >= m_refreshThreshold) {
                m_refreshIndicatorItem->setText("松开刷新");
            } else {
                m_refreshIndicatorItem->setText("下拉刷新");
            }
        }
    }

private:
    // 处理释放事件的通用逻辑
    void handleRelease() {
        if (m_refreshEnabled && !m_refreshing) {
            startRefresh();
        } else {
            // 未达到刷新条件或正在刷新，隐藏提示
            m_refreshIndicatorItem->setHidden(true);
        }
        m_refreshEnabled = false;
        m_pullDistance = 0;
    }

public slots:
    // 开始刷新
    void startRefresh()
    {
        m_refreshing = true;
        m_refreshIndicatorItem->setText("正在刷新...");
        // 固定指示项的高度，避免在刷新时抖动
        m_refreshIndicatorItem->setSizeHint(QSize(0, 40));
        emit refreshRequested();
    }

    // 结束刷新
    void endRefresh()
    {
        m_refreshing = false;
        // 使用动画隐藏指示项
        QPropertyAnimation *animation = new QPropertyAnimation(this, "pullDistance");
        animation->setDuration(300);
        animation->setStartValue(40);
        animation->setEndValue(0);
        animation->setEasingCurve(QEasingCurve::OutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);

        connect(animation, &QPropertyAnimation::finished, this, [this]() {
            m_refreshIndicatorItem->setHidden(true);
            m_refreshIndicatorItem->setSizeHint(QSize(0, 0));
        });
    }

private:
    int m_refreshThreshold;  // 触发刷新的下拉阈值
    bool m_refreshEnabled;   // 是否已达到刷新阈值
    bool m_refreshing;       // 是否正在刷新
    int m_lastDragY;         // 记录上一次鼠标/触摸的Y位置
    int m_pullDistance;      // 当前下拉距离
    bool m_isTouching = false; // 是否正在触摸
    QPoint m_dragStartPos; // 记录拖动起始位置
    QListWidgetItem *m_refreshIndicatorItem; // 刷新指示项
};

#endif // CUSTOMUI_H
