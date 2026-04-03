#ifndef ANKICONNECT_H
#define ANKICONNECT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>
#include <functional>

class AnkiConnect : public QObject
{
    Q_OBJECT
public:
    explicit AnkiConnect(QObject *parent = nullptr);

    // --- 核心业务接口 (异步回调方式) ---
    // 通过 std::function 代替繁琐的一对一信号/槽, 使用上更直观顺滑

    // 1. 验证连接 (获取版本号指示是否成功)
    void checkConnection(std::function<void(bool success, int version)> callback);
    
    // 2. 获取当前 Anki 所有卡牌组的名称
    void getDeckNames(std::function<void(bool success, const QStringList& decks)> callback);
    
    // 3. 获取能够使用的所有笔记模板(模型)
    void getModelNames(std::function<void(bool success, const QStringList& models)> callback);

    // 3.1 获取某个模板的字段名（用于前后字段映射）
    void getModelFieldNames(const QString& modelName, std::function<void(bool success, const QStringList& fieldNames, const QString& errorMsg)> callback);
    
    // 4. 发送新建笔记请求
    void addNote(const QString& deckName, 
                 const QString& modelName, 
                 const QString& frontField, const QString& frontContent, 
                 const QString& backField, const QString& backContent, 
                 const QStringList& tags,
                 std::function<void(bool success, const QString& errorMsg)> callback);

private:
    // --- 基础通信与 JSON 封装 ---
    // 负责发出实际的 POST 请求并解析基础结构的 result 和 error
    void sendRequest(const QString& action, int version, const QJsonObject& params, 
                     std::function<void(const QJsonValue& result, const QString& error)> callback);

    QNetworkAccessManager* m_networkManager;
    const QString m_apiUrl = "http://127.0.0.1:8765";
};

#endif // ANKICONNECT_H