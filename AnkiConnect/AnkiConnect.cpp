#include "AnkiConnect.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QUrl>

AnkiConnect::AnkiConnect(QObject *parent) 
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
}

void AnkiConnect::sendRequest(const QString& action, int version, const QJsonObject& params, 
                              std::function<void(const QJsonValue& result, const QString& error)> callback)
{
    // 1. 组装请求 JSON 骨架
    QJsonObject requestObj;
    requestObj["action"] = action;
    requestObj["version"] = version;
    if (!params.isEmpty()) {
        requestObj["params"] = params;
    }

    QByteArray requestData = QJsonDocument(requestObj).toJson(QJsonDocument::Compact);

    // 2. 构造 HTTP 请求
    QUrl url(m_apiUrl);
    QNetworkRequest req(url);
    // AnkiConnect 仅支持 POST, 且不需要特别去写认证头部，直接明文丢 application/json 即可
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));

    // 3. 执行非阻塞式的异步网络推送
    QNetworkReply* reply = m_networkManager->post(req, requestData);

    // 4. 定义回调函数 (利用 Qt 的信号槽，当网络请求返回结果时被触发)
    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        // 请求结束后一定要删除 reply 对象
        reply->deleteLater(); 

        // 如果网络层出错 (比如没开 Anki)
        if (reply->error() != QNetworkReply::NoError) {
            callback(QJsonValue(), "网络异常或者没有运行Anki: " + reply->errorString());
            return;
        }

        // 解析回应 JSON
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        
        if (!responseDoc.isObject()) {
            callback(QJsonValue(), "解析错误: 返回内容不是 JSON 对象");
            return;
        }

        QJsonObject responseObj = responseDoc.object();
        
        // 提取 error // ["error": null] 或 ["error": "字符串"]
        QString errorMsg;
        QJsonValue errorVal = responseObj.value("error");
        if (!errorVal.isNull() && errorVal.isString()) {
            errorMsg = errorVal.toString();
        }

        // 提取 result
        QJsonValue resultVal = responseObj.value("result");

        if (!errorMsg.isEmpty()) {
            callback(QJsonValue(), errorMsg); // 逻辑层面的明确错误
        } else {
            callback(resultVal, QString());   // 成功！
        }
    });
}

void AnkiConnect::checkConnection(std::function<void(bool, int)> callback)
{
    // 抛弃所有的参数为空的 QJsonObject() 给 sendRequest
    sendRequest("version", 6, QJsonObject(), [callback](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty() || !result.isDouble()) {
            callback(false, 0);
        } else {
            callback(true, result.toInt());
        }
    });
}

void AnkiConnect::getDeckNames(std::function<void(bool, const QStringList&)> callback)
{
    sendRequest("deckNames", 6, QJsonObject(), [callback](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty() || !result.isArray()) {
            callback(false, QStringList());
            return;
        }
        
        // 解析 QJsonArray 到 C++ QStringList 中
        QStringList decks;
        QJsonArray arr = result.toArray();
        for (const QJsonValue& v : arr) {
            if (v.isString()) {
                decks.append(v.toString());
            }
        }
        callback(true, decks);
    });
}

void AnkiConnect::getModelNames(std::function<void(bool, const QStringList&)> callback)
{
    sendRequest("modelNames", 6, QJsonObject(), [callback](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty() || !result.isArray()) {
            callback(false, QStringList());
            return;
        }
        
        QStringList models;
        QJsonArray arr = result.toArray();
        for (const QJsonValue& v : arr) {
            if (v.isString()) {
                models.append(v.toString());
            }
        }
        callback(true, models);
    });
}

void AnkiConnect::addNote(const QString& deckName, const QString& modelName, 
                          const QString& frontField, const QString& frontContent, 
                          const QString& backField, const QString& backContent, 
                          std::function<void(bool, const QString&)> callback)
{
    // 组装 JSON params (参考 addNote api 的 params.note...)
    QJsonObject noteObj;
    noteObj["deckName"] = deckName;
    noteObj["modelName"] = modelName;

    QJsonObject fieldsObj;
    fieldsObj[frontField] = frontContent;
    fieldsObj[backField] = backContent;
    noteObj["fields"] = fieldsObj;

    QJsonArray tagsArr;
    tagsArr.append("QtTex_Gen_自动录入");
    noteObj["tags"] = tagsArr;

    QJsonObject paramsObj;
    paramsObj["note"] = noteObj;

    sendRequest("addNote", 6, paramsObj, [callback](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty()) {
            callback(false, error); // 失败，带上回传的错误消息 (比如牌组不存在)
        } else if (result.isNull()) {
            callback(false, "添加重复卡片或执行失败！(result为null)");
        } else {
            callback(true, QString()); // 成功，没有错误信息
        }
    });
}