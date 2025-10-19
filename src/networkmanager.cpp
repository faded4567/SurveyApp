#include "networkmanager.h"
#include <QNetworkRequest>
#include <QFileInfo>
#include <QHttpMultiPart>


NetworkManager& NetworkManager::instance()
{
    static NetworkManager instance;
    return instance;
}

NetworkManager::NetworkManager(QObject* parent) : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &NetworkManager::onReplyFinished);
}

void NetworkManager::setAuthToken(const QString& token)
{
    m_authToken = token;
}

QString NetworkManager::authToken() const
{
    return m_authToken;
}

void NetworkManager::InitManager()
{
    // 获取RSA公钥
    QNetworkRequest request = createRequest("/system");
    m_networkManager->get(request);


}

QNetworkRequest NetworkManager::createRequest(const QString& endpoint)
{
    QUrl url(m_baseUrl + endpoint);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_authToken).toUtf8());
    }

    return request;
}




void NetworkManager::uploadFile(const QString& projectId, const QString& questionId, const QString& filePath)
{
    QFile* file = new QFile(filePath);
    if (!file->open(QIODevice::ReadOnly)) {
        emit fileUploadFailed("无法打开文件");
        delete file;
        return;
    }
    
    // 创建 multipart 请求
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    // 添加文件数据
    QHttpPart filePart;
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QString contentType;
    
    // 根据文件扩展名设置内容类型
    QString extension = fileInfo.suffix().toLower();
    if (extension == "png") {
        contentType = "image/png";
    } else if (extension == "jpg" || extension == "jpeg") {
        contentType = "image/jpeg";
    } else if (extension == "gif") {
        contentType = "image/gif";
    } else if (extension == "bmp") {
        contentType = "image/bmp";
    } else if (extension == "mp4") {
        contentType = "video/mp4";
    } else if (extension == "mov") {
        contentType = "video/quicktime";
    } else if (extension == "wmv") {
        contentType = "video/x-ms-wmv";
    } else if (extension == "avi") {
        contentType = "video/x-msvideo";
    } else if (extension == "mkv") {
        contentType = "video/x-matroska";
    } else if (extension == "mp3") {
        contentType = "audio/mpeg";
    }else if (extension == "aac") {
        contentType = "audio/aac";
    }
    else {
        contentType = "application/octet-stream";
    }
    
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
    filePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
    
    multiPart->append(filePart);
    
    // 添加其他必需的字段
    QHttpPart projectIdPart;
    projectIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"projectId\""));
    projectIdPart.setBody(projectId.toUtf8());
    multiPart->append(projectIdPart);
    
    QHttpPart questionIdPart;
    questionIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"questionId\""));
    questionIdPart.setBody(questionId.toUtf8());
    multiPart->append(questionIdPart);
    
    QHttpPart fileTypePart;
    fileTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"fileType\""));
    fileTypePart.setBody("4"); // ANSWER_ATTACHMENT type
    multiPart->append(fileTypePart);
    
    // 添加publicUpload标志
    QHttpPart publicUploadPart;
    publicUploadPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"publicUpload\""));
    publicUploadPart.setBody("true");
    multiPart->append(publicUploadPart);
    
    // 发送请求
    QUrl url(m_baseUrl + "/public/upload");
    QNetworkRequest request(url);
    
    if (!m_authToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_authToken).toUtf8());
    }
    
    m_networkManager->post(request, multiPart);
}

void NetworkManager::handleFileUploadFinished(QNetworkReply* reply, QJsonObject jsonObj)
{
    if (jsonObj.value("code").toInt(0) == 200 || !jsonObj.contains("error")) {
        // 成功上传
        emit fileUploadSuccess(jsonObj);
    }
    else
    {
        QString errorMsg = "upload failed";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        }
        emit networkError(errorMsg);
    }
}

