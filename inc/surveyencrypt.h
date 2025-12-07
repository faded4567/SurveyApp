#ifndef SURVEYENCRYPT_H
#define SURVEYENCRYPT_H

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QByteArray>
#include <QFile>
#include <QDebug>

// 注意：QT 原生不支持 RSA-OAEP，需要使用第三方库如 OpenSSL 或 Botan
// 这里提供一个基于 OpenSSL 的实现示例

#ifdef USE_OPENSSL
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#endif

#if defined(Q_OS_IOS)
#include <QLibrary>
#endif

class SurveyEncrypt {
public:
    /**
     * @brief 将undefined值转换为空字符串
     * @param data 输入的JSON对象
     * @return 处理后的JSON对象
     */
    static QJsonObject undefinedToString(const QJsonObject& data) {
        QJsonObject result;
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (it.value().isUndefined()) {
                result[it.key()] = QString("");
            } else {
                result[it.key()] = it.value();
            }
        }
        return result;
    }

    static QString jsonValueToString(const QJsonValue& value) {
        if (value.isObject()) {
            QJsonDocument doc(value.toObject());
            return doc.toJson(QJsonDocument::Compact);
        } else if (value.isArray()) {
            QJsonDocument doc(value.toArray());
            return doc.toJson(QJsonDocument::Compact);
        } else if (value.isString()) {
            return "\"" + value.toString() + "\"";
        } else if (value.isDouble()) {
            double num = value.toDouble();
            if (num == (int)num) {
                return QString::number((int)num);
            } else {
                return QString::number(num, 'g', 15);
            }
        } else if (value.isBool()) {
            return value.toBool() ? "true" : "false";
        } else if (value.isNull()) {
            return "null";
        } else {
            // 使用QJsonDocument处理其他情况
            QJsonDocument doc(QJsonObject{{"temp", value}});
            QString result = doc.toJson(QJsonDocument::Compact);
            // 提取值部分: {"temp":value} -> value
            result = result.mid(8, result.length() - 9);
            return result;
        }
    }

    /**
     * @brief 根据数据生成签名
     * @param sourceData 源数据
     * @return 生成的签名字符串 (格式: sha256_hash.timestamp)
     */
    static QString getSignByData(const QJsonObject& sourceData) {
        // 处理undefined值
        QJsonObject data = undefinedToString(sourceData);

        // 获取所有键并排序
        QStringList keys;
        for (auto it = data.begin(); it != data.end(); ++it) {
            keys.append(it.key());
        }
        std::sort(keys.begin(), keys.end());

        // 构建签名数组
        QStringList signArr;
        for (const QString& key : keys) {
            QJsonValue value = data[key];
            QString signPart;

            if (value.isString()) {
                // 对字符串进行URL编码
                QString encodedValue = QString::fromUtf8(
                    QUrl::toPercentEncoding(value.toString())
                    );
                signPart = QString("%1=%2").arg(key).arg(encodedValue);
            } else {
                // 对非字符串值进行JSON序列化，与前端的JSON.stringify保持一致
                QString jsonStr = jsonValueToString(value);
                signPart = QString("%1=%2").arg(key).arg(jsonStr);
            }

            signArr.append(signPart);
        }

        // 获取当前时间戳
        qint64 ts = QDateTime::currentDateTime().toMSecsSinceEpoch();

        // 连接所有签名部分并添加时间戳
        QString signString = signArr.join("") + QString::number(ts);

        // 计算SHA256哈希
        QByteArray hash = QCryptographicHash::hash(
            signString.toUtf8(),
            QCryptographicHash::Sha256
            );

        // 转换为十六进制字符串
        QString sign = hash.toHex();

        // 返回格式: hash.timestamp
        return QString("%1.%2").arg(sign).arg(ts);
    }

    /**
     * @brief 为请求数据添加签名
     * @param requestData 请求数据
     * @return 添加签名后的数据
     */
    static QString addSignToRequest(QJsonObject requestData) {
        return getSignByData(requestData);
    }

    /**
     * @brief RSA 加密实现（模拟前端 encrypt.js 的 rsa 函数）
     * @param data 需要加密的数据
     * @param publicKey PEM 格式的公钥
     * @return 加密后的数据列表（Base64 编码）
     */
    static QStringList rsaEncrypt(const QString& data, const QString& publicKey) {
        QStringList result;

#ifdef USE_OPENSSL
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS)
        // 对数据进行 URL 编码，模拟前端 encodeURIComponent
        QString encodedData = QString::fromUtf8(QUrl::toPercentEncoding(data));
        QByteArray originData = encodedData.toUtf8();

        // 检查公钥是否为 PEM 格式，如果不是则转换
        QByteArray publicKeyData = publicKey.toUtf8();
        if (!publicKeyData.startsWith("-----BEGIN PUBLIC KEY-----")) {
            // 将Base64编码的PKCS#8公钥转换为PEM格式
            publicKeyData = "-----BEGIN PUBLIC KEY-----\n" + publicKeyData + "\n-----END PUBLIC KEY-----";
        }

        // 将 PEM 格式的公钥转换为 OpenSSL RSA 对象
        BIO* bio = BIO_new_mem_buf(publicKeyData.constData(), -1);
        RSA* rsaKey = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);

        if (!rsaKey) {
            qWarning() << "Failed to load public key";
            unsigned long err = ERR_get_error();
            char errBuf[256];
            ERR_error_string_n(err, errBuf, sizeof(errBuf));
            qWarning() << "OpenSSL error:" << errBuf;
            return result;
        }

        // 获取 RSA 密钥大小
        int keySize = RSA_size(rsaKey);
        int step = 117; // PKCS1 padding最大块大小为117字节（对于1024位密钥）

        // 分块加密
        for (int i = 0; i < originData.length(); i += step) {
            QByteArray chunk = originData.mid(i, step);

            // 分配加密缓冲区
            QByteArray encryptedData(keySize, 0);
            int encryptedLen = 0;

            // 执行 RSA-PKCS1_PADDING 加密（与服务端保持一致）
            encryptedLen = RSA_public_encrypt(
                chunk.length(),
                reinterpret_cast<const unsigned char*>(chunk.constData()),
                reinterpret_cast<unsigned char*>(encryptedData.data()),
                rsaKey,
                RSA_PKCS1_PADDING  // 改为使用PKCS1填充而不是OAEP
                );

            if (encryptedLen == -1) {
                qWarning() << "RSA encryption failed";
                unsigned long err = ERR_get_error();
                char errBuf[256];
                ERR_error_string_n(err, errBuf, sizeof(errBuf));
                qWarning() << "OpenSSL error:" << errBuf;
                RSA_free(rsaKey);
                return result;
            }

            // 调整缓冲区大小并进行 Base64 编码
            encryptedData.resize(encryptedLen);
            QString base64Encrypted = encryptedData.toBase64();
            result.append(base64Encrypted);
        }

        RSA_free(rsaKey);
