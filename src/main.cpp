#include "mainwindow.h"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

// Qt 6 中隐藏启动画面的函数
void hideSplashScreen(int fadeDuration = 0)
{
#ifdef Q_OS_ANDROID
    // 使用 QJniObject 调用静态方法
    QJniObject::callStaticMethod<void>(
        "org/qtproject/qt/android/QtNative",
        "hideSplashScreen",
        "(I)V",
        fadeDuration
        );

    qDebug() << "启动画面隐藏调用完成，淡出时长:" << fadeDuration << "ms";
#elif defined(Q_OS_IOS)
    // iOS平台不需要特殊处理，应用启动后会自动隐藏启动画面
    qDebug() << "iOS平台，启动画面会自动隐藏";
#else
    qDebug() << "非移动平台，忽略启动画面隐藏";
#endif
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    qputenv("QT_ANDROID_BLOCK_ON_PAUSE", "1");
    qputenv("QT_ANDROID_ENABLE_WINDOW_FOCUS", "0");
#endif

    QApplication a(argc, argv);

    // 从文件加载样式表
    QFile styleFile(":/styles/global.qss");  // 使用资源文件路径

    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);  // 设置全局样式
        styleFile.close();
    }

    // 延迟2秒隐藏启动画面
    QTimer::singleShot(3000, []() {
        hideSplashScreen();
    });

    MainWindow w;
    w.show();
    return a.exec();
}
