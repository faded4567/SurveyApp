#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPixmap>
#include <QDebug>
#include <QJsonArray>
#include <QDateTime>
#include <QFile>
#include <QHttpMultiPart>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include "surveyencrypt.h"
#include "locationmanager.h"

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager& instance();


    QString authToken() const;
    QString getBaseUrl() const { return m_baseUrl; }


signals:
    void loginSuccess(const QJsonObject& userInfo);
    void loginFailed(const QString& error);
    void registerSuccess(const QJsonObject& userInfo);
    void registerFailed(const QString& error);
    void surveySchemaReceived(const QJsonObject& schema);
    void submitSuccess();
    void submitFailed(const QString& error);
    void networkError(const QString& error);
    void captchaReceived(const QString& captchaId, const QPixmap& captchaImage);
    void currentUserReceived(const QJsonObject& userInfo);
    void projectListReceived(const QJsonArray& projects);
    void fileUploadSuccess(const QJsonObject& response);
    void fileUploadFailed(const QString& error);

public slots:
    void InitManager();
    void onRefreshCaptcha();

    void setAuthToken(const QString& token);
    // 用户认证相关
    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& password);

    // 问卷相关
    void getSurveyList(int pageSize = 10, int curPage = 1);
    void getSurveySchema(const QString& surveyId);
    void submitResponse(const QString& surveyId, const QJsonObject& data, qint64 diffTime);
    void uploadFile(const QString& projectId, const QString& questionId, const QString& filePath);
    
    // 仪表板相关
    void getCurrentUser();
    void getProjectList();

private slots:
    void onReplyFinished(QNetworkReply* reply);


private:
    explicit NetworkManager(QObject* parent = nullptr);
    QNetworkRequest createRequest(const QString& url);
    QString getLocalIPv4Address();
    void handleCurrentUser(QNetworkReply* reply, QJsonObject jsonObj);
    void handleSystemInfo(QNetworkReply* reply, QJsonObject jsonObj);
    void handleLoginResponse(QNetworkReply* reply, QJsonObject jsonObj);
    void handleRegisterResponse(QNetworkReply* reply,QJsonObject jsonObj);
    void handleSurveyListResponse(QJsonObject jsonObj);
    void handleSurveySchemaResponse(QJsonObject jsonObj);
    void handleSubmitResponse(QNetworkReply* reply,QJsonObject jsonObj);
    void handleProjectListResponse(QJsonObject jsonObj);
    void handleFileUploadFinished(QNetworkReply* reply, QJsonObject jsonObj);

    QNetworkAccessManager* m_networkManager;
    QString m_authToken;
    QString m_baseUrl = "http://8.141.124.154:1991/api";
    QJsonArray m_metaArray;
    QJsonArray m_SchemaMetaArray;
    QJsonObject m_encryptInfo;
    QString m_currentUsername;
};

#endif // NETWORKMANAGER_H
