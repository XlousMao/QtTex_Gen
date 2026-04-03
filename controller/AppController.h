#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>
#include "../core/core.h"

class AppController : public QObject
{
	Q_OBJECT
		// 使用 QML_ELEMENT (Qt6) 或者在 main.cpp 注册

public:
	explicit AppController(QObject* parent = nullptr);
	~AppController() = default;


	// Q_INVOKABLE 使得 QML 中可以直接调用这些 C++ 函式
	Q_INVOKABLE void openMarkdownFile(const QString& fileUrl);
	Q_INVOKABLE void processCards();
	Q_INVOKABLE void saveResultFile(const QString& fileUrl);

signals:
	// C++ 处理完后，发送信号通知 QML 更新界面文本
	void fileLoaded(const QString& rawContent);
	void processCompleted(const QString& resultText);
	void errorMessage(const QString& msg);

private:
	QString m_currentRawText{};
	QString m_processedText{};
};

#endif // APPCONTROLLER_H