#else
        qWarning() << "not android or ios dont use openssl";
#endif
#else
        // 如果没有 OpenSSL，可以使用 QT 内置的 RSA 实现（功能有限）
        Q_UNUSED(data);
        Q_UNUSED(publicKey);
        qWarning() << "OpenSSL not available, RSA encryption not supported";
#endif

        return result;
    }

    /**
     * @brief 生成签名（如果需要额外的签名机制）
     * @param data 待签名的数据
     * @param secretKey 密钥
     * @return 签名值
     */
    static QString generateSignature(const QString& data, const QString& secretKey) {
        // 使用 SHA256 生成哈希
        QByteArray signData = (data + secretKey).toUtf8();
        QByteArray hash = QCryptographicHash::hash(signData, QCryptographicHash::Sha256);
        return hash.toHex();
    }

    static QJsonArray GetRSA(QString data, QString key)
    {
        // 使用 RSA 加密数据
        QStringList encryptedData = rsaEncrypt(data, key);

        // 将加密后的数据数组转换为 JSON 数组
        QJsonArray dataArray;
        for (const QString& item : encryptedData) {
            dataArray.append(item);
        }
        return dataArray;
    }

    /**
     * @brief 获取RSA加密后的密码字符串（用于登录）
     * @param data 待加密的数据
     * @param key 公钥
     * @return 加密后的字符串
     */
    static QString GetRSAPassword(QString data, QString key)
    {
        // 使用 RSA 加密数据
        QStringList encryptedData = rsaEncrypt(data, key);

        // 对于登录，我们只需要第一块加密数据（因为密码通常不会太长）
        // 如果有多块，只取第一块
        if (!encryptedData.isEmpty()) {
            return encryptedData.first();
        }
        return QString();
    }

    /**
     * @brief 构建提交请求
     * @param surveyPath 问卷路径
     * @param formData 表单数据
     * @param encryptType 加密类型
     * @param secretKey 密钥
     * @return 构建的请求对象
     */
    static QJsonObject buildSubmitRequest(
        const QString& surveyPath,
        const QJsonObject& formData,
        QJsonObject encryotObj,
        qint32 diff) {

        QJsonObject request;
        request["surveyPath"] = surveyPath;
        request["clientTime"] = QDateTime::currentMSecsSinceEpoch();
        request["diffTime"] = diff;

        // 将表单数据转换为 JSON 字符串
        QJsonDocument doc(formData);
        QString dataStr = doc.toJson(QJsonDocument::Compact);

        // 如果有加密信息，则进行加密 只针对表单数据进行加密
        QJsonObject serObj = encryotObj["data"].toObject();
        if (!encryotObj.isEmpty() && !serObj["secretKey"].toString().isEmpty() && encryotObj["encryptType"].toString() == "rsa") {
            // 使用 RSA 加密数据
            QStringList encryptedData = rsaEncrypt(dataStr, serObj["secretKey"].toString());

            // 将加密后的数据数组转换为 JSON 数组
            QJsonArray dataArray;
            for (const QString& item : encryptedData) {
                dataArray.append(item);
            }
            request["data"] = dataArray;
            request["encryptType"] = encryotObj["encryptType"].toString();
            request["sessionId"] = serObj["sessionId"].toString();
        } else {
            // 不加密直接使用 JSON 字符串
            request["data"] = dataStr;
        }

        // 添加签名
        request["sign"] = SurveyEncrypt::addSignToRequest(request);

        return request;
    }



};


#endif // SURVEYENCRYPT_H
