# AnkiConnect API 总结 - 自动录入卡片相关

为了实现将转换后的卡片自动录入到指定的 Anki 牌组中，我们只需要关注 AnkiConnect 提供的部分核心 API。这种基于 HTTP 的 JSON RPC 调用方式在现代应用开发中非常常见。

以下是实现“自动录入卡片”功能所需的关键请求：

## 1. 验证连接 (必须)
在进行任何操作前，需要先确认 Anki 和 AnkiConnect 是否正在运行并可以通信。

*   **API Action:** `version`
*   **用途:** 获取 AnkiConnect 的版本，验证连接是否成功。
*   **请求示例:**
    
    ```json
    {
        "action": "version",
        "version": 5
    }
    ```

## 2. 获取目标牌组和模板信息 (可选，但推荐)
在插入卡片前，你可能需要让用户选择，或者代码中验证牌组名和模板名是否有效。

*   **API Action:** `deckNames`
    *   **用途:** 获取当前用户的所有牌组名称列表。可以用来验证你想插入的牌组是否存在。
*   **API Action:** `modelNames`
    *   **用途:** 获取所有卡片模板 (模型) 的名称列表。

## 3. 核心功能：添加卡片 (必须)
这是最核心的接口，用于将生成的正面和背面内容实际写入 Anki。

*   **API Action:** `addNote` （添加单张卡片）
*   **用途:** 使用指定的牌组和模板，创建一条新笔记（卡片）。
*   **请求示例:**
    ```json
    {
        "action": "addNote",
        "version": 5,
        "params": {
            "note": {
                "deckName": "目标牌组名称",  // 例如: "Default"
                "modelName": "目标木板名称", // 例如: "Basic" 或你自定义的模板
                "fields": {
                    "正面字段名": "转换后的正面内容（含 {{c1::...}}）", // 通常是 "Front" 或 "正面"
                    "背面字段名": "转换后的背面内容",             // 通常是 "Back" 或 "背面"
                },
                "tags": [
                    "QtTex_Gen_自动导入",
                    "某个标签"
                ]
            }
        }
    }
    ```

## 开发者小建议 (C++ / Qt 实现指引)
在 Qt C++ 中，这种 HTTP JSON 通信非常容易实现：
1.  **使用 `QNetworkAccessManager`**: 发送 HTTP POST 请求到 `http://127.0.0.1:8765`。
2.  **使用 `QJsonObject` / `QJsonDocument`**: 组装上面的 JSON 请求结构。
3.  **解析返回的 JSON**: 读取返回的 `result` 字段判断是否成功（如果有错误看 `error` 字段）。

这个改造确实能将软件的自动化程度提升一个档次，直接跳过手动复制粘贴环节，极大地提高效率！你可以先用 Postman 或者 curl 去测试一下上面提及的 `addNote` 请求，验证通了之后再写到 Qt 代码里面。









我是不是这样理解，我就是追踪链路：
假设从程序调用checkAnkiConncetion()开始
线性执行
1.--->checkConncetion。传递的参数是一个lambda表达式，参数（success,version），内容是发送一个信号,为了形象的了解，我把他写成一个函数，void A(success,version){emit xxxxxxx},也就是调用A发送信号
2.--->进入checkConncetion执行，checkConncction的参数是刚刚那个lambda表达式，也是就函数A
3.--->执行checkConnection逻辑
4.---->调用sendRequest，参数是
（
"version", 6, QJsonObject(), 
[callback](const QJsonValue& result, const QString& error) {
        if (!error.isEmpty() || !result.isDouble()) {
            callback(false, 0);
        } else {
            callback(true, result.toInt());
        }
    }
)假设这个函数事C()。效果时如果参数error为空，那么就是执行A
5.----->进入sendRequest
6.------>执行sendRequest的逻辑：
6.1---->组装请求json的骨架
6.2--->构造http请求
6.3---->发送post请求:QNetworkReply* reply = m_networkManager->post(req, requestData);
6.4---->调用connect进行绑定,绑定reply的finished信号，槽函数是:

 [reply, callback]() {
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

我假设这个lambda表达式时函数B。
----->7.执行完毕

假设等待10s（为了理清楚路线，我夸大了）后请求返回了，reply发送了finished信号，这时候moc实际循环，发现这个信号，找到对应绑定的槽函数，也就是B()。这时候就调用B().（中断调用，回调，很形象）
a.------>调用B()。
b.----->执行B()逻辑：B在绑定时捕获了reply,和callback。
b.1------>删除reply对象
b.2---->如果网络层出问题,调用caalback,参数shi （QJsonValue(), "网络异常或者没有运行Anki: " + reply->errorString()），这个callback实际上时checkConnection传递那个callback,
b.3-------->拿到json对象
b.3--------->从json中，提取error和result
