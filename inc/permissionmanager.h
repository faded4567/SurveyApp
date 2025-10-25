#ifndef PERMISSIONMANAGER_H
#define PERMISSIONMANAGER_H

#include <QObject>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QScopedPointer>

#ifdef Q_OS_ANDROID
#include <QCoreApplication>
#include <QJniObject>
#include <QPermissions>
#endif

class PermissionManager : public QObject
{
    Q_OBJECT

public:
    // 获取单例实例
    static PermissionManager& instance();

    // 删除拷贝构造函数和赋值操作符，确保单例唯一性
    PermissionManager(const PermissionManager &) = delete;
    PermissionManager& operator=(const PermissionManager &) = delete;

    // 请求录音权限
    void requestAudioRecordingPermission(std::function<void(bool)> callback);

    // 请求相机权限
    void requestCameraPermission(std::function<void(bool)> callback);
    
    // 请求定位权限
    void requestLocationPermission(std::function<void(bool)> callback);

    // 检查是否已获得录音权限
    bool hasAudioRecordingPermission() const;

    // 检查是否已获得相机权限
    bool hasCameraPermission() const;
    
    // 检查是否已获得定位权限
    bool hasLocationPermission() const;

signals:
    void audioPermissionGranted();
    void audioPermissionDenied();
    void cameraPermissionGranted();
    void cameraPermissionDenied();
    void locationPermissionGranted();
    void locationPermissionDenied();

private:
    explicit PermissionManager(QObject *parent = nullptr);

#ifdef Q_OS_ANDROID
    void handleAudioPermissionResult(const QPermission &permission);
    void handleCameraPermissionResult(const QPermission &permission);
    void handleLocationPermissionResult(const QPermission &permission);
#endif

    std::function<void(bool)> m_audioCallback;
    std::function<void(bool)> m_cameraCallback;
    std::function<void(bool)> m_locationCallback;
};

#endif // PERMISSIONMANAGER_H