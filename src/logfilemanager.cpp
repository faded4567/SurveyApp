#include "logfilemanager.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>

LogFileManager::LogFileManager(QObject *parent) : QObject(parent),
    m_logFile(nullptr),
    m_textStream(nullptr),
    m_maxSize(2 * 1024 * 1024), // 默认2MB
    m_maxDays(7) // 默认保留7天
{
}

LogFileManager::~LogFileManager()
{
    if (m_textStream) {
        delete m_textStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

void LogFileManager::init()
{
    // 初始化日志目录和文件
    initLogFile();
    // 清理过期日志
    cleanupOldLogs();

    // 安装消息处理器，捕获Qt的调试信息
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        Q_UNUSED(context);

        QString level;
        switch (type) {
        case QtDebugMsg:
            level = "DEBUG";
            break;
        case QtInfoMsg:
            level = "INFO";
            break;
        case QtWarningMsg:
            level = "WARN";
            break;
        case QtCriticalMsg:
            level = "ERROR";
            break;
        case QtFatalMsg:
            level = "FATAL";
            break;
        }

        QString logMessage = QString("[%1] [%2] %3").arg(
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),
            level,
            msg);

        LogFileManager::instance().writeLog(logMessage);
    });
}
LogFileManager& LogFileManager::instance()
{
    static LogFileManager instance;
    return instance;
}
void LogFileManager::initLogFile()
{
    // 获取应用数据目录[7](@ref)
    QString logDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir logDir(logDirPath);
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    // 创建以当前日期命名的日志文件
    QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    m_logFileName = logDirPath + "/applog_" + dateStr + ".txt";

    m_logFile = new QFile(m_logFileName);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << m_logFile->errorString();
        delete m_logFile;
        m_logFile = nullptr;
        return;
    }

    m_textStream = new QTextStream(m_logFile);
    *m_textStream << "=============== Application Started ===============\n";
    *m_textStream << "Start Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *m_textStream << "==================================================\n";
    m_textStream->flush();
}

void LogFileManager::writeLog(const QString &log)
{
    if (!m_textStream || !m_logFile) {
        return;
    }

    // 检查日志文件大小，必要时轮转[8](@ref)
    checkLogSize();

    *m_textStream << log << "\n";
    m_textStream->flush();
}

void LogFileManager::checkLogSize()
{
    if (m_logFile->size() > m_maxSize) {
        // 关闭当前文件
        m_textStream->flush();
        m_logFile->close();
        delete m_textStream;
        delete m_logFile;

        // 创建新的日志文件（带时间戳）
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString newFileName = m_logFileName + "." + timestamp + ".bak";

        // 重命名当前文件
        QFile::rename(m_logFileName, newFileName);

        // 重新初始化日志文件
        initLogFile();
    }
}

void LogFileManager::cleanupOldLogs()
{
    QString logDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir logDir(logDirPath);

    QDateTime cutoffDate = QDateTime::currentDateTime().addDays(-m_maxDays);

    QFileInfoList logFiles = logDir.entryInfoList(QStringList() << "applog_*.txt" << "*.bak", QDir::Files);
    for (const QFileInfo &fileInfo : logFiles) {
        if (fileInfo.lastModified() < cutoffDate) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

void LogFileManager::setMaxSize(quint64 maxSize)
{
    m_maxSize = maxSize;
}

void LogFileManager::setMaxDays(int maxDays)
{
    m_maxDays = maxDays;
}
