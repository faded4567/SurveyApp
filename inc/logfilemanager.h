#ifndef LOGFILEMANAGER_H
#define LOGFILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>

class LogFileManager : public QObject
{
    Q_OBJECT
public:
    static LogFileManager& instance();

    // 初始化日志系统
    void init();
    // 设置日志文件最大大小（字节）
    void setMaxSize(quint64 maxSize);
    // 设置日志文件最长保留天数
    void setMaxDays(int maxDays);

public slots:
    // 写入日志
    void writeLog(const QString &log);

private:
    explicit LogFileManager(QObject *parent = nullptr);
    ~LogFileManager();

    QFile *m_logFile;
    QTextStream *m_textStream;
    QString m_logFileName;
    quint64 m_maxSize; // 单个日志文件最大大小
    int m_maxDays; // 日志文件最长保留天数

    // 初始化日志文件
    void initLogFile();
    // 检查并处理日志文件大小
    void checkLogSize();
    // 清理过期日志文件
    void cleanupOldLogs();
};

#endif // LOGFILEMANAGER_H
