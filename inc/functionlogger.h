#ifndef FUNCTIONLOGGER_H
#define FUNCTIONLOGGER_H

#include "logfilemanager.h"
#include <QString>
#include <QThread>
#include <QFileInfo>

// 定义一个用于跟踪函数执行的辅助类
class FunctionLogger
{
public:
    FunctionLogger(const QString& functionName, const QString& fileName, int lineNumber, const QString& details = "")
        : m_functionName(functionName), m_lineNumber(lineNumber)
    {
        m_fileName = QFileInfo(fileName).fileName();
        LogFileManager::instance().logFunctionEnter(functionName, m_fileName, lineNumber, details);
    }

    ~FunctionLogger()
    {
        LogFileManager::instance().logFunctionExit(m_functionName, m_fileName, m_lineNumber);
    }

private:
    QString m_functionName;
    QString m_fileName;
    int m_lineNumber;
};

// 定义宏，用于在函数开始处自动记录函数进入和退出
#define FUNCTION_LOG() \
FunctionLogger _functionLogger_##__LINE__(__FUNCTION__, __FILE__, __LINE__)

#define FUNCTION_LOG_DETAIL(details) \
    FunctionLogger _functionLogger_##__LINE__(__FUNCTION__, __FILE__, __LINE__, details)

// 如果只想在调试模式下记录函数调用，可以使用以下宏
#ifdef QT_DEBUG
#define DEBUG_FUNCTION_LOG() \
    FunctionLogger _functionLogger_##__LINE__(__FUNCTION__, __FILE__, __LINE__)

#define DEBUG_FUNCTION_LOG_DETAIL(details) \
    FunctionLogger _functionLogger_##__LINE__(__FUNCTION__, __FILE__, __LINE__, details)
#else
#define DEBUG_FUNCTION_LOG() ((void)0)
#define DEBUG_FUNCTION_LOG_DETAIL(details) ((void)0)
#endif

#endif // FUNCTIONLOGGER_H
