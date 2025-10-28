#include "logfilemanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include <QJsonDocument>

LogFileManager& LogFileManager::instance()
{
    static LogFileManager instance;
    return instance;
}

LogFileManager::LogFileManager(QObject *parent)
    : QObject(parent)
    , m_initialized(false)
{
}

LogFileManager::~LogFileManager()
{
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void LogFileManager::initialize()
{
    if (m_initialized) {
        return;
    }
    
    // 获取应用数据目录
    QString logDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir logDir(logDirPath);
    
    // 如果目录不存在则创建
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    
    // 创建日志文件名（按日期）
    QString logFileName = "surveyking_" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
    QString logFilePath = logDirPath + "/" + logFileName;
    
    m_logFile.setFileName(logFilePath);
    
    // 以追加模式打开文件
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
        m_initialized = true;
        
        logApplicationStart();
    } else {
        qDebug() << "Failed to open log file:" << logFilePath;
    }
}

void LogFileManager::logUserAction(const QString& action, const QString& details)
{
    if (!m_initialized) return;
    
    QString message = QString("User Action: %1").arg(action);
    if (!details.isEmpty()) {
        message += QString(" | Details: %1").arg(details);
    }
    
    writeLogEntry("ACTION", message);
}

void LogFileManager::logNetworkRequest(const QString& endpoint, const QJsonObject& requestData)
{
    if (!m_initialized) return;
    
    QString message = QString("Network Request: %1").arg(endpoint);
    
    if (!requestData.isEmpty()) {
        QJsonDocument doc(requestData);
        message += QString(" | Request Data: %1").arg(doc.toJson(QJsonDocument::Compact).constData());
    }
    
    writeLogEntry("REQUEST", message);
}

void LogFileManager::logNetworkResponse(const QString& endpoint, int statusCode, const QJsonObject& responseData)
{
    if (!m_initialized) return;
    
    QString message = QString("Network Response: %1 | Status Code: %2").arg(endpoint).arg(statusCode);
    
    if (!responseData.isEmpty()) {
        QJsonDocument doc(responseData);
        message += QString(" | Response Data: %1").arg(doc.toJson(QJsonDocument::Compact).constData());
    }
    
    writeLogEntry("RESPONSE", message);
}

void LogFileManager::logError(const QString& errorType, const QString& errorMessage, const QString& details)
{
    if (!m_initialized) return;
    
    QString message = QString("Error: %1 | Message: %2").arg(errorType, errorMessage);
    if (!details.isEmpty()) {
        message += QString(" | Details: %1").arg(details);
    }
    
    writeLogEntry("ERROR", message);
}

void LogFileManager::logApplicationStart()
{
    if (!m_initialized) return;
    
    QString message = QString("Application Started | Version: %1 | Platform: %2")
                         .arg(QApplication::applicationVersion())
                         .arg(QApplication::platformName());
    
    writeLogEntry("INFO", message);
}

void LogFileManager::logApplicationClose()
{
    if (!m_initialized) return;
    
    QString message = "Application Closed";
    writeLogEntry("INFO", message);
    
    m_logStream.flush();
}

void LogFileManager::logFunctionEnter(const QString &functionName, const QString &fileName, int lineNumber, const QString &details)
{
    if (!m_initialized) return;

    QString message = QString("Function Enter: %1 (File: %2, Line: %3, Thread: %4)")
                          .arg(functionName)
                          .arg(fileName)
                          .arg(lineNumber)
                          .arg(QString::number((quintptr)QThread::currentThreadId(), 16));

    if (!details.isEmpty()) {
        message += QString(" | Details: %1").arg(details);
    }

    writeLogEntry("FUNC_ENTER", message);
}

void LogFileManager::logFunctionExit(const QString &functionName, const QString &fileName, int lineNumber)
{
    if (!m_initialized) return;

    QString message = QString("Function Exit: %1 (File: %2, Line: %3, Thread: %4)")
                          .arg(functionName)
                          .arg(fileName)
                          .arg(lineNumber)
                          .arg(QString::number((quintptr)QThread::currentThreadId(), 16));

    writeLogEntry("FUNC_EXIT", message);
}

void LogFileManager::writeLogEntry(const QString& type, const QString& message)
{
    if (!m_initialized) return;
    
    QMutexLocker locker(&m_mutex);
    
    QString logEntry = QString("[%1] [%2] %3\n")
                          .arg(getCurrentTimestamp())
                          .arg(type)
                          .arg(message);
    
    m_logStream << logEntry;
    m_logStream.flush();
}

QString LogFileManager::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}