void NetworkManager::handleCurrentUser(QNetworkReply *reply, QJsonObject jsonObj)
{
    if (jsonObj.value("code").toInt(0) == 200 || !jsonObj.contains("error")) {
        // 解析并保存当前用户名
        if (jsonObj.contains("data") && jsonObj["data"].toObject().contains("name")) {
            m_currentUsername = jsonObj["data"].toObject()["name"].toString();
        }
        emit currentUserReceived(jsonObj);
    }
    else
    {
        QString errorMsg = "Failed to Current User Info";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        }
        emit networkError(errorMsg);
    }
}

void NetworkManager::handleSystemInfo(QNetworkReply *reply, QJsonObject jsonObj)
{
    if (jsonObj.value("code").toInt(0) == 200 || !jsonObj.contains("error")) {
        m_encryptInfo = jsonObj["data"].toObject();
        qDebug()<<"get server public key"<<m_encryptInfo["publicKey"].toString();
    }
    else
    {
        QString errorMsg = "Failed to Get system Info";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        }
        emit networkError(errorMsg);
    }
}

void NetworkManager::login(const QString& username, const QString& password)
{
    QNetworkRequest request = createRequest("/public/login");

    QJsonObject jsonData;
    jsonData["username"] = username;
    jsonData["password"] = SurveyEncrypt::GetRSAPassword(password, m_encryptInfo["publicKey"].toString());

    QByteArray postData = QJsonDocument(jsonData).toJson();
    m_networkManager->post(request, postData);
}

void NetworkManager::registerUser(const QString& username, const QString& password)
{
    QNetworkRequest request = createRequest("/public/register");

    QJsonObject jsonData;
    jsonData["username"] = username;
    jsonData["password"] = password;

    QByteArray postData = QJsonDocument(jsonData).toJson();
    m_networkManager->post(request, postData);
}

void NetworkManager::getSurveyList(int pageSize, int curPage)
{
    QString endpoint = QString("/project/list?pageSize=%1&curPage=%2").arg(pageSize).arg(curPage);

    QNetworkRequest request = createRequest(endpoint);
    m_networkManager->get(request);
}

void NetworkManager::getSurveySchema(const QString& surveyId)
{
    QNetworkRequest request = createRequest("/public/loadProject");
    
    QJsonObject requestData;
    requestData["id"] = surveyId;
    
    QByteArray postData = QJsonDocument(requestData).toJson();
    m_networkManager->post(request, postData);
}
QString NetworkManager::getLocalIPv4Address() {
    // 遍历所有网络接口
    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        // 检查网络接口是否有效、是否处于活动状态、是否正在运行并且不是回环接口
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            // 获取该接口的所有IP地址条目
            foreach (QNetworkAddressEntry entry, interface.addressEntries()) {
                // 获取当前条目的IP地址
                QHostAddress ip = entry.ip();
                // 检查是否是IPv4协议并且不是回环地址
                if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
                    // 返回找到的第一个符合条件的IPv4地址字符串
                    return ip.toString();
                }
            }
        }
    }
    // 如果没有找到符合条件的IP地址，返回本地回环地址（通常是 "127.0.0.1"）
    return QHostAddress(QHostAddress::LocalHost).toString();
}

void NetworkManager::submitResponse(const QString& surveyId, const QJsonObject& data, qint64 diffTime)
{
    QNetworkRequest request = createRequest("/public/saveAnswer");
    
    QJsonObject requestData;
    requestData["projectId"] = surveyId;
    requestData["answer"] = data;
    requestData["tempSave"] = 1;

    // Add metadata
    QJsonObject metaInfo;

    // Add client info
    QJsonObject clientInfo;
    clientInfo["agent"] = "Mobile Client";
    clientInfo["remoteIp"] = getLocalIPv4Address();
    metaInfo["clientInfo"] = clientInfo;

    // Add answer info
    QJsonObject answerInfo;
    qint64 startTime = QDateTime::currentMSecsSinceEpoch() - diffTime;
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    answerInfo["startTime"] = QString::number(startTime);
    answerInfo["endTime"] = QString::number(endTime);
    metaInfo["answerInfo"] = answerInfo;

    requestData["metaInfo"] = metaInfo;
    
    QByteArray postData = QJsonDocument(requestData).toJson();
    m_networkManager->post(request, postData);
}

