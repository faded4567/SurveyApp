#ifndef SURVEYFORMWIDGET_H
#define SURVEYFORMWIDGET_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QButtonGroup>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QComboBox>
#include <QSlider>
#include <QTableWidget>
#include <QGestureEvent>
#include <QDateTime>
#include <QPushButton>
#include <QHeaderView>
#include <QFile>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMap>
#include <QStackedWidget>
#include <QList>
#include <QProgressBar>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QMediaFormat>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QFile>
#include "CustomUI.h"

class SurveyFormWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SurveyFormWidget(QWidget *parent = nullptr);

    void setSurveySchema(const QJsonObject& schema);

    qint64 GetDiffTime(){return m_endTime - m_startTime;}

public slots:
    void handleUploadSuccsee(const QJsonObject& response);
    void handleUploadFailed(const QString& error);

signals:
    void submitSurvey(const QJsonObject& data);
    void startSurvey();
    void UploadFile(QString , QString, QString);
    void backToSurveyList(); // 添加返回问卷列表信号

protected:
    // 添加事件处理函数
    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool gestureEvent(QGestureEvent *event);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onSubmitClicked();
    void onNextClicked();
    void onPrevClicked();
    void onBackToSurveyListClicked();
    void evaluateJumpLogic(int currentQuestionIndex);
    bool evaluateFinishRule(int currentQuestionIndex); // 添加结束规则评估函数
    bool evaluateGlobalRules(); // 添加全局规则评估函数
    void updateRecordTime(qint64 duration);
    void handleRecorderError();

private:
    void renderSurvey(const QJsonObject& schema);
    void renderQuestionPage(int questionIndex);
    void handleUploadButton(QPushButton* uploadButton, const QString& field);
    QJsonObject collectAnswers();
    int getNextQuestionIndex(int currentQuestionIndex);
    bool isQuestionAnswered(int questionIndex);
    bool isQuestionRequired(int questionIndex);
    void updateProgress(int num); // 添加更新进度条的方法
    void updateScrollBarVisibility();
    void adjustScrollBarRange(int pageHeight);
    void ensureLayoutCalculated(); // 玣保布局计算完成

    // 录音相关
    void requestAudioPermission();
    void StartRecord();
    void StopRecord();
    
    // 添加处理逻辑规则的函数
    bool evaluateExpression(const QString& expression, const QJsonObject& answers);
    bool evaluateGlobalRuleConditions(const QJsonObject& rule, const QJsonObject& currentAnswers); // 添加全局规则条件评估函数
    int findQuestionIndexById(const QString& questionId);
    
    // 添加滚动和触摸相关变量
    QScrollArea *m_scrollArea;
    QWidget *m_scrollWidget;
    qint64 m_startTime;
    qint64 m_endTime;
    QJsonObject m_schema;
    
    // 文件上传相关
    QMap<QString, QString> m_selectedFiles; // field -> file path
    QMap<QString, QLabel*> m_fileLabels;    // field -> file list label
    QMap<QString, QJsonObject> m_uploadedFiles; // field -> uploaded file info
    
    // 触摸和滚动相关变量
    QPoint m_lastTouchPoint;
    bool m_isTouching = false;
    int m_lastScrollValue = 0;
    
    // 分页相关变量
    CustomStackedWidget *m_stackedWidget;
    QList<QWidget*> m_questionPages;
    QList<QJsonObject> m_questions;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QPushButton *m_submitButton;
    QPushButton *m_backToListButton;
    int m_currentQuestionIndex;
    QList<QString> m_globalRules;
    
    // 进度条相关变量
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    int m_showNum;

    // 录音 摄像相关
    QMediaCaptureSession *captureSession;
    QAudioInput *audioInput;
    QMediaRecorder *mediaRecorder;
    QString m_output;
    QJsonObject m_autoUpLoadObj;
};

#endif // SURVEYFORMWIDGET_H
