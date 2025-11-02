#include "mainwindow.h"

#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    qputenv("QT_ANDROID_BLOCK_ON_PAUSE", "1");
    qputenv("QT_ANDROID_ENABLE_WINDOW_FOCUS", "0");

    QApplication a(argc, argv);

    // 从文件加载样式表
    QFile styleFile(":/styles/global.qss");  // 使用资源文件路径

    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);  // 设置全局样式
        styleFile.close();
    }

    MainWindow w;
    w.show();
    return a.exec();
}