void NetworkManager::getCurrentUser()
{
    QNetworkRequest request = createRequest("/currentUser");
    m_networkManager->get(request);
}

void NetworkManager::getProjectList()
{
    QNetworkRequest request = createRequest("/project/list");
    m_networkManager->get(request);
}

void NetworkManager::onRefreshCaptcha()
{
    // SurveyKing does not use captcha, keeping empty implementation
}

void NetworkManager::onReplyFinished(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit networkError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    QJsonObject jsonObj = jsonDoc.object();

    QString url = reply->url().toString();
    if(url.contains("/system")) {
        handleSystemInfo(reply, jsonObj);
    }else if (url.contains("/public/login")) {
        handleLoginResponse(reply, jsonObj);
    } else if (url.contains("/currentUser")) {
        handleCurrentUser(reply, jsonObj);
    } else if (url.contains("/public/register")) {
        handleRegisterResponse(reply, jsonObj);
    } else if (url.contains("/project/list")) {
        handleProjectListResponse(jsonObj);
    } else if (url.contains("/public/loadProject")) {
        handleSurveySchemaResponse(jsonObj);
    } else if (url.contains("/public/saveAnswer")) {
        handleSubmitResponse(reply, jsonObj);
    } else if (url.contains("/public/upload")) {
        handleFileUploadFinished(reply, jsonObj);
    }


    reply->deleteLater();
}

void NetworkManager::handleLoginResponse(QNetworkReply* reply, QJsonObject jsonObj)
{
    // Check if Authorization header is present
    if (reply->hasRawHeader("Authorization")) {
        QString token = reply->rawHeader("Authorization");
        // Remove "Bearer " prefix if present
        if (token.startsWith("Bearer ")) {
            token = token.mid(7);
        }
        setAuthToken(token);
        // emit loginSuccess(jsonObj);

        // 获取当前用户权限
        getCurrentUser();
    } else {
        QString errorMsg = "Login failed";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        } else if (jsonObj.contains("errorMsg")) {
            errorMsg = jsonObj["errorMsg"].toString();
        }
        emit loginFailed(errorMsg);
    }
}

void NetworkManager::handleRegisterResponse(QNetworkReply* reply, QJsonObject jsonObj)
{
    // For SurveyKing, assume success on 200 response
    emit registerSuccess(jsonObj);
}


void NetworkManager::handleProjectListResponse(QJsonObject jsonObj)
{
    if (jsonObj.value("code").toInt(0) == 200 || !jsonObj.contains("error")) {
        QJsonArray list = jsonObj["data"].toObject()["list"].toArray();
        emit projectListReceived(list);
    } else {
        QString errorMsg = "Failed to get project list";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        }
        emit networkError(errorMsg);
    }
}

void NetworkManager::handleSurveySchemaResponse(QJsonObject jsonObj)
{
    // SurveyKing returns the full project info directly
    // Check if the response contains data field
    if (jsonObj.contains("data")) {
        QJsonObject data = jsonObj["data"].toObject();
        emit surveySchemaReceived(data);
    } else {
        emit surveySchemaReceived(jsonObj);
    }
}

void NetworkManager::handleSubmitResponse(QNetworkReply* reply, QJsonObject jsonObj)
{
    if (jsonObj.value("code").toInt(0) == 200 || !jsonObj.contains("error")) {
        emit submitSuccess();
    } else {
        QString errorMsg = "Failed to submit survey";
        if (jsonObj.contains("message")) {
            errorMsg = jsonObj["message"].toString();
        } else if (jsonObj.contains("errorMsg")) {
            errorMsg = jsonObj["errorMsg"].toString();
        }
        emit submitFailed(errorMsg);
    }
}
