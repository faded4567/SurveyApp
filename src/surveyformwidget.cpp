#include "surveyformwidget.h"
#include <QJsonDocument>
#include <QScrollBar>
#include <QScroller>
#include <QScrollerProperties>
#include <QDateTime>
#include <QGesture>
#include <QPanGesture>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QFileInfo>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QProgressBar>
#include <QSettings>
#include <QPermissions>
#include <QStandardPaths>
#include "settingsmanager.h"



SurveyFormWidget::SurveyFormWidget(QWidget *parent) : QWidget(parent)
{
    FUNCTION_LOG();
    m_stackedWidget = new CustomStackedWidget(this);
    m_currentQuestionIndex = 0;
    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 创建顶部标题区域
    m_scrollWidget = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(m_scrollWidget);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(0);
    
    // 创建导航按钮
    m_prevButton = new QPushButton("上一题");
    m_nextButton = new QPushButton("下一题");
    m_submitButton = new QPushButton("提交");
    m_backToListButton = new QPushButton("返回问卷列表");
    
    // 设置按钮样式
    QString buttonStyle = "QPushButton {"
                         "background-color: #4A90E2;"
                         "color: white;"
                         "border: none;"
                         "border-radius: 6px;"
                         "padding: 10px 15px;"
                         "font-size: 14px;"
                         "font-weight: bold;"
                         "margin: 5px;"
                         "}"
                         "QPushButton:hover {"
                         "background-color: #5fa0f0;"
                         "}"
                         "QPushButton:pressed {"
                         "background-color: #3a7bc8;"
                         "}";
                         
    m_prevButton->setStyleSheet(buttonStyle);
    m_nextButton->setStyleSheet(buttonStyle);
    m_submitButton->setStyleSheet(buttonStyle);
    m_backToListButton->setStyleSheet(buttonStyle);
    
    // 创建进度条
    QWidget *progressWidget = new QWidget;
    progressWidget->setStyleSheet("background-color: white; border-bottom: 1px solid #b0d4e3; padding: 10px;");
    QHBoxLayout *progressLayout = new QHBoxLayout(progressWidget);
    
    m_progressLabel = new QLabel("进度: 0%");
    m_progressLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; margin-right: 10px;");
    
    m_progressBar = new QProgressBar;
    m_progressBar->setStyleSheet("QProgressBar {"
                                "border: 1px solid #b0d4e3;"
                                "border-radius: 6px;"
                                "text-align: center;"
                                "background-color: #f0f8ff;"
                                "}"
                                "QProgressBar::chunk {"
                                "background-color: #4A90E2;"
                                "border-radius: 5px;"
                                "}");
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    
    progressLayout->addWidget(m_progressLabel);
    progressLayout->addWidget(m_progressBar);
    
    // 添加标题和进度条到主布局
    mainLayout->addWidget(m_scrollWidget, 0);
    mainLayout->addWidget(progressWidget, 0);

    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true); // 重要：允许内容自适应
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 按需显示垂直滚动条
    m_scrollArea->setWidget(m_stackedWidget);
    mainLayout->addWidget(m_scrollArea, 1);
    
    // 创建底部按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(15, 10, 15, 15);
    buttonLayout->addWidget(m_prevButton);
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addWidget(m_submitButton);
    buttonLayout->addWidget(m_backToListButton);
    
    QWidget *buttonWidget = new QWidget;
    buttonWidget->setLayout(buttonLayout);
    mainLayout->addWidget(buttonWidget, 0);
    
    // 连接按钮信号
    connect(m_submitButton, &QPushButton::clicked, this, &SurveyFormWidget::onSubmitClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &SurveyFormWidget::onNextClicked);
    connect(m_prevButton, &QPushButton::clicked, this, &SurveyFormWidget::onPrevClicked);
    connect(m_backToListButton, &QPushButton::clicked, this, &SurveyFormWidget::onBackToSurveyListClicked);
    
    // 设置初始按钮状态
    m_prevButton->setEnabled(false);
    m_nextButton->setVisible(false);
    m_submitButton->setVisible(false);


    // 初始化媒体捕获会话
    captureSession = new QMediaCaptureSession(this);

    // 初始化音频输入
    audioInput = new QAudioInput(this);
    captureSession->setAudioInput(audioInput);

    // 初始化媒体录制器
    mediaRecorder = new QMediaRecorder(this);
    captureSession->setRecorder(mediaRecorder);

    // 连接信号槽
    connect(mediaRecorder, &QMediaRecorder::durationChanged, this, &SurveyFormWidget::updateRecordTime);
    connect(mediaRecorder, &QMediaRecorder::errorChanged, this, &SurveyFormWidget::handleRecorderError);

    // 初始化拍照相关组件
    m_camera = new QCamera(this);
    m_captureTimer = new QTimer(this);
    m_autoCaptureEnabled = false;

    // 连接相机信号
    connect(m_camera, &QCamera::activeChanged, this, &SurveyFormWidget::onCameraActiveChanged);
    connect(m_captureTimer, &QTimer::timeout, this, &SurveyFormWidget::capturePhoto);
}

void SurveyFormWidget::handleUploadButton(QPushButton* uploadButton, const QString& field)
{
    FUNCTION_LOG();
    // 打开文件选择对话框，支持图片和视频
    QString filter = "Images (*.png *.jpg *.jpeg *.gif *.bmp);;Videos (*.mp4 *.mov *.wmv *.avi *.mkv);;All Files (*.*)";
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", filter);
    
    if (!filePath.isEmpty()) {
        // 保存选中的文件路径
        AddFile(field, filePath);
        
        // 更新文件列表标签
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.fileName();
        QString fileSize = QString::number(fileInfo.size() / 1024.0, 'f', 1) + " KB";
        
        if (m_fileLabels.contains(field)) {
            m_fileLabels[field]->setText(QString("已选择文件：%1 (%2)").arg(fileName, fileSize));
        }
        
        // 立即上传文件
        // NetworkManager::instance().uploadFile(m_schema["id"].toString(), field, filePath);
        emit UploadFile(m_schema["id"].toString(), field, filePath);
    }
}

void SurveyFormWidget::setSurveySchema(const QJsonObject& schema)
{
    FUNCTION_LOG();
    m_schema = schema;
    renderSurvey(schema);


}

void SurveyFormWidget::handleUploadSuccsee(const QJsonObject &response)
{
    FUNCTION_LOG();
    qDebug()<<"handleUploadSuccsee.....";
    QString field,subField;
    QString fileName = response["data"].toObject()["originalName"].toString();
    qDebug()<<fileName;
    for(int i=0;i<m_selectedFiles.size();i++)
    {
        QJsonObject obj = m_selectedFiles[i].toObject();
        QJsonObject::iterator it;
        for (it = obj.begin(); it != obj.end(); ++it) {
            QString key = it.key();
            QJsonValue value = it.value();
            if(value.toString().contains(fileName))
            {
                field = key;
                m_selectedFiles.removeAt(i);
                break;
            }
        }
    }
    for(int i=0;i<m_questions.size();i++)
    {
        if(m_questions[i]["id"].toString() == field)
        {
            QJsonArray array = m_questions[i]["children"].toArray();
            for(int j=0;j<array.size();j++)
            {
                if(array[j].toObject().contains("id"))
                {
                    subField = array[j].toObject()["id"].toString();
                }
            }
            break;
        }

    }
    // 从响应中提取文件信息并保存
    if (!field.isEmpty()) {
        QJsonObject obj = response["data"].toObject();
        obj["subField"] = subField;
        obj["field"] = field;
        m_uploadedFiles.append(obj);
        qDebug()<<"success:"<<m_uploadedFiles;
        // QMessageBox::information(this, "上传成功", "文件上传成功");
        // 减少待上传计数
        m_pendingUploads--;
        // 如果所有文件都已上传完成，则更新上传状态
        if (m_pendingUploads <= 0) {
            m_isUploading = false;
            m_pendingUploads = 0; // 确保不会出现负数

            // 如果之前有提交请求被阻止，则现在执行提交
            if (m_shouldSubmitAfterUpload) {
                m_shouldSubmitAfterUpload = false;
                emit submitSurvey(collectAnswers());
            }
        }
    }
}

void SurveyFormWidget::handleUploadFailed(const QString& error)
{
    FUNCTION_LOG();
    QMessageBox::warning(this, "上传失败", "文件上传失败: " + error);
    // 减少待上传计数
    m_pendingUploads--;
    // 如果所有文件都已上传完成，则更新上传状态
    if (m_pendingUploads <= 0) {
        m_isUploading = false;
        m_pendingUploads = 0; // 确保不会出现负数

        // 如果之前有提交请求被阻止，则现在执行提交
        if (m_shouldSubmitAfterUpload) {
            m_shouldSubmitAfterUpload = false;
            emit submitSurvey(collectAnswers());
        }
    }
}

