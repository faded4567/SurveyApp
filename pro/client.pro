QT       += core gui network widgets qml core-private svg quickwidgets multimedia location

CONFIG += link_pkgconfig

QTPLUGIN += qtvirtualkeyboardplugin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = SurveyKingClient
TEMPLATE = app




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
    ../src/settingswidget.cpp \
    ../src/permissionmanager.cpp \
    ../src/locationmanager.cpp \
    ../src/globalstyle.cpp

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
    ../inc/settingswidget.h \
    ../inc/permissionmanager.h \
    ../inc/locationmanager.h \
    ../inc/functionlogger.h \
    ../inc/globalstyle.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    ../ui/logindialog.ui

message("QT_ARCH (Target) is: $${QT_ARCH}")
message("QMAKE_HOST.arch (Host) is: $${QMAKE_HOST.arch}")

android{
    ANDROID_PACKAGE_SOURCE_DIR = ../../android
    ANDROID_PERMISSIONS += READ_EXTERNAL_STORAGE
    ANDROID_PERMISSIONS += WRITE_EXTERNAL_STORAGE
    ANDROID_PERMISSIONS += CAMERA
    ANDROID_PERMISSIONS += RECORD_AUDIO
    ANDROID_PERMISSIONS += ACCESS_FINE_LOCATION
    ANDROID_PERMISSIONS += ACCESS_COARSE_LOCATION

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

# macos {
#     # 在 macOS 上启用 OpenSSL 支持
#     DEFINES += USE_OPENSSL
    
#     # 检测 Homebrew 安装的 OpenSSL 路径
#     OPENSSL_PATH = $$system(brew --prefix openssl 2>/dev/null)
#     isEmpty(OPENSSL_PATH) {
#         # 如果 brew 命令失败，尝试默认路径
#         exists(/usr/local/opt/openssl) {
#             OPENSSL_PATH = /usr/local/opt/openssl
#         } else:exists(/opt/homebrew/opt/openssl) {
#             OPENSSL_PATH = /opt/homebrew/opt/openssl
#         }
#     }
    
#     # 设置 OpenSSL 包含路径和库路径
#     INCLUDEPATH += $${OPENSSL_PATH}/include
#     LIBS += -L$${OPENSSL_PATH}/lib -lssl -lcrypto
    
#     # 链接 macOS 系统库
#     LIBS += -framework CoreFoundation -framework Security
# }

ios {
    # iOS平台不启用OpenSSL，因为SurveyEncrypt中的OpenSSL代码不适用于iOS
    # OpenSSL在iOS上的集成比较复杂，需要特殊的构建配置
    
    # 移除OpenSSL相关代码的编译
    DEFINES += USE_OPENSSL
    OPENSSL_PATH = /Volumes/DATA/yangshuai/client_all/ios/libs
    INCLUDEPATH += $${OPENSSL_PATH}/include

    LIBS += -L$${OPENSSL_PATH} -lssl-universal -lcrypto-universal

    contains(QT_ARCH, x86_64) {
        message("We are compiling on an x86_64 ios machine.")
        DESTDIR = ../bin/x86_64/
    }
    contains(QT_ARCH, arm64) {
        message("We are compiling on an ARM64 ios machine.")
        DESTDIR = ../bin/arm64-v8a/
    }

    # 在 .pro 文件中添加图标配置
    ICON = /Volumes/DATA/yangshuai/client_all/ios/res/icon.jpg

    # 或者指定多个图标文件

    CONFIG += add_ios_ffmpeg_libraries
    QMAKE_INFO_PLIST = ../ios/Info.plist


}

RESOURCES += \
    ../res/global.qrc
