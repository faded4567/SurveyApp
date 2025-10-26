#include "locationmanager.h"
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
#include <QDateTime>
#include <QDebug>

LocationManager::LocationManager(QObject *parent)
    : QObject(parent)
    , m_positionSource(nullptr)
    , m_isContinuousUpdating(false)
{
}

LocationManager& LocationManager::instance()
{
    static LocationManager instance;
    return instance;
}

bool LocationManager::initialize()
{
    if (m_positionSource) {
        return true; // 已经初始化
    }

    // 创建位置信息源
    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    
    if (!m_positionSource) {
        emit locationError("无法创建位置信息源");
        return false;
    }

    // 连接信号和槽
    connect(m_positionSource, &QGeoPositionInfoSource::positionUpdated,
            this, &LocationManager::onPositionUpdated);
    connect(m_positionSource, &QGeoPositionInfoSource::errorOccurred,
            this, &LocationManager::onPositionError);

    return true;
}

void LocationManager::requestCurrentLocation()
{
    if (!m_positionSource) {
        if (!initialize()) {
            return;
        }
    }

    // 请求当前位置（超时时间30秒）
    m_positionSource->requestUpdate(30000);
}

void LocationManager::startContinuousLocationUpdates(int interval)
{
    if (!m_positionSource) {
        if (!initialize()) {
            return;
        }
    }

    if (!m_isContinuousUpdating) {
        m_positionSource->setUpdateInterval(interval);
        m_positionSource->startUpdates();
        m_isContinuousUpdating = true;
    }
}

void LocationManager::stopContinuousLocationUpdates()
{
    if (m_positionSource && m_isContinuousUpdating) {
        m_positionSource->stopUpdates();
        m_isContinuousUpdating = false;
    }
}

LocationManager::LocationInfo LocationManager::getLastKnownLocation() const
{
    return m_lastKnownLocation;
}

bool LocationManager::isLocationServiceAvailable() const
{
    return !QGeoPositionInfoSource::availableSources().isEmpty();
}

void LocationManager::onPositionUpdated(const QGeoPositionInfo &info)
{
    if (!info.isValid()) {
        emit locationError("接收到无效的位置信息");
        return;
    }

    LocationInfo locationInfo;
    locationInfo.isValid = true;
    locationInfo.timestamp = info.timestamp().toMSecsSinceEpoch();

    QGeoCoordinate coordinate = info.coordinate();
    if (coordinate.isValid()) {
        locationInfo.latitude = coordinate.latitude();
        locationInfo.longitude = coordinate.longitude();

        // 正确获取海拔信息的方法
        if (coordinate.type() == QGeoCoordinate::Coordinate3D) {
            locationInfo.altitude = coordinate.altitude();
        }
    }

    if (info.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        locationInfo.accuracy = info.attribute(QGeoPositionInfo::HorizontalAccuracy);
    }

    if (info.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        locationInfo.speed = info.attribute(QGeoPositionInfo::GroundSpeed);
    }

    m_lastKnownLocation = locationInfo;

    emit locationUpdated(locationInfo);
}

void LocationManager::onPositionError(QGeoPositionInfoSource::Error error)
{
    QString errorString;
    switch (error) {
    case QGeoPositionInfoSource::AccessError:
        errorString = "访问定位服务被拒绝";
        break;
    case QGeoPositionInfoSource::ClosedError:
        errorString = "定位服务关闭";
        break;
    case QGeoPositionInfoSource::UnknownSourceError:
        errorString = "未知的定位源错误";
        break;
    case QGeoPositionInfoSource::NoError:
        return; // 没有错误
    }

    emit locationError(errorString);
}