void SurveyFormWidget::renderSurvey(const QJsonObject& schema)
{
    FUNCTION_LOG();
    m_startTime = QDateTime::currentMSecsSinceEpoch();

    LocationManager::instance().startContinuousLocationUpdates(30000);
    
    // 清除之前的内容
    while (m_stackedWidget->count() > 0) {
        QWidget *widget = m_stackedWidget->widget(0);
        m_stackedWidget->removeWidget(widget);
        delete widget;
    }
    
    m_questionPages.clear();
    m_questions.clear();
    m_showQuestions.clear();

    // 设置整体样式表
    setStyleSheet("QWidget { background-color: #f0f8ff; font-family: 'Segoe UI', Arial, sans-serif; }"
                  "QGroupBox { background-color: white; border: 1px solid #b0d4e3; border-radius: 10px; margin: 10px; padding: 15px; }"
                  "QLabel { color: #333333; }"
                  "QPushButton { background-color: #4A90E2; color: white; border: none; border-radius: 6px; padding: 8px 12px; font-size: 14px; font-weight: bold; }"
                  "QPushButton:hover { background-color: #5fa0f0; }"
                  "QPushButton:pressed { background-color: #3a7bc8; }"
                  "QLineEdit, QTextEdit, QComboBox { padding: 8px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                  "QLineEdit:focus, QTextEdit:focus, QComboBox:focus { border-color: #4A90E2; }"
                  "QRadioButton, QCheckBox { font-size: 14px; spacing: 10px; color: #333333; }"
                  "QRadioButton::indicator, QCheckBox::indicator { width: 18px; height: 18px; }"
                  "QRadioButton::indicator { border-radius: 9px; }"
                  "QRadioButton::indicator, QCheckBox::indicator { border: 2px solid #b0d4e3; }"
                  "QRadioButton::indicator:unchecked, QCheckBox::indicator:unchecked { background-color: white; }"
                  "QRadioButton::indicator:checked, QCheckBox::indicator:checked { background-color: #4A90E2; border: 2px solid #4A90E2; }"
                  "QRadioButton::indicator:hover, QCheckBox::indicator:hover { border-color: #4A90E2; }"
                  "QRadioButton::indicator:checked:hover, QCheckBox::indicator:checked:hover { background-color: #5fa0f0; }"
                  "QComboBox::drop-down { border: none; }"
                  "QComboBox QAbstractItemView { border: 1px solid #b0d4e3; selection-background-color: #4A90E2; }"
                  "QTableWidget { gridline-color: #b0d4e3; }"
                  "QHeaderView::section { background-color: #e1f0fa; padding: 6px; border: 1px solid #b0d4e3; }"
                  "QScrollBar:vertical { border: none; background: #e1f0fa; width: 12px; margin: 0px 0px 0px 0px; border-radius: 6px; }"
                  "QScrollBar::handle:vertical { background: #b0d4e3; border-radius: 6px; min-height: 20px; }"
                  "QScrollBar::handle:vertical:hover { background: #4A90E2; }");

    // 解析并显示问卷标题 (SurveyKing中问卷标题在survey.title字段)
    QJsonObject survey = schema["survey"].toObject();
    QString title = survey["title"].toString();

    // 清空标题区域内容
    QLayout *titleLayout = m_scrollWidget->layout();
    if (titleLayout) {
        QLayoutItem *item;
        while ((item = titleLayout->takeAt(0)) != 0) {
            delete item->widget();
            delete item;
        }
    } else {
        titleLayout = new QVBoxLayout(m_scrollWidget);
        titleLayout->setContentsMargins(0, 0, 0, 0);
        titleLayout->setSpacing(0);
    }

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #2c3e50; margin: 15px; padding: 10px; text-align: center;");
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLayout->addWidget(titleLabel);

    // 解析并显示问卷描述
    QString description = survey["description"].toString();
    if (!description.isEmpty()) {
        QLabel *descLabel = new QLabel(description);
        descLabel->setStyleSheet("font-size: 14px; color: #555555; margin: 10px 15px 15px 15px; padding: 10px; background-color: #e1f0fa; border-radius: 8px;");
        descLabel->setWordWrap(true);
        titleLayout->addWidget(descLabel);
    }
    
    // 发射开始答题信号
    emit startSurvey();

    m_showNum = 0;
    // 解析题目 (SurveyKing中问题结构在survey.children字段中)
    QJsonArray questions = survey["children"].toArray();
    for (int i = 0; i < questions.size(); ++i) {
        QJsonObject question = questions[i].toObject();
        m_questions.append(question);

        // 检查题目是否应该隐藏
        QJsonObject attribute = question["attribute"].toObject();
        bool isHidden = false;
        // 检查 display 属性
        if (attribute.contains("display") && attribute["display"].isString()) {
            QString displayValue = attribute["display"].toString();
            if (displayValue == "hidden") {
                isHidden = true;
            }
        }

        // 只有非隐藏题目才添加到问卷中
        if (!isHidden) {
            m_showQuestions.append(question);
            m_showNum++;
        }

        QString title = question["title"].toString();
        // 保存自动上传相关的隐藏题目
        if(title == "录音和拍摄文件" && isHidden)
            m_autoUpLoadObj = question;
        else if(title == "位置信息" && isHidden)
            m_locationObj = question;
    }

    // 处理全局规则
    QJsonObject surveyAttribute = survey["attribute"].toObject();
    if (surveyAttribute.contains("globalRule") && surveyAttribute["globalRule"].isArray()) {
        QJsonArray globalRules = surveyAttribute["globalRule"].toArray();
        for (int i = 0; i < globalRules.size(); ++i) {
            QString rule = globalRules[i].toString();
            m_globalRules.append(rule);
        }
    }

    // 为每道题创建页面
    for (int i = 0; i < m_showQuestions.size(); ++i) {

        QWidget *page = new QWidget;
        QVBoxLayout *pageLayout = new QVBoxLayout(page);
        pageLayout->setContentsMargins(0, 0, 0, 0);
        m_questionPages.append(page);
        m_stackedWidget->addWidget(page);
    }
    
    // 如果有题目，则显示第一题
    if (!m_showQuestions.isEmpty()) {
        m_currentQuestionIndex = 0;
        renderQuestionPage(0);
        m_stackedWidget->setCurrentIndex(0);
        updateProgress(m_showNum); // 初始化进度条
        m_scrollArea->verticalScrollBar()->setValue(0);

        // 更新按钮状态
        m_prevButton->setEnabled(false);
        m_nextButton->setVisible(true);
        m_submitButton->setVisible(m_showQuestions.size() <= 1);
        
        if (m_showQuestions.size() <= 1) {
            m_nextButton->setVisible(false);
            m_submitButton->setVisible(true);
        } else {
            m_nextButton->setVisible(true);
            m_submitButton->setVisible(false);
        }

        // 是否有录音
        if(SettingsManager::getInstance().getValue("survey/autoRecord").toBool())
        {
            requestAudioPermission();
        }

        // 检查是否启用自动拍照
        m_autoCaptureEnabled = SettingsManager::getInstance().getValue("survey/autoCapture", false).toBool();
        if (m_autoCaptureEnabled) {
            initCamera();
        }
    }
    
    // 设置StackedWidget的尺寸策略，确保它能正确填充可用空间
    m_stackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SurveyFormWidget::renderQuestionPage(int questionIndex)
{
    FUNCTION_LOG();
    if (questionIndex < 0 || questionIndex >= m_showQuestions.size()) {
        return;
    }
    
    // 获取当前页面
    QWidget *page = m_questionPages[questionIndex];
    QVBoxLayout *pageLayout = qobject_cast<QVBoxLayout*>(page->layout());
    
    // 清除页面内容（保留布局）
    QLayoutItem *item;
    while ((item = pageLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    
    // 获取题目信息
    QJsonObject question = m_showQuestions[questionIndex];
    QString type = question["type"].toString();
    QString title = question["title"].toString();
    QString field = question["id"].toString();
    QJsonObject attribute = question["attribute"].toObject();
    bool isRequired = attribute["required"].toBool();
    QString description = question["description"].toString(); // 获取题目描述

    QGroupBox *questionGroup = new QGroupBox;
    questionGroup->setStyleSheet("QGroupBox { background-color: white; border: 1px solid #b0d4e3; border-radius: 10px; margin: 10px; padding: 15px; }");
    questionGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *questionLayout = new QVBoxLayout(questionGroup);
    questionLayout->setSpacing(12);

    QString questionText = QString("<html><body><p style='font-size:16px; font-weight:bold; margin:0; color:#2c3e50;'>%1%2</p></body></html>")
                          .arg(title)
                          .arg(isRequired ? "<span style='color:#e74c3c;'> *</span>" : "");

    QLabel *questionLabel = new QLabel(questionText);
    questionLabel->setWordWrap(true);
    questionLabel->setTextFormat(Qt::RichText);
    questionLayout->addWidget(questionLabel);

    // 如果有题目描述，则显示题目描述
    if (!description.isEmpty()) {
        QLabel *descriptionLabel = new QLabel(description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet("font-size: 14px; color: #666666; margin-top: 5px; margin-bottom: 10px;");
        questionLayout->addWidget(descriptionLabel);
    }

    // 根据题型创建不同的输入控件
    if (type == "FillBlank") {
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setProperty("field", field);
        QJsonArray options = question["children"].toArray();
        for (int j = 0; j < options.size(); ++j) {
            QJsonObject option = options[j].toObject();
            QString optionId = option["id"].toString();
            lineEdit->setProperty("id", optionId);
        }

        lineEdit->setStyleSheet("QLineEdit { padding: 10px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                               "QLineEdit:focus { border-color: #4A90E2; box-shadow: 0 0 5px rgba(74, 144, 226, 0.5); }");
        questionLayout->addWidget(lineEdit);
    }
    else if (type == "Textarea") {
        QTextEdit *textEdit = new QTextEdit;
        textEdit->setProperty("field", field);
        QJsonArray options = question["children"].toArray();
        for (int j = 0; j < options.size(); ++j) {
            QJsonObject option = options[j].toObject();
            QString optionId = option["id"].toString();
            textEdit->setProperty("id", optionId);
        }
        textEdit->setStyleSheet("QTextEdit { padding: 10px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                               "QTextEdit:focus { border-color: #4A90E2; box-shadow: 0 0 5px rgba(74, 144, 226, 0.5); }");
        textEdit->setMaximumHeight(150);
        questionLayout->addWidget(textEdit);
    }
    else if (type == "Radio") {
        QButtonGroup *buttonGroup = new QButtonGroup(questionGroup);
        buttonGroup->setExclusive(true);
        buttonGroup->setProperty("field", field);

        QJsonArray options = question["children"].toArray();
        QVBoxLayout *optionsLayout = new QVBoxLayout;
        optionsLayout->setSpacing(10);

        for (int j = 0; j < options.size(); ++j) {
            QJsonObject option = options[j].toObject();
            QString optionText = option["title"].toString();
            QString optionId = option["id"].toString();
            
            // 检查选项是否包含填空
            QJsonObject optionAttribute = option["attribute"].toObject();
            bool hasBlank = optionAttribute["dataType"].toString() == "horzBlank";

            if (hasBlank) {
                // 创建包含输入框的选项布局
                QHBoxLayout *optionLayout = new QHBoxLayout;
                
                QRadioButton *radioButton = new QRadioButton;
                radioButton->setProperty("field", field);
                radioButton->setProperty("value", optionId);
                radioButton->setStyleSheet("QRadioButton { font-size: 14px; spacing: 12px; color: #333333; }"
                                          "QRadioButton::indicator { width: 20px; height: 20px; border-radius: 10px; border: 2px solid #b0d4e3; }"
                                          "QRadioButton::indicator:unchecked { background-color: white; }"
                                          "QRadioButton::indicator:checked { background-color: #4A90E2; border: 2px solid #4A90E2; }"
                                          "QRadioButton::indicator:hover { border-color: #4A90E2; }"
                                          "QRadioButton::indicator:checked:hover { background-color: #5fa0f0; }");
                // 提取下划线前的文本作为标签
                QString label = optionText;
                label.replace("____________", "");
                QLabel *optionLabel = new QLabel(label);
                optionLabel->setStyleSheet("font-size: 14px; color: #333333;");
                
                QLineEdit *lineEdit = new QLineEdit;
                // 示例：设置下划线样式
                lineEdit->setStyleSheet("QLineEdit { border: none; border-bottom: 1px solid black; background-color: transparent; }");
                lineEdit->setProperty("field", field);
                QJsonArray chi = option["children"].toArray();
                QString sub = chi.at(0)["id"].toString();
                lineEdit->setProperty("subField", optionId);
                lineEdit->setProperty("id", sub);
                lineEdit->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #b0d4e3; border-radius: 4px; font-size: 14px; background-color: white; margin-left: 10px; }"
                                       "QLineEdit:focus { border-color: #4A90E2; box-shadow: 0 0 3px rgba(74, 144, 226, 0.5); }");
                lineEdit->setPlaceholderText("请输入");
                lineEdit->setEnabled(false); // 默认禁用，选中单选按钮后启用
                
                // 连接单选按钮和输入框的状态
                connect(radioButton, &QRadioButton::toggled, [lineEdit](bool checked) {
                    lineEdit->setEnabled(checked);
                    if (!checked) {
                        lineEdit->clear();
                    }
                });
                
                optionLayout->addWidget(radioButton);
                optionLayout->addWidget(optionLabel);
                optionLayout->addWidget(lineEdit);
                optionLayout->addStretch();
                
                optionsLayout->addLayout(optionLayout);
                buttonGroup->addButton(radioButton, j);
            } else {
                // 普通选项
                QRadioButton *radioButton = new QRadioButton(optionText);
                radioButton->setProperty("field", field);
                radioButton->setProperty("value", optionId);
                radioButton->setStyleSheet("QRadioButton { font-size: 14px; spacing: 12px; color: #333333; }"
                                          "QRadioButton::indicator { width: 20px; height: 20px; border-radius: 10px; border: 2px solid #b0d4e3; }"
                                          "QRadioButton::indicator:unchecked { background-color: white; }"
                                          "QRadioButton::indicator:checked { background-color: #4A90E2; border: 2px solid #4A90E2; }"
                                          "QRadioButton::indicator:hover { border-color: #4A90E2; }"
                                          "QRadioButton::indicator:checked:hover { background-color: #5fa0f0; }");
                buttonGroup->addButton(radioButton, j);
                optionsLayout->addWidget(radioButton);
            }
        }

        questionLayout->addLayout(optionsLayout);
    }
    else if (type == "Checkbox") {
        QButtonGroup *buttonGroup = new QButtonGroup(questionGroup);
        buttonGroup->setExclusive(false);  // 设置为非独占，允许多选
        buttonGroup->setProperty("field", field);

        QJsonArray options = question["children"].toArray();
        QVBoxLayout *optionsLayout = new QVBoxLayout;
        optionsLayout->setSpacing(10);

        for (int j = 0; j < options.size(); ++j) {
            QJsonObject option = options[j].toObject();
            QString optionText = option["title"].toString();
            QString optionId = option["id"].toString();
            
            // 检查选项是否包含填空
            QJsonObject optionAttribute = option["attribute"].toObject();
            bool hasBlank = optionAttribute["dataType"].toString() == "horzBlank";

            if (hasBlank) {
                // 创建包含输入框的选项布局
                QHBoxLayout *optionLayout = new QHBoxLayout;
                
                QCheckBox *checkBox = new QCheckBox;
                checkBox->setProperty("field", field);
                checkBox->setProperty("value", optionId);
                checkBox->setStyleSheet("QCheckBox { font-size: 14px; spacing: 12px; color: #333333; }"
                                       "QCheckBox::indicator { width: 20px; height: 20px; border: 2px solid #b0d4e3; border-radius: 4px; }"
                                       "QCheckBox::indicator:unchecked { background-color: white; }"
                                       "QCheckBox::indicator:checked { background-color: #4A90E2; border: 2px solid #4A90E2; }"
                                       "QCheckBox::indicator:hover { border-color: #4A90E2; }"
                                       "QCheckBox::indicator:checked:hover { background-color: #5fa0f0; }");
                
                // 提取下划线前的文本作为标签
                QString label = optionText;
                label.replace("____________", "");
                QLabel *optionLabel = new QLabel(label);
                optionLabel->setStyleSheet("font-size: 14px; color: #333333;");
                
                QLineEdit *lineEdit = new QLineEdit;
                lineEdit->setProperty("field", field);
                lineEdit->setProperty("subField", optionId);
                lineEdit->setStyleSheet("QLineEdit { padding: 6px; border: 1px solid #b0d4e3; border-radius: 4px; font-size: 14px; background-color: white; margin-left: 10px; }"
                                       "QLineEdit:focus { border-color: #4A90E2; box-shadow: 0 0 3px rgba(74, 144, 226, 0.5); }");
                lineEdit->setPlaceholderText("请输入");
                lineEdit->setEnabled(false); // 默认禁用，选中复选框后启用
                
                // 连接复选框和输入框的状态
                connect(checkBox, &QCheckBox::toggled, [lineEdit](bool checked) {
                    lineEdit->setEnabled(checked);
                    if (!checked) {
                        lineEdit->clear();
                    }
                });
                
                optionLayout->addWidget(checkBox);
                optionLayout->addWidget(optionLabel);
                optionLayout->addWidget(lineEdit);
                optionLayout->addStretch();
                
                optionsLayout->addLayout(optionLayout);
                buttonGroup->addButton(checkBox, j);
            } else {
                // 普通选项
                QCheckBox *checkBox = new QCheckBox(optionText);
                checkBox->setProperty("field", field);
                checkBox->setProperty("value", optionId);
                checkBox->setStyleSheet("QCheckBox { font-size: 14px; spacing: 12px; color: #333333; }"
                                       "QCheckBox::indicator { width: 20px; height: 20px; border: 2px solid #b0d4e3; border-radius: 4px; }"
                                       "QCheckBox::indicator:unchecked { background-color: white; }"
                                       "QCheckBox::indicator:checked { background-color: #4A90E2; border: 2px solid #4A90E2; }"
                                       "QCheckBox::indicator:hover { border-color: #4A90E2; }"
                                       "QCheckBox::indicator:checked:hover { background-color: #5fa0f0; }");
                buttonGroup->addButton(checkBox, j);
                optionsLayout->addWidget(checkBox);
            }
        }

        questionLayout->addLayout(optionsLayout);
    }
    else if (type == "Select") {
        QComboBox *comboBox = new QComboBox;
        comboBox->setProperty("field", field);
        comboBox->setStyleSheet("QComboBox { padding: 10px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                               "QComboBox:focus { border-color: #4A90E2; box-shadow: 0 0 5px rgba(74, 144, 226, 0.5); }"
                               "QComboBox::drop-down { border: none; }"
                               "QComboBox::down-arrow { image: none; width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #4A90E2; margin-right: 10px; }"
                               "QComboBox QAbstractItemView { border: 1px solid #b0d4e3; selection-background-color: #4A90E2; border-radius: 0 0 6px 6px; }");
        
        // 添加空选项作为默认选项
        comboBox->addItem("请选择");
        
        QJsonArray options = question["children"].toArray();
        for (int j = 0; j < options.size(); ++j) {
            QJsonObject option = options[j].toObject();
            QString optionText = option["title"].toString();
            QString optionId = option["id"].toString();
            comboBox->addItem(optionText, optionId);
        }
        
        questionLayout->addWidget(comboBox);
    }
    else if (type == "MultipleBlank") {
        QJsonArray blanks = question["children"].toArray();
        for (int j = 0; j < blanks.size(); ++j) {
            QJsonObject blank = blanks[j].toObject();
            QString blankTitle = blank["title"].toString();
            QString blankId = blank["id"].toString();
            
            QHBoxLayout *blankLayout = new QHBoxLayout;
            QLabel *blankLabel = new QLabel(blankTitle + ":");
            blankLabel->setStyleSheet("font-size: 14px; color: #333333;");
            QLineEdit *lineEdit = new QLineEdit;
            lineEdit->setProperty("field", field);
            lineEdit->setProperty("subField", blankId);
            lineEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                                   "QLineEdit:focus { border-color: #4A90E2; box-shadow: 0 0 5px rgba(74, 144, 226, 0.5); }");
            
            blankLayout->addWidget(blankLabel);
            blankLayout->addWidget(lineEdit);
            questionLayout->addLayout(blankLayout);
        }
    }
    else if (type == "Score" || type == "Nps") {
        // 评分题和NPS题 - 使用滑块
        QVBoxLayout *scoreLayout = new QVBoxLayout;
        
        QSlider *slider = new QSlider(Qt::Horizontal);
        slider->setRange(0, 10);
        slider->setValue(0);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setTickInterval(1);
        slider->setProperty("field", field);
        slider->setStyleSheet("QSlider::groove:horizontal { height: 8px; background: #e1f0fa; border-radius: 4px; }"
                             "QSlider::handle:horizontal { background: #4A90E2; border: 2px solid #4A90E2; width: 20px; margin: -6px 0; border-radius: 10px; }"
                             "QSlider::handle:horizontal:hover { background: #5fa0f0; }"
                             "QSlider::sub-page:horizontal { background: #4A90E2; border-radius: 4px; }");
        
        QHBoxLayout *scoreLabelsLayout = new QHBoxLayout;
        for (int i = 0; i <= 10; ++i) {
            QLabel *label = new QLabel(QString::number(i));
            label->setStyleSheet("font-size: 12px; color: #555555;");
            label->setAlignment(Qt::AlignCenter);
            scoreLabelsLayout->addWidget(label);
        }
        
        scoreLayout->addWidget(slider);
        scoreLayout->addLayout(scoreLabelsLayout);
        questionLayout->addLayout(scoreLayout);
    }
    else if (type == "Cascader") {
        // 级联题 - 添加级联选择器
        QVBoxLayout *cascaderLayout = new QVBoxLayout;
        
        QComboBox *firstLevelCombo = new QComboBox;
        firstLevelCombo->setStyleSheet("QComboBox { padding: 8px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                                      "QComboBox:focus { border-color: #4A90E2; }"
                                      "QComboBox::drop-down { border: none; }");
        firstLevelCombo->addItem("请选择");
        firstLevelCombo->setProperty("field", field);
        
        QComboBox *secondLevelCombo = new QComboBox;
        secondLevelCombo->setStyleSheet("QComboBox { padding: 8px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                                       "QComboBox:focus { border-color: #4A90E2; }"
                                       "QComboBox::drop-down { border: none; }");
        secondLevelCombo->addItem("请选择");
        secondLevelCombo->setEnabled(false);
        
        cascaderLayout->addWidget(firstLevelCombo);
        cascaderLayout->addWidget(secondLevelCombo);
        
        // 添加示例选项
        firstLevelCombo->addItem("选项1", "opt1");
        firstLevelCombo->addItem("选项2", "opt2");
        
        // 连接级联逻辑
        connect(firstLevelCombo, &QComboBox::currentTextChanged, [secondLevelCombo](const QString &text) {
            secondLevelCombo->clear();
            secondLevelCombo->addItem("请选择");
            if (!text.isEmpty() && text != "请选择") {
                secondLevelCombo->setEnabled(true);
                secondLevelCombo->addItem("子选项1", "sub1");
                secondLevelCombo->addItem("子选项2", "sub2");
            } else {
                secondLevelCombo->setEnabled(false);
            }
        });
        
        questionLayout->addLayout(cascaderLayout);
    }
    else if (type == "Upload") {
        // 上传题 - 添加文件选择按钮
        QVBoxLayout *uploadLayout = new QVBoxLayout;
        
        QPushButton *uploadButton = new QPushButton("选择文件");
        uploadButton->setStyleSheet("QPushButton {"
                                   "background-color: #e1f0fa;"
                                   "color: #4A90E2;"
                                   "border: 1px solid #b0d4e3;"
                                   "border-radius: 6px;"
                                   "padding: 10px 15px;"
                                   "font-size: 14px;"
                                   "font-weight: normal;"
                                   "}"
                                   "QPushButton:hover {"
                                   "background-color: #d0e6f8;"
                                   "}");
        uploadButton->setProperty("field", field);
        
        QLabel *fileListLabel = new QLabel("已选择文件：无");
        fileListLabel->setStyleSheet("font-size: 14px; color: #555555; padding: 8px;");
        fileListLabel->setWordWrap(true);
        
        // 保存引用以便后续更新
        m_fileLabels[field] = fileListLabel;
        
        uploadLayout->addWidget(uploadButton);
        uploadLayout->addWidget(fileListLabel);
        
        questionLayout->addLayout(uploadLayout);
        
        // 连接上传按钮点击事件
        connect(uploadButton, &QPushButton::clicked, [=]() {
            handleUploadButton(uploadButton, field);
        });
    }
    else if (type == "Signature") {
        // 电子签名 - 添加签名区域
        QVBoxLayout *signatureLayout = new QVBoxLayout;
        
        QLabel *signatureLabel = new QLabel("请在下方区域签名（模拟实现）");
        signatureLabel->setStyleSheet("font-size: 14px; color: #555555; padding: 8px;");
        
        // 签名显示区域
        QLabel *signatureArea = new QLabel("签名区域");
        signatureArea->setStyleSheet("background-color: #f8fbfd; border: 1px dashed #b0d4e3; border-radius: 6px; padding: 20px; margin: 10px 0; color: #999999;");
        signatureArea->setAlignment(Qt::AlignCenter);
        signatureArea->setMinimumHeight(100);
        signatureArea->setProperty("field", field);
        
        QHBoxLayout *signatureButtonsLayout = new QHBoxLayout;
        QPushButton *signButton = new QPushButton("签名");
        signButton->setStyleSheet("QPushButton {"
                                 "background-color: #4A90E2;"
                                 "color: white;"
                                 "border: none;"
                                 "border-radius: 6px;"
                                 "padding: 8px 15px;"
                                 "font-size: 14px;"
                                 "}"
                                 "QPushButton:hover {"
                                 "background-color: #5fa0f0;"
                                 "}");
        
        QPushButton *clearButton = new QPushButton("清除");
        clearButton->setStyleSheet("QPushButton {"
                                  "background-color: #e74c3c;"
                                  "color: white;"
                                  "border: none;"
                                  "border-radius: 6px;"
                                  "padding: 8px 15px;"
                                  "margin-left: 10px;"
                                  "}"
                                  "QPushButton:hover {"
                                  "background-color: #f06c5c;"
                                  "}");
        
        signatureButtonsLayout->addWidget(signButton);
        signatureButtonsLayout->addWidget(clearButton);
        signatureButtonsLayout->addStretch();
        
        signatureLayout->addWidget(signatureLabel);
        signatureLayout->addWidget(signatureArea);
        signatureLayout->addLayout(signatureButtonsLayout);
        
        questionLayout->addLayout(signatureLayout);
    }
    else if (type == "Barcode") {
        // 扫码题 - 添加扫码按钮
        QVBoxLayout *barcodeLayout = new QVBoxLayout;
        
        QPushButton *scanButton = new QPushButton("扫描二维码");
        scanButton->setStyleSheet("QPushButton {"
                                 "background-color: #4A90E2;"
                                 "color: white;"
                                 "border: none;"
                                 "border-radius: 6px;"
                                 "padding: 10px 15px;"
                                 "font-size: 14px;"
                                 "font-weight: normal;"
                                 "}"
                                 "QPushButton:hover {"
                                 "background-color: #5fa0f0;"
                                 "}");
        barcodeLayout->addWidget(scanButton);
        
        QLabel *barcodeResult = new QLabel("扫描结果将显示在这里");
        barcodeResult->setStyleSheet("font-size: 14px; color: #555555; padding: 8px; background-color: #f8fbfd; border-radius: 6px; margin-top: 10px;");
        barcodeResult->setWordWrap(true);
        barcodeLayout->addWidget(barcodeResult);
        
        questionLayout->addLayout(barcodeLayout);
    }
    else {
        // 默认处理 - 简单文本输入
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setProperty("field", field);
        lineEdit->setStyleSheet("QLineEdit { padding: 10px; border: 1px solid #b0d4e3; border-radius: 6px; font-size: 14px; background-color: white; }"
                               "QLineEdit:focus { border-color: #4A90E2; box-shadow: 0 0 5px rgba(74, 144, 226, 0.5); }");
        questionLayout->addWidget(lineEdit);
    }
    
    pageLayout->addWidget(questionGroup);
    pageLayout->addStretch();

    // 在页面渲染完成后，强制计算尺寸
    page->setMinimumHeight(0); // 清除最小高度限制
    page->adjustSize(); // 调整尺寸以适应内容

    // 设置页面的尺寸策略
    page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

    // 强制布局更新
    if (pageLayout) {
        pageLayout->activate();
    }

    // 延迟更新滚动条，确保所有尺寸计算已完成
    QTimer::singleShot(5, this, [this, page]() {
        // 确保页面可见后再更新滚动条
        if (page->isVisible()) {
            updateScrollBarVisibility();
        }
    });
}

void SurveyFormWidget::onNextClicked()
{
    FUNCTION_LOG();
    // 保存当前题目的答案
    saveCurrentAnswer(m_currentQuestionIndex);

    // 检查当前题是否为必填题但未回答
    if (isQuestionRequired(m_currentQuestionIndex) && !isQuestionAnswered(m_currentQuestionIndex)) {
        QMessageBox::warning(this, "提示", "这是必填题，请完成作答后再继续");
        return;
    }

    // 检查全局规则
    if (!evaluateGlobalRules()) {
        // 如果全局规则检查失败，不继续执行
        return;
    }
    
    // 检查是否有结束规则
    if (evaluateFinishRule(m_currentQuestionIndex)) {
        // 如果满足结束规则，直接提交问卷
        onSubmitClicked();
        return;
    }
    
    int nextIndex = getNextQuestionIndex(m_currentQuestionIndex);
    if (nextIndex < m_showQuestions.size()) {
        renderQuestionPage(nextIndex);
        m_stackedWidget->setCurrentIndex(nextIndex);

        m_currentQuestionIndex = nextIndex;
        updateProgress(m_showNum); // 更新进度条
        m_scrollArea->verticalScrollBar()->setValue(0);
        
        // 更新按钮状态
        m_prevButton->setEnabled(true);
        if (nextIndex == m_showQuestions.size() - 1) {
            m_nextButton->setVisible(false);
            m_submitButton->setVisible(true);
        }
    }
}

void SurveyFormWidget::saveCurrentAnswer(int questionIndex)
{
    FUNCTION_LOG();
    if (questionIndex < 0 || questionIndex >= m_showQuestions.size()) {
        return;
    }

    QJsonObject currentAnswer = collectSingleQuestionAnswer(questionIndex);
    m_answerCache[questionIndex] = currentAnswer;
}

QString SurveyFormWidget::getAnswerValue(const QJsonObject& savedAnswer, const QString& field)
{
    FUNCTION_LOG();
    if (savedAnswer.contains(field)) {
        QJsonObject fieldObj = savedAnswer[field].toObject();
        if (!fieldObj.isEmpty()) {
            // 返回第一个值（适用于单选、填空等）
            return fieldObj.begin().value().toString();
        }
    }
    return QString();
}

QJsonObject SurveyFormWidget::collectSingleQuestionAnswer(int questionIndex)
{
    FUNCTION_LOG();
    QJsonObject answer;

    if (questionIndex < 0 || questionIndex >= m_showQuestions.size()) {
        return answer;
    }

    QWidget* page = m_questionPages[questionIndex];
    if (!page) return answer;

    QList<QWidget*> widgets = page->findChildren<QWidget*>();
    QJsonObject question = m_showQuestions[questionIndex];
    QString field = question["id"].toString();
    QString type = question["type"].toString();

    // 根据题型收集答案，确保格式与collectAnswers()一致
    if (type == "FillBlank") {
        for (QWidget* widget : widgets) {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                if (lineEdit->property("field").toString() == field) {
                    QString value = lineEdit->text();
                    if (!value.isEmpty()) {
                        // 格式: {"questionId": {"optionId": "value"}}
                        QString id = lineEdit->property("id").toString();
                        if (!id.isEmpty()) {
                            answer[field] = QJsonObject{{id, value}};
                        }
                    }
                    break; // 每个填空题只有一个输入框
                }
            }
        }
    }
    else if (type == "Textarea") {
        for (QWidget* widget : widgets) {
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                if (textEdit->property("field").toString() == field) {
                    QString value = textEdit->toPlainText();
                    if (!value.isEmpty()) {
                        // 格式: {"questionId": {"optionId": "value"}}
                        QString id = textEdit->property("id").toString();
                        if (!id.isEmpty()) {
                            answer[field] = QJsonObject{{id, value}};
                        }
                    }
                    break;
                }
            }
        }
    }
    else if (type == "Radio") {
        QJsonObject valueObj;
        for (QWidget* widget : widgets) {
            if (QRadioButton* radioButton = qobject_cast<QRadioButton*>(widget)) {
                if (radioButton->property("field").toString() == field && radioButton->isChecked()) {
                    QString value = radioButton->property("value").toString();
                    // 格式: {"questionId": {"optionId": "optionText"}}
                    valueObj[value] = radioButton->text();
                    break; // 单选只选一个
                }
            }
        }
        if (!valueObj.isEmpty()) {
            answer[field] = valueObj;
        }
    }
    else if (type == "Checkbox") {
        QJsonObject valueObj;
        for (QWidget* widget : widgets) {
            if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                if (checkBox->property("field").toString() == field && checkBox->isChecked()) {
                    QString value = checkBox->property("value").toString();
                    // 格式: {"questionId": {"optionId1": "optionText1", "optionId2": "optionText2"}}
                    valueObj[value] = checkBox->text();
                }
            }
        }
        if (!valueObj.isEmpty()) {
            answer[field] = valueObj;
        }
    }
    else if (type == "Select") {
        for (QWidget* widget : widgets) {
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                if (comboBox->property("field").toString() == field) {
                    int currentIndex = comboBox->currentIndex();
                    if (currentIndex > 0) { // 排除"请选择"
                        QString value = comboBox->currentData().toString();
                        // 格式: {"questionId": {"optionId": "optionId"}}
                        answer[field] = QJsonObject{{value, value}};
                    }
                    break;
                }
            }
        }
    }
    else if (type == "MultipleBlank") {
        QJsonObject valueObj;
        for (QWidget* widget : widgets) {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                if (lineEdit->property("field").toString() == field) {
                    QString value = lineEdit->text();
                    QString subField = lineEdit->property("subField").toString();
                    if (!value.isEmpty() && !subField.isEmpty()) {
                        // 格式: {"questionId": {"subFieldId": "value"}}
                        valueObj[subField] = value;
                    }
                }
            }
        }
        if (!valueObj.isEmpty()) {
            answer[field] = valueObj;
        }
    }
    else if (type == "Score" || type == "Nps") {
        for (QWidget* widget : widgets) {
            if (QSlider* slider = qobject_cast<QSlider*>(widget)) {
                if (slider->property("field").toString() == field) {
                    int value = slider->value();
                    // 格式: {"questionId": {"questionId": "value"}}
                    answer[field] = QJsonObject{{field, QString::number(value)}};
                    break;
                }
            }
        }
    }
    // 处理有填空的Radio和Checkbox选项
    processOptionsWithBlankInputs(page, field, answer);

    return answer;
}

void SurveyFormWidget::processOptionsWithBlankInputs(QWidget* page, const QString& field, QJsonObject& answer)
{
    FUNCTION_LOG();
    // 处理单选按钮中的填空输入框
    QList<QRadioButton*> radioButtons = page->findChildren<QRadioButton*>();
    for (QRadioButton* radioButton : radioButtons) {
        if (radioButton->property("field").toString() == field && radioButton->isChecked()) {
            // 查找关联的输入框
            QLineEdit* lineEdit = findAssociatedLineEdit(radioButton);
            if (lineEdit && lineEdit->isEnabled() && !lineEdit->text().isEmpty()) {
                QString value = lineEdit->text();
                QString subField = lineEdit->property("subField").toString();
                QString id = lineEdit->property("id").toString();

                if (!answer.contains(field)) {
                    answer[field] = QJsonObject();
                }

                QJsonObject obj = answer[field].toObject();
                if (!obj.contains(subField)) {
                    obj[subField] = QJsonObject();
                }

                QJsonObject subObj = obj[subField].toObject();
                subObj[id] = value;
                obj[subField] = subObj;
                answer[field] = obj;
            }
            break; // 单选只处理一个
        }
    }

    // 处理复选框中的填空输入框
    QList<QCheckBox*> checkBoxes = page->findChildren<QCheckBox*>();
    for (QCheckBox* checkBox : checkBoxes) {
        if (checkBox->property("field").toString() == field && checkBox->isChecked()) {
            // 查找关联的输入框
            QLineEdit* lineEdit = findAssociatedLineEdit(checkBox);
            if (lineEdit && lineEdit->isEnabled() && !lineEdit->text().isEmpty()) {
                QString value = lineEdit->text();
                QString subField = lineEdit->property("subField").toString();
                QString id = lineEdit->property("id").toString();

                if (!answer.contains(field)) {
                    answer[field] = QJsonObject();
                }

                QJsonObject obj = answer[field].toObject();
                if (!obj.contains(subField)) {
                    obj[subField] = QJsonObject();
                }

                QJsonObject subObj = obj[subField].toObject();
                subObj[id] = value;
                obj[subField] = subObj;
                answer[field] = obj;
            }
        }
    }
}

QLineEdit* SurveyFormWidget::findAssociatedLineEdit(QWidget* optionWidget)
{
    FUNCTION_LOG();
    // 在同一个布局中查找关联的输入框
    QLayout* parentLayout = optionWidget->parentWidget()->layout();
    if (!parentLayout) return nullptr;

    for (int i = 0; i < parentLayout->count(); ++i) {
        QLayoutItem* item = parentLayout->itemAt(i);
        if (item && item->widget()) {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(item->widget())) {
                // 检查是否在同一个水平布局中
                if (lineEdit->parentWidget() == optionWidget->parentWidget()) {
                    return lineEdit;
                }
            }
        }
    }

    return nullptr;
}

void SurveyFormWidget::restoreAnswer(int questionIndex)
{
    FUNCTION_LOG();
    if (!m_answerCache.contains(questionIndex)) {
        return;
    }

    QJsonObject savedAnswer = m_answerCache[questionIndex];
    if (savedAnswer.isEmpty()) {
        return;
    }

    // 获取当前页面
    QWidget *page = m_questionPages[questionIndex];
    if (!page) return;

    // 获取题目信息
    QJsonObject question = m_showQuestions[questionIndex];
    QString field = question["id"].toString();
    QString type = question["type"].toString();

    if (!savedAnswer.contains(field)) {
        return;
    }

    QJsonObject fieldAnswer = savedAnswer[field].toObject();

    // 根据题型恢复答案
    if (type == "FillBlank") {
        QLineEdit* lineEdit = page->findChild<QLineEdit*>();
        if (lineEdit) {
            // 获取第一个答案值
            if (!fieldAnswer.isEmpty()) {
                QString value = fieldAnswer.begin().value().toString();
                lineEdit->setText(value);
            }
        }
    }
    else if (type == "Textarea") {
        QTextEdit* textEdit = page->findChild<QTextEdit*>();
        if (textEdit) {
            if (!fieldAnswer.isEmpty()) {
                QString value = fieldAnswer.begin().value().toString();
                textEdit->setPlainText(value);
            }
        }
    }
    else if (type == "Radio") {
        QList<QRadioButton*> radioButtons = page->findChildren<QRadioButton*>();
        for (QRadioButton* radio : radioButtons) {
            QString optionValue = radio->property("value").toString();
            if (fieldAnswer.contains(optionValue)) {
                radio->setChecked(true);

                // 处理关联的填空输入框
                QLineEdit* lineEdit = findAssociatedLineEdit(radio);
                if (lineEdit) {
                    // 查找填空输入框的答案
                    QString subField = lineEdit->property("subField").toString();
                    if (fieldAnswer.contains(subField)) {
                        QJsonObject subAnswer = fieldAnswer[subField].toObject();
                        if (!subAnswer.isEmpty()) {
                            QString blankValue = subAnswer.begin().value().toString();
                            lineEdit->setText(blankValue);
                            lineEdit->setEnabled(true);
                        }
                    }
                }
                break;
            }
        }
    }
    else if (type == "Checkbox") {
        QList<QCheckBox*> checkBoxes = page->findChildren<QCheckBox*>();
        for (QCheckBox* checkBox : checkBoxes) {
            QString optionValue = checkBox->property("value").toString();
            if (fieldAnswer.contains(optionValue)) {
                checkBox->setChecked(true);

                // 处理关联的填空输入框
                QLineEdit* lineEdit = findAssociatedLineEdit(checkBox);
                if (lineEdit) {
                    // 查找填空输入框的答案
                    QString subField = lineEdit->property("subField").toString();
                    if (fieldAnswer.contains(subField)) {
                        QJsonObject subAnswer = fieldAnswer[subField].toObject();
                        if (!subAnswer.isEmpty()) {
                            QString blankValue = subAnswer.begin().value().toString();
                            lineEdit->setText(blankValue);
                            lineEdit->setEnabled(true);
                        }
                    }
                }
            }
        }
    }
    else if (type == "Select") {
        QComboBox* comboBox = page->findChild<QComboBox*>();
        if (comboBox) {
            // 查找匹配的选项
            for (int i = 0; i < comboBox->count(); ++i) {
                QString itemData = comboBox->itemData(i).toString();
                if (fieldAnswer.contains(itemData)) {
                    comboBox->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
    else if (type == "MultipleBlank") {
        QList<QLineEdit*> lineEdits = page->findChildren<QLineEdit*>();
        for (QLineEdit* lineEdit : lineEdits) {
            QString subField = lineEdit->property("subField").toString();
            if (fieldAnswer.contains(subField)) {
                QString value = fieldAnswer[subField].toString();
                lineEdit->setText(value);
            }
        }
    }
    else if (type == "Score" || type == "Nps") {
        QSlider* slider = page->findChild<QSlider*>();
        if (slider && fieldAnswer.contains(field)) {
            QString valueStr = fieldAnswer[field].toString();
            bool ok;
            int value = valueStr.toInt(&ok);
            if (ok) {
                slider->setValue(value);
            }
        }
    }
}

void SurveyFormWidget::onPrevClicked()
{
    FUNCTION_LOG();
    // 保存当前题目的答案
    saveCurrentAnswer(m_currentQuestionIndex);

    if (m_currentQuestionIndex > 0) {
        int prevIndex = m_currentQuestionIndex - 1;
        renderQuestionPage(prevIndex);

        // 恢复上一题的答案
        restoreAnswer(prevIndex);

        m_stackedWidget->setCurrentIndex(prevIndex);
        m_currentQuestionIndex = prevIndex;
        updateProgress(m_showNum);
        m_scrollArea->verticalScrollBar()->setValue(0);

        // 更新按钮状态
        m_nextButton->setVisible(true);
        m_submitButton->setVisible(false);
        m_prevButton->setEnabled(prevIndex > 0);
    }
}

void SurveyFormWidget::onBackToSurveyListClicked()
{
    FUNCTION_LOG();
    LocationManager::instance().stopContinuousLocationUpdates();
    emit backToSurveyList();
}

void SurveyFormWidget::evaluateJumpLogic(int currentQuestionIndex)
{
    // 这里可以实现跳转逻辑的评估
    // 目前仅作占位，实际项目中需要根据题目答案和逻辑规则来判断是否跳转
}

int SurveyFormWidget::getNextQuestionIndex(int currentQuestionIndex)
{
    FUNCTION_LOG();
    // 检查当前题目是否有跳转规则
    if (currentQuestionIndex >= 0 && currentQuestionIndex < m_showQuestions.size()) {
        QJsonObject question = m_showQuestions[currentQuestionIndex];
        QJsonObject attribute = question["attribute"].toObject();
        
        // 检查是否有jumpRule
        if (attribute.contains("jumpRule") && !attribute["jumpRule"].toString().isEmpty()) {
            QString jumpRule = attribute["jumpRule"].toString();
            
            // 收集当前答案用于表达式计算
            QJsonObject currentAnswers = collectAnswers();
            
            // 解析跳转规则表达式
            if (evaluateExpression(jumpRule, currentAnswers)) {
                // 如果表达式计算结果为true，需要解析跳转目标
                // 处理jump函数表达式，如 "jump('questionId')"
                QRegularExpression jumpRe("jump\\s*\\(\\s*['\"]?([\\w\\d]+)['\"]?\\s*\\)");
                QRegularExpressionMatch jumpMatch = jumpRe.match(jumpRule);
                if (jumpMatch.hasMatch()) {
                    QString targetQuestionId = jumpMatch.captured(1);
                    int targetIndex = findQuestionIndexById(targetQuestionId);
                    if (targetIndex >= 0 && targetIndex < m_showQuestions.size()) {
                        return targetIndex;
                    }
                }
                
                // 处理直接指定目标题号的表达式，如 "true && 'questionId'"
                QRegularExpression idRe("'([\\w\\d]+)'");
                QRegularExpressionMatch idMatch = idRe.match(jumpRule);
                if (idMatch.hasMatch()) {
                    QString targetQuestionId = idMatch.captured(1);
                    int targetIndex = findQuestionIndexById(targetQuestionId);
                    if (targetIndex >= 0 && targetIndex < m_showQuestions.size()) {
                        return targetIndex;
                    }
                }
                
                // 处理条件表达式中的题目ID，如 "#{axq2}==#{axq2.feg4}" 中的目标题目
                // 这种情况下，如果条件满足，跳转到下一题
            }
        }
    }
    
    // 默认返回下一题
    return currentQuestionIndex + 1;
}

bool SurveyFormWidget::evaluateFinishRule(int currentQuestionIndex)
{
    FUNCTION_LOG();
    // 检查当前题目是否有结束规则
    if (currentQuestionIndex >= 0 && currentQuestionIndex < m_showQuestions.size()) {
        QJsonObject question = m_showQuestions[currentQuestionIndex];
        QJsonObject attribute = question["attribute"].toObject();
        
        // 检查是否有finishRule
        if (attribute.contains("finishRule") && !attribute["finishRule"].toString().isEmpty()) {
            QString finishRule = attribute["finishRule"].toString();
            
            // 收集当前答案用于表达式计算
            QJsonObject currentAnswers = collectAnswers();
            
            // 解析结束规则表达式
            return evaluateExpression(finishRule, currentAnswers);
        }
    }
    
    return false; // 默认不结束
}

bool SurveyFormWidget::evaluateGlobalRules()
{
    FUNCTION_LOG();
    // 获取问卷属性中的全局规则
    QJsonObject survey = m_schema["survey"].toObject();
    QJsonObject surveyAttribute = survey["attribute"].toObject();

    if (surveyAttribute.contains("globalRule") && surveyAttribute["globalRule"].isArray()) {
        QJsonArray globalRules = surveyAttribute["globalRule"].toArray();
        QJsonObject currentAnswers = collectAnswers();

        // 遍历所有全局规则
        for (int i = 0; i < globalRules.size(); ++i) {
            QString rule = globalRules[i].toString();

            // 尝试将规则解析为JSON对象
            QJsonDocument ruleDoc = QJsonDocument::fromJson(rule.toUtf8());
            if (!ruleDoc.isNull() && ruleDoc.isObject()) {
                QJsonObject ruleObj = ruleDoc.object();

                // 检查是否为跳转类型的全局规则
                if (ruleObj.contains("conditionItem") && ruleObj.contains("result")) {
                    // 处理条件逻辑
                    bool conditionResult = evaluateGlobalRuleConditions(ruleObj, currentAnswers);

                    // 如果条件满足，处理结果
                    if (conditionResult) {
                        QJsonArray results = ruleObj["result"].toArray();
                        for (int j = 0; j < results.size(); ++j) {
                            QJsonObject result = results[j].toObject();
                            if (result["type"].toString() == "jump") {
                                QString targetQuestionId = result["qId"].toString();
                                int targetIndex = findQuestionIndexById(targetQuestionId);
                                if (targetIndex != -1) {
                                    // 执行跳转
                                    renderQuestionPage(targetIndex);
                                    m_stackedWidget->setCurrentIndex(targetIndex);
                                    m_currentQuestionIndex = targetIndex;
                                    updateProgress(m_showNum);
                                    m_scrollArea->verticalScrollBar()->setValue(0);

                                    // 更新按钮状态
                                    m_prevButton->setEnabled(targetIndex > 0);
                                    m_nextButton->setVisible(targetIndex < m_showQuestions.size() - 1);
                                    m_submitButton->setVisible(targetIndex == m_showQuestions.size() - 1);

                                    // 对于跳转类型的全局规则，我们不需要阻止用户继续操作
                                    // 只是跳转到指定题目
                                    return true;
                                }
                            }
                        }
                    }
                    // 如果条件不满足，继续检查其他全局规则
                    continue;
                }
            }

            // 如果不是结构化规则，按表达式处理
            if (!evaluateExpression(rule, currentAnswers)) {
                // 可以添加提示信息，告诉用户全局规则验证失败
                return false;
            }
        }
    }

    // 如果没有全局规则或所有规则都通过，则返回true
    return true;
}

void SurveyFormWidget::updateRecordTime(qint64 duration)
{

}

void SurveyFormWidget::handleRecorderError()
{

}

void SurveyFormWidget::capturePhoto()
{
    FUNCTION_LOG();
    if (!m_camera->isActive()) {
        qWarning() << "摄像头未激活，无法拍照";
        m_captureTimer->stop();
        return;
    }

    // 生成照片文件名
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".jpg";
    QString filePath = m_photoDir.absoluteFilePath(fileName);

    // 创建图片捕获器并设置到捕获会话
    QImageCapture *imageCapture = new QImageCapture(captureSession);
    captureSession->setImageCapture(imageCapture);
    imageCapture->setQuality(QImageCapture::HighQuality);

    // 连接图片捕获信号
    connect(imageCapture, &QImageCapture::imageCaptured, this, [this, filePath](int id, const QImage &preview) {
        Q_UNUSED(id)
        // 保存图片到文件
        if (preview.save(filePath, "JPG")) {
            m_capturedPhotos.append(filePath);
            qDebug() << "照片已保存:" << filePath;
        } else {
            qWarning() << "保存照片失败:" << filePath;
        }
    });

    // 拍摄完成后删除imageCapture对象
    connect(imageCapture, &QImageCapture::imageSaved, this, [imageCapture]() {
        // 从捕获会话中断开连接
        imageCapture->captureSession()->setImageCapture(nullptr);
        imageCapture->deleteLater();
    });

    // 连接错误信号
    connect(imageCapture, &QImageCapture::errorOccurred, this, [imageCapture](int id, QImageCapture::Error error, const QString &errorString) {
        Q_UNUSED(id)
        qWarning() << "拍照发生错误:" << errorString << "错误代码:" << error;
        // 从捕获会话中断开连接
        imageCapture->captureSession()->setImageCapture(nullptr);
        imageCapture->deleteLater();
    });

    // 捕获图片
    int id = imageCapture->captureToFile(filePath);
    if (id == -1) {
        qWarning() << "拍照失败";
        // 从捕获会话中断开连接
        captureSession->setImageCapture(nullptr);
        imageCapture->deleteLater();
    }
}

void SurveyFormWidget::onCameraActiveChanged()
{
    FUNCTION_LOG();
    if (m_camera->isActive()) {
        qDebug() << "摄像头已激活";
    } else {
        qDebug() << "摄像头已停止";
    }
}

bool SurveyFormWidget::evaluateExpression(const QString& expression, const QJsonObject& answers)
{
    FUNCTION_LOG();
    // 这是一个简化的表达式计算器实现
    // 实际项目中应该实现一个完整的表达式解析器
    
    // 处理空表达式
    if (expression.trimmed().isEmpty()) {
        return false;
    }
    
    // 处理简单布尔值
    if (expression.trimmed().compare("true", Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (expression.trimmed().compare("false", Qt::CaseInsensitive) == 0) {
        return false;
    }
    
    // 处理比较两个题目答案的表达式，如 "#{questionId1} == #{questionId2.subField}"
    QRegularExpression twoVarsRe("#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}\\s*(==|!=)\\s*#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}");
    QRegularExpressionMatch twoVarsMatch = twoVarsRe.match(expression);
    if (twoVarsMatch.hasMatch()) {
        QString questionId1 = twoVarsMatch.captured(1);
        QString subField1 = twoVarsMatch.captured(2);
        QString operatorStr = twoVarsMatch.captured(3);
        QString questionId2 = twoVarsMatch.captured(4);
        QString subField2 = twoVarsMatch.captured(5);
        
        QString value1, value2;
        bool hasValue1 = false, hasValue2 = false;
        
        // 获取第一个题目的值
        if (answers.contains(questionId1)) {
            QJsonObject answerObj1 = answers[questionId1].toObject();
            if (!subField1.isEmpty() && answerObj1.contains(subField1)) {
                value1 = answerObj1[subField1].toString();
                hasValue1 = true;
            } else if(!subField1.isEmpty()){
                QJsonArray array = m_showQuestions[m_currentQuestionIndex]["children"].toArray();
                foreach(auto obj, array)
                {
                    if(obj.toObject()["id"].toString() == subField1)
                    {
                        value1 = obj.toObject()["title"].toString();
                        hasValue1 = true;
                        break;
                    }
                }
            }
            else
            {
                // 遍历查找值
                for (auto it = answerObj1.begin(); it != answerObj1.end(); ++it) {
                    if (it.value().isString()) {
                        value1 = it.value().toString();
                        hasValue1 = true;
                        break;
                    }
                }
            }
        }
        
        // 获取第二个题目的值
        if (answers.contains(questionId2)) {
            QJsonObject answerObj2 = answers[questionId2].toObject();
            if (!subField2.isEmpty() && answerObj2.contains(subField2)) {
                value2 = answerObj2[subField2].toString();
                hasValue2 = true;
            }else if(!subField2.isEmpty()){
                QJsonArray array = m_showQuestions[m_currentQuestionIndex]["children"].toArray();
                foreach(auto obj, array)
                {
                    if(obj.toObject()["id"].toString() == subField2)
                    {
                        value2 = obj.toObject()["title"].toString();
                        hasValue2 = true;
                        break;
                    }
                }
            } else {
                // 遍历查找值
                for (auto it = answerObj2.begin(); it != answerObj2.end(); ++it) {
                    if (it.value().isString()) {
                        value2 = it.value().toString();
                        hasValue2 = true;
                        break;
                    }
                }
            }
        }
        
        if (operatorStr == "==") {
            return hasValue1 && hasValue2 && (value1 == value2);
        } else if (operatorStr == "!=") {
            if (!hasValue1 || !hasValue2) {
                return true; // 如果任一值不存在，则认为不相等
            }
            return value1 != value2;
        }
    }
    
    // 处理等于比较表达式，如 "#{questionId} == 'value'"
    QRegularExpression eqRe("#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}\\s*(==|!=)\\s*['\"](.*)['\"]");
    QRegularExpressionMatch eqMatch = eqRe.match(expression);
    if (eqMatch.hasMatch()) {
        QString questionId = eqMatch.captured(1);
        QString subField = eqMatch.captured(2);
        QString operatorStr = eqMatch.captured(3);
        QString expectedValue = eqMatch.captured(4);
        
        // 查找该题目的答案
        if (answers.contains(questionId)) {
            QJsonObject answerObj = answers[questionId].toObject();
            bool hasValue = false;
            QString actualValue;
            
            if (!subField.isEmpty() && answerObj.contains(subField)) {
                // 处理子字段
                QJsonObject subObj = answerObj[subField].toObject();
                for (auto it = subObj.begin(); it != subObj.end(); ++it) {
                    if (it.value().isString()) {
                        actualValue = it.value().toString();
                        hasValue = true;
                        break;
                    }
                }
            } else {
                // 遍历答案对象，查找匹配的值
                for (auto it = answerObj.begin(); it != answerObj.end(); ++it) {
                    if (it.value().isString()) {
                        actualValue = it.value().toString();
                        hasValue = true;
                        break;
                    }
                }
            }
            
            if (operatorStr == "==") {
                return hasValue && (actualValue == expectedValue);
            } else if (operatorStr == "!=") {
                return !hasValue || (actualValue != expectedValue);
            }
        } else {
            // 如果问题没有答案，根据操作符决定返回值
            if (operatorStr == "!=" && expectedValue.isEmpty()) {
                return true;
            }
        }
    }
    
    // 处理存在性检查表达式，如 "#{questionId} != ''"
    QRegularExpression existRe("#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}\\s*!=\\s*['\"]['\"]");
    QRegularExpressionMatch existMatch = existRe.match(expression);
    if (existMatch.hasMatch()) {
        QString questionId = existMatch.captured(1);
        QString subField = existMatch.captured(2);
        
        // 查找该题目的答案
        if (answers.contains(questionId)) {
            QJsonObject answerObj = answers[questionId].toObject();
            if (!subField.isEmpty()) {
                return answerObj.contains(subField) && !answerObj[subField].toObject().isEmpty();
            }
            return !answerObj.isEmpty();
        }
        return false;
    }
    
    // 处理存在性检查表达式，如 "#{questionId} == ''"
    QRegularExpression notExistRe("#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}\\s*==\\s*['\"]['\"]");
    QRegularExpressionMatch notExistMatch = notExistRe.match(expression);
    if (notExistMatch.hasMatch()) {
        QString questionId = notExistMatch.captured(1);
        QString subField = notExistMatch.captured(2);
        
        // 查找该题目的答案
        if (answers.contains(questionId)) {
            QJsonObject answerObj = answers[questionId].toObject();
            if (!subField.isEmpty()) {
                return !answerObj.contains(subField) || answerObj[subField].toObject().isEmpty();
            }
            return answerObj.isEmpty();
        }
        return true;
    }
    
    // 处理跳转表达式，如 "jump('questionId')"
    QRegularExpression jumpRe("jump\\s*\\(\\s*['\"]?([\\w\\d]+)['\"]?\\s*\\)");
    QRegularExpressionMatch jumpMatch = jumpRe.match(expression);
    if (jumpMatch.hasMatch()) {
        // jump函数表示满足条件，应该执行跳转
        return true;
    }
    
    // 处理包含函数，如 "includes(#{questionId}, 'value')"
    QRegularExpression includesRe("includes\\s*\\(\\s*#\\{([\\w\\d]+)(?:\\.([\\w\\d]+))?\\}\\s*,\\s*['\"](.*)['\"]\\s*\\)");
    QRegularExpressionMatch includesMatch = includesRe.match(expression);
    if (includesMatch.hasMatch()) {
        QString questionId = includesMatch.captured(1);
        QString subField = includesMatch.captured(2);
        QString valueToCheck = includesMatch.captured(3);
        
        if (answers.contains(questionId)) {
            QJsonObject answerObj = answers[questionId].toObject();
            if (!subField.isEmpty() && answerObj.contains(subField)) {
                QJsonObject subObj = answerObj[subField].toObject();
                for (auto it = subObj.begin(); it != subObj.end(); ++it) {
                    if (it.value().toString() == valueToCheck) {
                        return true;
                    }
                }
            } else {
                // 检查答案中是否包含指定值
                for (auto it = answerObj.begin(); it != answerObj.end(); ++it) {
                    if (it.value().toString() == valueToCheck) {
                        return true;
                    }
                }
            }
        }
    }
    
    // 更复杂的表达式处理需要实现完整的表达式解析器
    // 这里只是简单示例
    
    // 默认情况下，尝试解析为布尔表达式
    return false;
}

bool SurveyFormWidget::evaluateGlobalRuleConditions(const QJsonObject &rule, const QJsonObject &currentAnswers)
{
    FUNCTION_LOG();
    QJsonArray conditionItems = rule["conditionItem"].toArray();
    QString conditionLogic = rule["conditionLogic"].toString("AND");

    bool finalResult = (conditionLogic == "AND") ? true : false;

    for (int i = 0; i < conditionItems.size(); ++i) {
        QJsonObject conditionItem = conditionItems[i].toObject();
        QString qId = conditionItem["qId"].toString();
        QString condition = conditionItem["condition"].toString();
        QJsonArray oIds = conditionItem["oId"].toArray();

        bool conditionResult = false;

        // 检查当前答案中是否有对应的题目
        if (currentAnswers.contains(qId)) {
            QJsonObject questionAnswer = currentAnswers[qId].toObject();

            if (condition == "CHECKED") {
                // 检查是否选中了指定的选项
                for (int j = 0; j < oIds.size(); ++j) {
                    QString oId = oIds[j].toString();
                    if (questionAnswer.contains(oId)) {
                        conditionResult = true;
                        break;
                    }
                }
            }
        }

        // 根据逻辑运算符更新最终结果
        if (conditionLogic == "AND") {
            finalResult = finalResult && conditionResult;
        } else if (conditionLogic == "OR") {
            finalResult = finalResult || conditionResult;
        }

        // 对于AND逻辑，如果任何一个条件为false，可以提前退出
        if (conditionLogic == "AND" && !finalResult) {
            break;
        }
    }

    return finalResult;
}

int SurveyFormWidget::findQuestionIndexById(const QString& questionId)
{
    FUNCTION_LOG();
    for (int i = 0; i < m_showQuestions.size(); ++i) {
        if (m_showQuestions[i]["id"].toString() == questionId) {
            return i;
        }
    }
    return -1; // 未找到
}

bool SurveyFormWidget::isQuestionRequired(int questionIndex)
{
    FUNCTION_LOG();
    if (questionIndex < 0 || questionIndex >= m_showQuestions.size()) {
        return false;
    }
    
    QJsonObject question = m_showQuestions[questionIndex];
    QJsonObject attribute = question["attribute"].toObject();
    return attribute["required"].toBool();
}

bool SurveyFormWidget::isQuestionAnswered(int questionIndex)
{
    FUNCTION_LOG();
    if (questionIndex < 0 || questionIndex >= m_showQuestions.size()) {
        return false;
    }
    
    // 获取当前页面
    QWidget *page = m_questionPages[questionIndex];
    QList<QWidget*> widgets = page->findChildren<QWidget*>();
    
    QJsonObject question = m_showQuestions[questionIndex];
    QString type = question["type"].toString();
    QString field = question["id"].toString();
    
    // 根据题型检查是否已回答
    if (type == "FillBlank") {
        for (QWidget* widget : widgets) {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                if (lineEdit->property("field").toString() == field) {
                    return !lineEdit->text().trimmed().isEmpty();
                }
            }
        }
    }
    else if (type == "Textarea") {
        for (QWidget* widget : widgets) {
            if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                if (textEdit->property("field").toString() == field) {
                    return !textEdit->toPlainText().trimmed().isEmpty();
                }
            }
        }
    }
    else if (type == "Radio") {
        for (QWidget* widget : widgets) {
            if (QRadioButton* radioButton = qobject_cast<QRadioButton*>(widget)) {
                if (radioButton->property("field").toString() == field) {
                    if (radioButton->isChecked()) {
                        return true;
                    }
                }
            }
        }
        return false; // 没有选中任何选项
    }
    else if (type == "Checkbox") {
        for (QWidget* widget : widgets) {
            if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                if (checkBox->property("field").toString() == field) {
                    if (checkBox->isChecked()) {
                        return true;
                    }
                }
            }
        }
        return false; // 没有选中任何选项
    }
    else if (type == "Select") {
        for (QWidget* widget : widgets) {
            if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                if (comboBox->property("field").toString() == field) {
                    return comboBox->currentIndex() > 0; // 排除"请选择"
                }
            }
        }
    }
    else if (type == "MultipleBlank") {
        for (QWidget* widget : widgets) {
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                if (lineEdit->property("field").toString() == field) {
                    if (lineEdit->text().trimmed().isEmpty()) {
                        return false; // 有任何一个子字段为空就不算完成
                    }
                }
            }
        }
        return true; // 所有子字段都已填写
    }
    else if (type == "Score" || type == "Nps") {
        // 评分题默认已回答（因为有默认值0）
        return true;
    }
    
    // 其他类型默认返回true
    return true;
}

void SurveyFormWidget::onSubmitClicked()
{
    FUNCTION_LOG();
    // 检查最后一题是否为必填题但未回答
    if (isQuestionRequired(m_currentQuestionIndex) && !isQuestionAnswered(m_currentQuestionIndex)) {
        QMessageBox::warning(this, "提示", "这是必填题，请完成作答后再提交");
        return;
    }
    
    m_endTime = QDateTime::currentMSecsSinceEpoch();

    // 更新进度条到100%
    if (m_progressBar && m_progressLabel) {
        m_progressBar->setValue(100);
        m_progressLabel->setText(QString("进度: 100% (%1/%1)").arg(m_showQuestions.size()));
    }

    // 停止录音
    if(m_autoCaptureEnabled)
    {
        stopAutoCapture();
    }

    // 停止录音
    if(SettingsManager::getInstance().getValue("survey/autoRecord").toBool())
    {
        StopRecord();
    }

    // 如果有文件正在上传，则等待上传完成后再提交
    if (m_isUploading) {
        m_shouldSubmitAfterUpload = true;
        // QMessageBox::information(this, "正在上传", "文件正在上传中，请稍候...");
        return;
    }

    // 清空答案缓存
    m_answerCache.clear();

    // 停止定位
    LocationManager::instance().stopContinuousLocationUpdates();

    emit submitSurvey(collectAnswers());
}

QJsonObject SurveyFormWidget::collectAnswers()
{
    FUNCTION_LOG();
    QJsonObject answers;

    // 遍历所有题目页面收集答案
    for (int i = 0; i < m_questionPages.size(); ++i) {
        QWidget* page = m_questionPages[i];
        QList<QWidget*> widgets = page->findChildren<QWidget*>();

        for (QWidget* widget : widgets) {
            QString field = widget->property("field").toString();
            if (field.isEmpty()) continue;

            // 根据控件类型收集答案
            if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                QString value = lineEdit->text();
                if (!value.isEmpty()) {
                    // 检查是否是选项中的填空输入框
                    QString subField = widget->property("subField").toString();
                    QString id = widget->property("id").toString();
                    if (!subField.isEmpty()) {
                        // 这是选项中的填空输入框，需要特殊处理
                        if (!answers.contains(field)) {
                            answers[field] = QJsonObject();
                        }
                        QJsonObject obj = answers[field].toObject();
                        QJsonObject cus;
                        cus[id] = value;
                        obj[subField] = cus;
                        answers[field] = obj;
                    } else {
                        // 普通填空题
                        answers[field] = QJsonObject{{widget->property("id").toString(), value}};
                    }
                }
            }
            else if (QTextEdit* textEdit = qobject_cast<QTextEdit*>(widget)) {
                QString value = textEdit->toPlainText();
                if (!value.isEmpty()) {
                    answers[field] = QJsonObject{{widget->property("id").toString(), value}};
                }
            }
            else if (QRadioButton* radioButton = qobject_cast<QRadioButton*>(widget)) {
                if (radioButton->isChecked()) {
                    QString value = radioButton->property("value").toString();
                    // 单选题答案格式: {"questionId": {"optionId": "optionId"}}
                    QJsonObject valueObj;
                    valueObj[value] = radioButton->text();
                    answers[field] = valueObj;
                }
            }
            else if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                if (checkBox->isChecked()) {
                    QString value = checkBox->property("value").toString();
                    // 多选题答案格式: {"questionId": {"optionId1": "optionId1", "optionId2": "optionId2"}}
                    if (!answers.contains(field)) {
                        answers[field] = QJsonObject();
                    }
                    QJsonObject obj = answers[field].toObject();
                    obj[value] = checkBox->text();
                    answers[field] = obj;
                }
            }
            else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                int currentIndex = comboBox->currentIndex();
                if (currentIndex > 0) { // 排除"请选择"
                    QString value = comboBox->currentData().toString();
                    // 下拉题答案格式: {"questionId": {"optionId": "optionId"}}
                    QJsonObject valueObj;
                    valueObj[value] = value;
                    answers[field] = valueObj;
                }
            }
            else if (QSlider* slider = qobject_cast<QSlider*>(widget)) {
                int value = slider->value();
                // 评分题答案格式: {"questionId": {"questionId": "value"}}
                answers[field] = QJsonObject{{field, QString::number(value)}};
            }
        }
    }

    // 处理上传题的答案数据
    for (int i=0;i<m_uploadedFiles.size();i++) {

        QString field = m_uploadedFiles[i].toObject()["field"].toString();
        QString fileId = m_uploadedFiles[i].toObject()["id"].toString();
        QString sub_id = m_uploadedFiles[i].toObject()["subField"].toString();

        // 按照服务器要求的格式构建上传题答案
        // 格式为: {"questionId": {"fileId": "fileId"}}
        if (!fileId.isEmpty()) {
            if(answers[field].toObject().empty())
                answers[field] = QJsonObject();
            QJsonObject obj = answers[field].toObject();
            if(obj[sub_id].toArray().empty())
            {
                obj[sub_id] = QJsonArray();
            }
            QJsonArray array = obj[sub_id].toArray();
            array.append(fileId);
            obj[sub_id] = array;
            answers[field] = obj;
        }
    }

    if(!m_locationObj.isEmpty())
    {
        // 添加位置信息到客户端信息中
        LocationManager::LocationInfo location = LocationManager::instance().getLastKnownLocation();
        QString str;
        if (location.isValid) {
            str = QString("lat:%1,lon:%2,alt:%3").arg(QString::number(location.latitude)).arg(QString::number(location.longitude)).arg(QString::number(location.altitude));
        }
        answers[m_locationObj["id"].toString()] = QJsonObject{{m_locationObj["children"].toArray().at(0).toObject()["id"].toString(), str}};
    }

    qDebug()<<answers;

    return answers;
}

bool SurveyFormWidget::event(QEvent *event)
{

    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    return QWidget::event(event);
}

void SurveyFormWidget::wheelEvent(QWheelEvent *event)
{

    // 处理滚轮事件，使滚动更平滑
    QScrollBar *vScrollBar = m_scrollArea->verticalScrollBar();
    if (vScrollBar) {
        int delta = event->angleDelta().y();
        vScrollBar->setValue(vScrollBar->value() - delta / 2);
        event->accept();
    } else {
        QWidget::wheelEvent(event);
    }
}

bool SurveyFormWidget::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pan = event->gesture(Qt::PanGesture)) {
        QPanGesture *panGesture = static_cast<QPanGesture *>(pan);
        QScrollBar *vScrollBar = m_scrollArea->verticalScrollBar();
        if (vScrollBar) {
            QPointF delta = panGesture->delta();
            vScrollBar->setValue(vScrollBar->value() - delta.y());
            return true;
        }
    }
    return false;
}

