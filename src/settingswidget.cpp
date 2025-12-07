#include "settingswidget.h"
#include <QApplication>
#include "settingsmanager.h"
#include <QMessageBox>
#include <QTextBrowser>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QScrollBar>
#include <QScreen>
#include <QMouseEvent>
#include <QApplication>
#include <QGuiApplication>
#include <QRect>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QTimer>
#include <QPainter>
#include <QStyledItemDelegate>

// 自定义ItemDelegate用于在列表项右侧绘制>符号
class ClickableItemDelegate : public QStyledItemDelegate
{
public:
    explicit ClickableItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyledItemDelegate::paint(painter, option, index);

        // 绘制>符号
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        QRect rect = opt.rect;
        QFont font = opt.font;
        font.setPixelSize(18);
        painter->setFont(font);
        painter->setPen(QColor("#4A90E2"));

        // 在右侧绘制>符号
        QRect arrowRect(rect.right() - 30, rect.top(), 20, rect.height());
        painter->drawText(arrowRect, Qt::AlignVCenter | Qt::AlignRight, ">");
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), 50)); // 确保最小高度为50
        return size;
    }
};

// 自定义对话框实现
ChangelogDialog::ChangelogDialog(QWidget *parent) : QDialog(parent)
{
    // 设置窗口标志，无边框和置顶
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setModal(true);

    
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);
    
    // 创建标题栏
    QWidget *titleBar = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(10, 5, 10, 5);
    
    QLabel *titleLabel = new QLabel("更新日志");
    titleLayout->addWidget(titleLabel);
    
    QPushButton *closeButton = new QPushButton("×");
    closeButton->setFixedSize(30, 30);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    titleLayout->addWidget(closeButton);
    
    mainLayout->addWidget(titleBar);
    
    // 创建内容区域
    m_contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建标签页控件显示不同版本的更新日志
    QTabWidget *tabWidget = new QTabWidget;
    
    
    // v0.1.0 版本更新日志
    QTextBrowser *v010Log = new QTextBrowser;
    v010Log->setOpenExternalLinks(true);
    v010Log->setHtml(
        "<h3 style='color: #2c3e50; margin-top: 10px;'>v1.0.0 (2025-10-26)</h3>"
        "<h4 style='color: #3498db;'>🆕 新增功能</h4>"
        "<ul>"
        "<li>添加问卷基本功能，适配基本的问卷题型</li>"
        "<li>用户登陆功能</li>"
        "<li>适配每页一题</li>"
        "<li>问卷列表界面支持下拉刷新</li>"
        "<li>添加基本的逻辑跳转功能，适配中途结束逻辑</li>"
        "<li>支持选项题中有自定义文本上传</li>"
        "<li>添加用户信息显示页面</li>"
        "<li>添加题干说明显示</li>"
        "<li>支持默认隐藏题目</li>"
        "<li>添加问卷设置页面</li>"
        "<li>支持问卷设置开启自动录音以及照相（可设置时间间隔），提交时将缓存上传</li>"
        "<li>添加应用的定位功能，上传时也会上传位置的经纬度</li>"
        "</ul>"
    );

    // v0.1.1 版本更新日志
    QTextBrowser *v011Log = new QTextBrowser;
    v011Log->setOpenExternalLinks(true);
    v011Log->setHtml(
        "<h3 style='color: #2c3e50; margin-top: 10px;'>v1.0.0 (2025-10-26)</h3>"
        "<h4 style='color: #3498db;'>🆕 新增功能</h4>"
        "<ul>"
        "<li>增加用户操作以及网络请求记录日志功能</li>"
        "</ul>"
        "<h4 style='color: #3498db;'>⛑ 问题修复</h4>"
        "<ul>"
        "<li>修改应用的资源为全局控制，更容易管理</li>"
        "<li>解决切回后台再切回应用时，布局会更改的问题</li>"
        "</ul>"
        );

    tabWidget->addTab(v010Log, "v0.1.0");
    
    contentLayout->addWidget(tabWidget);
    
    mainLayout->addWidget(m_contentWidget);
    
    // 设置合适的大小
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    int width = qMin(600, screenSize.width() - 20);
    int height = qMin(500, screenSize.height() - 20);
    resize(width, height);
    
    // 居中显示
    centerOnScreen();
}

void ChangelogDialog::mousePressEvent(QMouseEvent *event)
{
    // 如果点击的是对话框外部，则关闭对话框
    if (!m_contentWidget->geometry().contains(event->pos())) {
        accept();
    } else {
        QDialog::mousePressEvent(event);
    }
}

void ChangelogDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    // 调整大小时重新居中
    centerOnScreen();
}

