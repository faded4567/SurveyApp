QT       += core gui network widgets qml core-private svg quickwidgets multimedia

CONFIG += link_pkgconfig

QTPLUGIN += qtvirtualkeyboardplugin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = SurveyKingClient
TEMPLATE = app

ANDROID_PACKAGE_SOURCE_DIR = ../android

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH +=  ../inc

SOURCES += \
    ../src/CustomUI.cpp \
    ../src/logfilemanager.cpp \
    ../src/logindialog.cpp \
    ../src/main.cpp \
    ../src/mainwindow.cpp \
    ../src/networkmanager.cpp \
    ../src/settingsmanager.cpp \
    ../src/surveyencrypt.cpp \
    ../src/surveyformwidget.cpp\
    ../src/dashboardwidget.cpp \
    ../src/settingswidget.cpp

HEADERS += \
    ../inc/CustomUI.h \
    ../inc/logfilemanager.h \
    ../inc/logindialog.h \
    ../inc/mainwindow.h \
    ../inc/networkmanager.h \
    ../inc/settingsmanager.h \
    ../inc/surveyencrypt.h \
    ../inc/surveyformwidget.h \
    ../inc/dashboardwidget.h \
    ../inc/settingswidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ../ui/logindialog.ui

message("QT_ARCH (Target) is: $${QT_ARCH}")
message("QMAKE_HOST.arch (Host) is: $${QMAKE_HOST.arch}")

android{
    ANDROID_PERMISSIONS += READ_EXTERNAL_STORAGE
    ANDROID_PERMISSIONS += WRITE_EXTERNAL_STORAGE
    # 定义宏
    DEFINES += USE_OPENSSL

    # 如果需要指定OpenSSL路径
    OPENSSL_PATH = /home/ubuntu/Android/Sdk/android_openssl-master
    include(/home/ubuntu/Android/Sdk/android_openssl-master/openssl.pri)
    INCLUDEPATH += $${OPENSSL_PATH}/ssl_3/include

    contains(QT_ARCH, x86_64) {
        message("We are compiling on an x86_64 host machine.")
        LIBS += $${OPENSSL_PATH}/ssl_3/x86_64/libcrypto_3.so \
                $${OPENSSL_PATH}/ssl_3/x86_64/libssl_3.so
        DESTDIR = ../bin/x86_64/
    }
    contains(QT_ARCH, arm64-v8a) {
        message("We are compiling on an ARM64 host machine.")
        LIBS += $${OPENSSL_PATH}/ssl_3/arm64-v8a/libcrypto_3.so \
                $${OPENSSL_PATH}/ssl_3/arm64-v8a/libssl_3.so
        DESTDIR = ../bin/arm64-v8a/
    }
    contains(QT_ARCH, armeabi-v7a) {
        message("We are compiling on an ARM32 host machine.")
        LIBS += $${OPENSSL_PATH}/ssl_3/armeabi-v7a/libcrypto_3.so \
                $${OPENSSL_PATH}/ssl_3/armeabi-v7a/libssl_3.so
        DESTDIR = ../bin/armeabi-v7a/
    }
}
