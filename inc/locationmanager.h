#ifndef LOCATIONMANAGER_H
#define LOCATIONMANAGER_H

#include <QObject>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>

class LocationManager : public QObject
{
    Q_OBJECT

public:
    struct LocationInfo {
        double latitude = 0.0;      // 纬度
        double longitude = 0.0;     // 经度
        double altitude = 0.0;      // 海拔
        qreal accuracy = 0.0;       // 精度
        qreal speed = 0.0;          // 速度
        bool isValid = false;       // 位置信息是否有效
        qint64 timestamp = 0;       // 时间戳
    };

    static LocationManager& instance();
    
    // 初始化定位服务
    bool initialize();
    
    // 请求当前位置
    void requestCurrentLocation();
    
    // 开始连续定位
    void startContinuousLocationUpdates(int interval = 10000); // 默认10秒
    
    // 停止连续定位
    void stopContinuousLocationUpdates();
    
    // 获取最后一次已知位置
    LocationInfo getLastKnownLocation() const;
    
    // 检查定位服务是否可用
    bool isLocationServiceAvailable() const;

signals:
    // 当获取到新位置时发出此信号
    void locationUpdated(const LocationManager::LocationInfo& location);
    
    // 当定位服务出错时发出此信号
    void locationError(const QString& errorString);

private:
    explicit LocationManager(QObject *parent = nullptr);
    void onPositionUpdated(const QGeoPositionInfo &info);
    void onPositionError(QGeoPositionInfoSource::Error error);

private:
    QGeoPositionInfoSource *m_positionSource = nullptr;
    LocationInfo m_lastKnownLocation;
    bool m_isContinuousUpdating = false;
};

#endif // LOCATIONMANAGER_H