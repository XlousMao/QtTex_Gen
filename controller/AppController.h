#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "../core/core.h"
#include "../AnkiConnect/AnkiConnect.h"

// 前向声明，加快编译
class AnkiConnect;

class AppController : public QObject
{
	Q_OBJECT

public:
	explicit AppController(QObject* parent = nullptr);
	~AppController() = default;


	// Q_INVOKABLE 使得 QML 中可以直接调用这些 C++ 函式
	Q_INVOKABLE void openMarkdownFile(const QString& fileUrl);
	Q_INVOKABLE void processCards();
	Q_INVOKABLE void saveResultFile(const QString& fileUrl);

	// --- 暴露给 QML 调用的 AnkiConnect 接口 ---
	Q_INVOKABLE void checkAnkiConnection();
	Q_INVOKABLE void fetchDeckNames();
	Q_INVOKABLE void fetchModelNames();
 Q_INVOKABLE void fetchModelFieldNames(const QString& modelName);
	Q_INVOKABLE void addCardToAnki(const QString& deckName,
	                               const QString& modelName,
	                               const QString& frontField, const QString& frontContent,
	                               const QString& backField, const QString& backContent);
	Q_INVOKABLE void importCardsToAnki(const QString& deckName, 
	                                   const QString& modelName,
	                                   const QString& frontField, 
	                                   const QString& backField);

signals:
	// C++ 处理完后，发送信号通知 QML 更新界面文本
	void fileLoaded(const QString& rawContent);
	void processCompleted(const QString& resultText);
	void errorMessage(const QString& msg);

	// --- AnkiConnect 获取到异步流返回的数据后，发送给 QML 的信号 ---
	void ankiConnectionChecked(bool success, int version);
	void ankiDeckNamesFetched(bool success, const QStringList& decks);
	void ankiModelNamesFetched(bool success, const QStringList& models);
  void ankiModelFieldNamesFetched(bool success, const QStringList& fieldNames, const QString& errorMsg);
	void ankiNoteAdded(bool success, const QString& errorMsg);

private:
	QString m_currentRawText{};
	QString m_processedText{};

	// 维护一个 AnkiConnect 的实例指针
	AnkiConnect* m_ankiConnect;
};

#endif // APPCONTROLLER_H











