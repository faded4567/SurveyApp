#ifndef LOGFILEMANAGER_H
#define LOGFILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMutex>

class LogFileManager : public QObject
{
    Q_OBJECT

public:
    static LogFileManager& instance();
    
    // 删除拷贝构造函数和赋值操作符，确保单例
    LogFileManager(const LogFileManager&) = delete;
    LogFileManager& operator=(const LogFileManager&) = delete;
    
    // 初始化日志文件
    void initialize();
    
    // 记录用户操作
    void logUserAction(const QString& action, const QString& details = "");
    
    // 记录网络请求
    void logNetworkRequest(const QString& endpoint, const QJsonObject& requestData);
    
    // 记录网络响应
    void logNetworkResponse(const QString& endpoint, int statusCode, const QJsonObject& responseData);
    
    // 记录错误信息
    void logError(const QString& errorType, const QString& errorMessage, const QString& details = "");
    
    // 记录应用启动
    void logApplicationStart();
    
    // 记录应用关闭
    void logApplicationClose();

    // 记录函数进入和退出
    void logFunctionEnter(const QString& functionName, const QString& fileName, int lineNumber, const QString& details = "");
    void logFunctionExit(const QString& functionName, const QString& fileName, int lineNumber);

private:
    explicit LogFileManager(QObject *parent = nullptr);
    ~LogFileManager();
    
    // 写入日志条目
    void writeLogEntry(const QString& type, const QString& message);
    
    // 获取当前时间戳
    QString getCurrentTimestamp() const;
    
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    bool m_initialized;
};

#endif // LOGFILEMANAGER_H
