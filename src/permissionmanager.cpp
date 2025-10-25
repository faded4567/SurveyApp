#include "permissionmanager.h"
#include <QApplication>
#include <QDebug>
#include <QMediaDevices>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

// 单例实例定义
PermissionManager& PermissionManager::instance()
{
    static PermissionManager instance;
    return instance;
}

PermissionManager::PermissionManager(QObject *parent)
    : QObject(parent)
{
}

void PermissionManager::requestAudioRecordingPermission(std::function<void(bool)> callback)
{
    m_audioCallback = callback;

#ifdef Q_OS_ANDROID
    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Undetermined:
        // 请求权限
        qApp->requestPermission(microphonePermission, this, [this](const QPermission &permission) {
            handleAudioPermissionResult(permission);
        });
        break;
    case Qt::PermissionStatus::Denied:
        // 权限被拒绝
        if (m_audioCallback) {
            m_audioCallback(false);
        }
        emit audioPermissionDenied();
        break;
    case Qt::PermissionStatus::Granted:
        // 权限已授予
        if (m_audioCallback) {
            m_audioCallback(true);
        }
        emit audioPermissionGranted();
        break;
    }
#else
    // 非Android平台，默认授予权限
    if (m_audioCallback) {
        m_audioCallback(true);
    }
    emit audioPermissionGranted();
#endif
}

void PermissionManager::requestCameraPermission(std::function<void(bool)> callback)
{
    m_cameraCallback = callback;

#ifdef Q_OS_ANDROID
    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
    case Qt::PermissionStatus::Undetermined:
        // 请求权限
        qApp->requestPermission(cameraPermission, this, [this](const QPermission &permission) {
            handleCameraPermissionResult(permission);
        });
        break;
    case Qt::PermissionStatus::Denied:
        // 权限被拒绝
        if (m_cameraCallback) {
            m_cameraCallback(false);
        }
        emit cameraPermissionDenied();
        break;
    case Qt::PermissionStatus::Granted:
        // 权限已授予
        if (m_cameraCallback) {
            m_cameraCallback(true);
        }
        emit cameraPermissionGranted();
        break;
    }
#else
    // 非Android平台，默认授予权限
    if (m_cameraCallback) {
        m_cameraCallback(true);
    }
    emit cameraPermissionGranted();
#endif
}

void PermissionManager::requestLocationPermission(std::function<void(bool)> callback)
{
    m_locationCallback = callback;

#ifdef Q_OS_ANDROID
    QLocationPermission locationPermission;
    switch (qApp->checkPermission(locationPermission)) {
    case Qt::PermissionStatus::Undetermined:
        // 请求权限
        qApp->requestPermission(locationPermission, this, [this](const QPermission &permission) {
            handleLocationPermissionResult(permission);
        });
        break;
    case Qt::PermissionStatus::Denied:
        // 权限被拒绝
        if (m_locationCallback) {
            m_locationCallback(false);
        }
        emit locationPermissionDenied();
        break;
    case Qt::PermissionStatus::Granted:
        // 权限已授予
        if (m_locationCallback) {
            m_locationCallback(true);
        }
        emit locationPermissionGranted();
        break;
    }
#else
    // 非Android平台，默认授予权限
    if (m_locationCallback) {
        m_locationCallback(true);
    }
    emit locationPermissionGranted();
#endif
}

bool PermissionManager::hasAudioRecordingPermission() const
{
#ifdef Q_OS_ANDROID
    QMicrophonePermission microphonePermission;
    return (qApp->checkPermission(microphonePermission) == Qt::PermissionStatus::Granted);
#else
    return true;
#endif
}

bool PermissionManager::hasCameraPermission() const
{
#ifdef Q_OS_ANDROID
    QCameraPermission cameraPermission;
    return (qApp->checkPermission(cameraPermission) == Qt::PermissionStatus::Granted);
#else
    return true;
#endif
}

bool PermissionManager::hasLocationPermission() const
{
#ifdef Q_OS_ANDROID
    QLocationPermission locationPermission;
    return (qApp->checkPermission(locationPermission) == Qt::PermissionStatus::Granted);
#else
    return true;
#endif
}

#ifdef Q_OS_ANDROID
void PermissionManager::handleAudioPermissionResult(const QPermission &permission)
{
    if (permission.status() == Qt::PermissionStatus::Granted) {
        qDebug() << "Audio recording permission granted";
        if (m_audioCallback) {
            m_audioCallback(true);
        }
        emit audioPermissionGranted();
    } else {
        qDebug() << "Audio recording permission denied";
        if (m_audioCallback) {
            m_audioCallback(false);
        }
        emit audioPermissionDenied();
    }
}

void PermissionManager::handleCameraPermissionResult(const QPermission &permission)
{
    if (permission.status() == Qt::PermissionStatus::Granted) {
        qDebug() << "Camera permission granted";
        if (m_cameraCallback) {
            m_cameraCallback(true);
        }
        emit cameraPermissionGranted();
    } else {
        qDebug() << "Camera permission denied";
        if (m_cameraCallback) {
            m_cameraCallback(false);
        }
        emit cameraPermissionDenied();
    }
}

void PermissionManager::handleLocationPermissionResult(const QPermission &permission)
{
    if (permission.status() == Qt::PermissionStatus::Granted) {
        qDebug() << "Location permission granted";
        if (m_locationCallback) {
            m_locationCallback(true);
        }
        emit locationPermissionGranted();
    } else {
        qDebug() << "Location permission denied";
        if (m_locationCallback) {
            m_locationCallback(false);
        }
        emit locationPermissionDenied();
    }
}
#endif