#include "mainwindow.h"

#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    qputenv("QT_ANDROID_BLOCK_ON_PAUSE", "1");
    qputenv("QT_ANDROID_ENABLE_WINDOW_FOCUS", "0");

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
