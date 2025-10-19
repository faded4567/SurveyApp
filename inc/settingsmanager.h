#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QMutex>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>

class SettingsManager
{
public:
    // 获取单例实例的引用
    static SettingsManager& getInstance();

    // 设置值
    void setValue(const QString &key, const QVariant &value);
    // 获取值，如果key不存在则返回defaultValue
    QVariant getValue(const QString &key, const QVariant &defaultValue = QVariant()) const;
    // 检查是否包含某个key
    bool contains(const QString &key) const;
    // 移除某个key及其值
    void remove(const QString &key);
    // 获取所有键名
    QStringList allKeys() const;

    // 保存到文件（可选）
    void saveToFile() const;
    // 从文件加载（可选）
    void loadFromFile();

    // 禁止拷贝和赋值
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

private:
    // 私有构造函数和析构函数
    SettingsManager();
    ~SettingsManager();

    // 用于存储设置的容器
    QMap<QString, QVariant> m_settings;
    QString m_savePath;
    // 用于线程安全的互斥锁
    mutable QMutex m_mutex;
};

#endif // SETTINGSMANAGER_H
