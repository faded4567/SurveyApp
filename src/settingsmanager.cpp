#include "settingsmanager.h"
#include <QDebug>

#define SETTTING_FILE QString("/setting.ini")

SettingsManager& SettingsManager::getInstance()
{
    static SettingsManager instance; // C++11保证静态局部变量初始化线程安全
    return instance;
}

SettingsManager::SettingsManager()
{
    // 初始化代码（如果需要）
    qDebug() << "SettingsManager initialized";
    m_savePath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + SETTTING_FILE;
}

SettingsManager::~SettingsManager()
{
    // 清理代码（如果需要）
    qDebug() << "SettingsManager destroyed";
}

void SettingsManager::setValue(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&m_mutex);
    m_settings[key] = value;
}

QVariant SettingsManager::getValue(const QString &key, const QVariant &defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings.value(key, defaultValue);
}

bool SettingsManager::contains(const QString &key) const
{
    QMutexLocker locker(&m_mutex);
    return m_settings.contains(key);
}

void SettingsManager::remove(const QString &key)
{
    QMutexLocker locker(&m_mutex);
    m_settings.remove(key);
}

QStringList SettingsManager::allKeys() const
{
    QMutexLocker locker(&m_mutex);
    return m_settings.keys();
}

void SettingsManager::saveToFile() const
{
    QMutexLocker locker(&m_mutex);

    QSettings settings(m_savePath, QSettings::IniFormat);
    for (auto it = m_settings.constBegin(); it != m_settings.constEnd(); ++it) {
        settings.setValue(it.key(), it.value());
    }
}

void SettingsManager::loadFromFile()
{
    QMutexLocker locker(&m_mutex);
    QSettings settings(m_savePath, QSettings::IniFormat);
    QStringList keys = settings.allKeys();
    for (const QString &key : keys) {
        m_settings[key] = settings.value(key);
    }
}
