#ifndef SURVEYLISTWIDGET_H
#define SURVEYLISTWIDGET_H

#include <QWidget>
#include <QJsonArray>
#include <QGestureEvent>


class QListWidget;
class QListWidgetItem;
class QVBoxLayout;
class QLabel;
class QPushButton;

class SurveyListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SurveyListWidget(QWidget *parent = nullptr);
    void setSurveys(const QJsonArray& surveys);
    void clearSurveys();

signals:
    void surveySelected(QString surveyId, const QString& title);
    void refreshRequested();

protected:
    bool event(QEvent *event) override;
    bool gestureEvent(QGestureEvent *event);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onRefreshClicked();

private:
    void setupUi();
    void setupConnections();

    QVBoxLayout *m_mainLayout;
    QLabel *m_titleLabel;
    QListWidget *m_listWidget;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;
};

#endif // SURVEYLISTWIDGET_H
