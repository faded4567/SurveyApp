#include "globalstyle.h"
#include <QWidget>
#include <QFile>
#include <QApplication>
#include <QDir>

QString GlobalStyle::getStyleSheet()
{
    return loadStyleSheet(":/styles/global.qss");
}

QString GlobalStyle::loadStyleSheet(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        // 文件无法打开，返回空字符串
        return QString();
    }
    
    QTextStream stream(&file);
    QString styleSheet = stream.readAll();
    file.close();
    
    return styleSheet;
}

void GlobalStyle::applyToWidget(QWidget* widget)
{
    if (widget) {
        widget->setStyleSheet(getStyleSheet());
    }
}