void ChangelogDialog::centerOnScreen()
{
    // 获取屏幕大小
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(screenGeometry.topLeft() + QPoint(x, y));
}


SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    
    // 答题设置组
    m_surveySettingsGroup = new QGroupBox("答题设置");
    
    QGridLayout *surveySettingsLayout = new QGridLayout(m_surveySettingsGroup);
    

    // 自动录音选项
    m_autoRecordCheckBox = new QCheckBox("答题时自动录音");
    
    // 自动拍照选项
    m_autoCaptureCheckBox = new QCheckBox("答题时自动拍照");
    
    // 拍照时间间隔设置
    m_captureIntervalLabel = new QLabel("拍照时间间隔（秒）:");
    m_captureIntervalSpinBox = new QSpinBox;
    m_captureIntervalSpinBox->setRange(10, 300); // 10秒到5分钟
    m_captureIntervalSpinBox->setValue(30); // 默认30秒
    m_captureIntervalSpinBox->setSuffix(" 秒");
    
    surveySettingsLayout->addWidget(m_autoRecordCheckBox,0,0,1,3);
    surveySettingsLayout->addWidget(m_autoCaptureCheckBox,1,0,1,3);
    surveySettingsLayout->addWidget(m_captureIntervalLabel,2,0);
    surveySettingsLayout->addWidget(m_captureIntervalSpinBox,2,1,1,2);
    
    m_mainLayout->addWidget(m_surveySettingsGroup);

    // 其他设置组
    m_otherSettingsGroup = new QGroupBox("其他设置");

    QVBoxLayout *otherSettingsLayout = new QVBoxLayout(m_otherSettingsGroup);

    // 使用QListWidget实现其他设置
    m_otherSettingsList = new QListWidget;

    // 设置自定义委托以在右侧绘制>符号
    m_otherSettingsList->setItemDelegate(new ClickableItemDelegate(m_otherSettingsList));

    // 添加"查看更新日志"项到列表
    QListWidgetItem *changelogItem = new QListWidgetItem("查看更新日志");
    changelogItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_otherSettingsList->addItem(changelogItem);

    otherSettingsLayout->addWidget(m_otherSettingsList);
    m_mainLayout->addWidget(m_otherSettingsGroup);
    


    SettingsManager::getInstance().loadFromFile();
    
    // 加载设置
    loadSettings();

    // 连接设置控件的信号到槽函数，实现自动保存
    connect(m_autoRecordCheckBox, &QCheckBox::checkStateChanged, this, &SettingsWidget::onSettingChanged);
    connect(m_autoCaptureCheckBox, &QCheckBox::checkStateChanged, this, &SettingsWidget::onSettingChanged);
    connect(m_captureIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsWidget::onSettingChanged);

    // 连接其他设置列表项的点击信号
    connect(m_otherSettingsList, &QListWidget::itemClicked, this, &SettingsWidget::onOtherSettingsItemClicked);
}

void SettingsWidget::loadSettings()
{
    m_autoRecordCheckBox->setChecked(SettingsManager::getInstance().getValue("survey/autoRecord").toBool());
    m_autoCaptureCheckBox->setChecked(SettingsManager::getInstance().getValue("survey/autoCapture").toBool());
    m_captureIntervalSpinBox->setValue(SettingsManager::getInstance().getValue("survey/captureInterval", 30).toInt());
}

void SettingsWidget::saveSettings()
{
    SettingsManager::getInstance().setValue("survey/autoRecord", m_autoRecordCheckBox->isChecked());
    SettingsManager::getInstance().setValue("survey/autoCapture", m_autoCaptureCheckBox->isChecked());
    SettingsManager::getInstance().setValue("survey/captureInterval", m_captureIntervalSpinBox->value());

    SettingsManager::getInstance().saveToFile();
}

void SettingsWidget::onSettingChanged()
{
    saveSettings();
    
    // 显示保存成功的提示（可以使用 QMessageBox 或其他方式）
    // QMessageBox::information(this, "设置保存", "设置已成功保存！");
    
    // 通知主窗口设置已保存
    // QApplication::processEvents();
}

void SettingsWidget::onBackClicked()
{
    emit backToMain();
}

void SettingsWidget::onShowChangelogClicked()
{
    ChangelogDialog *dialog = createChangelogDialog();
    dialog->exec();
    delete dialog;
}

void SettingsWidget::onOtherSettingsItemClicked(QListWidgetItem *item)
{
    // 检查点击的项是否为"查看更新日志"
    if (item->text() == "查看更新日志") {
        onShowChangelogClicked();
    }
}

ChangelogDialog* SettingsWidget::createChangelogDialog()
{
    ChangelogDialog *dialog = new ChangelogDialog(this);
    return dialog;
}