void SurveyFormWidget::mousePressEvent(QMouseEvent *event)
{
    // 记录触摸起始点
    if (event->button() == Qt::LeftButton) {
        m_lastTouchPoint = event->pos();
        m_isTouching = true;
        m_lastScrollValue = m_scrollArea->verticalScrollBar()->value();
    }
    QWidget::mousePressEvent(event);
}

void SurveyFormWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 处理触摸拖拽
    if (m_isTouching && (event->buttons() & Qt::LeftButton)) {
        int delta = m_lastTouchPoint.y() - event->pos().y();
        QScrollBar *vScrollBar = m_scrollArea->verticalScrollBar();
        if (vScrollBar) {
            vScrollBar->setValue(m_lastScrollValue + delta);
        }
    }
    QWidget::mouseMoveEvent(event);
}

void SurveyFormWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // 重置触摸状态
    if (event->button() == Qt::LeftButton) {
        m_isTouching = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void SurveyFormWidget::updateProgress(int num)
{
    FUNCTION_LOG();
    if (m_showQuestions.isEmpty()) return;

    updateScrollBarVisibility();

    int progress = (m_currentQuestionIndex + 1) * 100 / num;
    m_progressBar->setValue(progress);
    m_progressLabel->setText(QString("进度: %1% (%2/%3)")
                                 .arg(progress)
                                 .arg(m_currentQuestionIndex + 1)
                                 .arg(num));
}

void SurveyFormWidget::updateScrollBarVisibility()
{
    if (!m_stackedWidget || !m_stackedWidget->currentWidget()) return;

    // 获取当前页面和滚动区域的高度
    QWidget* currentPage = m_stackedWidget->currentWidget();

    // 确保页面布局已经计算完成
    if (currentPage->layout()) {
        currentPage->layout()->activate();
    }

    // 获取页面的实际高度（包括边距等）
    int pageHeight = currentPage->sizeHint().height();
    if (pageHeight <= 0) {
        // 如果sizeHint无效，使用minimumSizeHint
        pageHeight = currentPage->minimumSizeHint().height();
    }

    // 获取视口高度
    int viewportHeight = m_scrollArea->viewport()->height();

    // 如果页面高度小于等于视口高度，隐藏滚动条
    if (pageHeight <= viewportHeight) {
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        // 根据页面高度调整滚动条范围
        adjustScrollBarRange(pageHeight);
    }

    // 强制更新布局和重绘
    m_scrollArea->updateGeometry();
    m_scrollArea->update();
}

void SurveyFormWidget::adjustScrollBarRange(int pageHeight)
{
    QScrollBar* vScrollBar = m_scrollArea->verticalScrollBar();
    if (!vScrollBar) return;

    // 获取视口高度
    int viewportHeight = m_scrollArea->viewport()->height();

    // 计算需要的滚动范围
    int neededRange = pageHeight - viewportHeight;
    if (neededRange < 0) neededRange = 0;

    // 获取当前滚动条的范围和值
    int currentMin = vScrollBar->minimum();
    int currentMax = vScrollBar->maximum();
    int currentValue = vScrollBar->value();

    // 只有当范围确实需要改变时才更新，避免不必要的信号触发
    if (currentMax != neededRange) {
        // 保存当前滚动位置的比例，以便在调整范围后保持相对位置
        double scrollRatio = 0.0;
        if (currentMax > 0) {
            scrollRatio = static_cast<double>(currentValue) / currentMax;
        }

        // 设置新的滚动范围
        vScrollBar->setRange(0, neededRange);

        // 根据比例恢复滚动位置
        if (neededRange > 0) {
            int newValue = static_cast<int>(scrollRatio * neededRange);
            vScrollBar->setValue(newValue);
        }

        // 调整滚动条步长，使其与页面内容匹配
        int pageStep = qMax(viewportHeight / 2, 50); // 页面步长为视口高度的一半，最小50像素
        int singleStep = qMax(viewportHeight / 10, 10); // 单步步长为视口高度的1/10，最小10像素

        vScrollBar->setPageStep(pageStep);
        vScrollBar->setSingleStep(singleStep);

        qDebug() << "Scroll bar adjusted - Page height:" << pageHeight
                 << "Viewport height:" << viewportHeight
                 << "Range: 0 to" << neededRange
                 << "Page step:" << pageStep
                 << "Single step:" << singleStep;
    }
}

void SurveyFormWidget::AddFile(QString id, QString path)
{
    FUNCTION_LOG();
    QJsonObject obj;
    obj[id] = path;
    m_selectedFiles.append(obj);
}

void SurveyFormWidget::requestAudioPermission()
{
    FUNCTION_LOG();
    PermissionManager::instance().requestAudioRecordingPermission([this](bool granted) {
        if (granted) {
            qDebug() << "AudioRecording permission granted, initializing Start Camera";
            StartRecord(); // 你的录音启动函数
        } else {
            qWarning() << "AudioRecording permission denied";
            // 可以在这里显示提示信息，告知用户需要权限才能使用相机
        }
    });
}

void SurveyFormWidget::StartRecord()
{
    FUNCTION_LOG();
    // 检查录音设备是否可用
    if (!mediaRecorder || !audioInput) {
        qWarning() << "录音设备未正确初始化";
        return;
    }

    // 设置为最兼容的WAV格式
    QMediaFormat format;
    format.setFileFormat(QMediaFormat::MP3);
    format.setAudioCodec(QMediaFormat::AudioCodec::MP3);

    mediaRecorder->setMediaFormat(format);
    mediaRecorder->setAudioSampleRate(44100); // 设置采样率
    mediaRecorder->setAudioChannelCount(1);   // 设置声道数
    mediaRecorder->setQuality(QMediaRecorder::HighQuality); // 设置质量
    mediaRecorder->setEncodingMode(QMediaRecorder::ConstantQualityEncoding); // 编码模式

    // 检查是否支持该格式
    if (!mediaRecorder->isAvailable()) {
        qWarning() << "当前设置的录音格式不可用";
        // 尝试使用默认格式
        // mediaRecorder->setAudioCodec(QMediaFormat::AudioCodec::Unspecified);
        return;
    }

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

    // 使用.wav扩展名
    m_output = QString("%1/%2_%3.mp3").arg(dataPath).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")).arg(m_schema["name"].toString());
    mediaRecorder->setOutputLocation(QUrl::fromLocalFile(m_output));

    // 开始录制
    mediaRecorder->record();

    // 检查是否成功开始录制
    if (mediaRecorder->error() != QMediaRecorder::NoError) {
        qWarning() << "录音启动失败:" << mediaRecorder->errorString();
    } else {
        qDebug() << "录音已启动，保存路径:" << m_output;
    }
}

void SurveyFormWidget::StopRecord()
{
    FUNCTION_LOG();
    // 确保正在录制
    if (mediaRecorder->recorderState() != QMediaRecorder::RecordingState) {
        qWarning() << "StopRecord: 录音器未处于录制状态, 当前状态:" << mediaRecorder->recorderState();
        return;
    }

    // 停止录制
    mediaRecorder->stop();

    // 主动等待录制状态变为StoppedState，最多500毫秒
    const QTime startTime = QTime::currentTime();
    while (mediaRecorder->recorderState() != QMediaRecorder::StoppedState) {
        QThread::msleep(10);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        if (startTime.msecsTo(QTime::currentTime()) > 500) {
            qWarning() << "StopRecord: 等待录音停止超时";
            break;
        }
    }

    // 检查停止过程中是否出现错误
    if (mediaRecorder->error() != QMediaRecorder::NoError) {
        qWarning() << "StopRecord: 录音停止时发生错误:" << mediaRecorder->errorString();
        return;
    }

    // 验证录音文件的有效性（WAV文件头通常为44字节）
    QFileInfo fileInfo(m_output);
    if (fileInfo.exists() && fileInfo.size() > 44) {
        qDebug() << "录音文件已成功保存:" << m_output << "大小:" << fileInfo.size() << "字节";
    } else {
        if (!fileInfo.exists()) {
            qWarning() << "StopRecord: 录音文件未生成:" << m_output;
        } else {
            qWarning() << "StopRecord: 录音文件大小异常 (可能为空):" << m_output << "大小:" << fileInfo.size() << "字节";
        }
        return;
    }

    // 上传有效的录音文件
    if (!m_autoUpLoadObj.isEmpty()) {

        AddFile(m_autoUpLoadObj["id"].toString(), m_output);
        m_pendingUploads++;  // 增加待上传计数
        m_isUploading = true; // 设置上传状态为正在上传
        emit UploadFile(m_schema["id"].toString(), m_autoUpLoadObj["id"].toString(), m_output);
    }
}

void SurveyFormWidget::initCamera()
{
    FUNCTION_LOG();
    PermissionManager::instance().requestCameraPermission([this](bool granted) {
        if (granted) {
            qDebug() << "Camera permission granted, initializing camera";
            initializeCamera();
        } else {
            qWarning() << "Camera permission denied";
            m_autoCaptureEnabled = false;
            // 可以在这里显示提示信息，告知用户需要权限才能使用相机
            // emit showMessage("无法使用相机功能，请在设置中授予相机权限");
        }
    });
}

void SurveyFormWidget::initializeCamera()
{
    FUNCTION_LOG();
    // 检查是否有可用的摄像头
    if (QMediaDevices::videoInputs().isEmpty()) {
        qWarning() << "没有找到可用的摄像头设备";
        m_autoCaptureEnabled = false;
        return;
    }

    // 设置摄像头
    QCameraDevice cameraDevice = QMediaDevices::defaultVideoInput();
    m_camera->setCameraDevice(cameraDevice);

    // 将摄像头连接到媒体捕获会话
    captureSession->setCamera(m_camera);

    qDebug() << "摄像头初始化完成";

    startAutoCapture();
}

void SurveyFormWidget::startAutoCapture()
{
    FUNCTION_LOG();
    if (!m_autoCaptureEnabled) {
        return;
    }

    // 启动摄像头
    m_camera->start();

    // 创建照片存储目录
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString dirName = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_" + m_schema["name"].toString();
    m_photoDir = QDir(cachePath);
    if (!m_photoDir.exists(dirName)) {
        m_photoDir.mkpath(dirName);
    }
    m_photoDir.cd(dirName);

    // 启动定时器，根据设置的时间间隔拍摄
    int interval = SettingsManager::getInstance().getValue("survey/captureInterval", 30).toInt();
    m_captureTimer->start(interval * 1000); // 转换为毫秒

    qDebug() << "自动拍照已启动，照片将保存到:" << m_photoDir.absolutePath() << "时间间隔:" << interval << "秒";
}

void SurveyFormWidget::stopAutoCapture()
{
    FUNCTION_LOG();
    if (m_captureTimer->isActive()) {
        m_captureTimer->stop();
    }

    if (m_camera->isActive()) {
        m_camera->stop();
    }
    m_photoDir.setNameFilters(QStringList()<<"*.jpg");
    QFileInfoList list = m_photoDir.entryInfoList();

    for(int i=0;i<list.size();i++)
    {
        // 上传全部照片
        if (!m_autoUpLoadObj.isEmpty()) {
            AddFile(m_autoUpLoadObj["id"].toString(), list.at(i).absoluteFilePath());
            m_pendingUploads++;  // 增加待上传计数
            m_isUploading = true; // 设置上传状态为正在上传
            emit UploadFile(m_schema["id"].toString(), m_autoUpLoadObj["id"].toString(), list.at(i).absoluteFilePath());
        }
    }
    qDebug() << "自动拍照已停止 "<<"上传自动拍照文件数量"<<list.size();
}
