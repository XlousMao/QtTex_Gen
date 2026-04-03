#include "AppController.h"
#include <QUrl>


AppController::AppController(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, m_currentRawText()
	, m_processedText()
	, m_ankiConnect(new AnkiConnect(this)) // 实例化 AnkiConnect 处理器，设置 AppController 为其父对象以自动管理内存
{

}

void AppController::openMarkdownFile(const QString& fileUrl)
{
	// QML 传来的路径通常带有 "file:///" 前缀，需要转换为本地绝对路径
	QString localPath = QUrl(fileUrl).toLocalFile();

	//调用现有的FileHandler
	m_currentRawText = CFileHandler::readFile(localPath);
	if(m_currentRawText.isEmpty()){
		emit errorMessage("无法读取文件，请检查路径和权限。");
		return;
	}

	emit fileLoaded(m_currentRawText);
}

void AppController::processCards()
{
	if(m_currentRawText.isEmpty()){
		emit errorMessage("没有加载任何文件，请先打开一个 Markdown 文件。");
		return;
	}

	//1. 解析 Markdown 文本为 Anki 卡片对象列表
	CAnkiCardParser parser;
	QList<CAnkiCard>cards = parser.parseMarkdown(m_currentRawText);

	//2. 准备一个字符串来存储最终处理结果
	m_processedText.clear();


	//3. 遍历每个卡片对象，转换为 Anki 格式的字符串(Md格式转换为Anki中LaTex格式)，并累加到结果中
	for (int i = 0; i < cards.size(); ++i) {
		CAnkiCard card = cards[i];

		//转换正面文字和背面额外文字的公式
		card.SetFrontText(CLatexConverter::convertToAnkiLatex(card.GetFrontText()));
		card.SetBackExtra(CLatexConverter::convertToAnkiLatex(card.GetBackExtra()));

		//重新拼接为md，然后追加到结果中
		m_processedText.append(card.toString());
		if (i < cards.size() - 1) {
			m_processedText.append("---\n\n");// 恢复卡片之间的分隔符
		}
	}

	emit processCompleted(m_processedText);
	emit errorMessage(QString("解析完成，共生成 %1 张卡片").arg(cards.size()));
}

void AppController::saveResultFile(const QString& fileUrl)
{
	QString localPath = QUrl(fileUrl).toLocalFile();
	bool ok = CFileHandler::saveFile(localPath, m_processedText);
	if (ok) {
		emit errorMessage("文件保存成功！");
	}
	else
	{
		emit errorMessage("文件保存失败，请检查路径和权限。");
	}
}

// -------------------- AnkiConnect 桥接部分 --------------------

void AppController::checkAnkiConnection()
{
	// 调用 m_ankiConnect 的 checkConnection 方法。
	// 这里传递了一个 Lambda 表达式，当获取到结果后它会被调用。
	// [this] 表示我们要在这个匿名函数里面使用当前 AppController 的函数和成员（比如 emit）。
	m_ankiConnect->checkConnection([this](bool success, int version) {
		// 发送信号给 QML（前端收到此信号后就可以弹出 Toast 提示等）
		emit ankiConnectionChecked(success, version);
	});
}

void AppController::fetchDeckNames()
{
	m_ankiConnect->getDeckNames([this](bool success, const QStringList& decks) {
		emit ankiDeckNamesFetched(success, decks);
	});
}

void AppController::fetchModelNames()
{
	m_ankiConnect->getModelNames([this](bool success, const QStringList& models) {
		emit ankiModelNamesFetched(success, models);
	});
}

void AppController::addCardToAnki(const QString& deckName, 
                                  const QString& modelName, 
                                  const QString& frontField, const QString& frontContent, 
                                  const QString& backField, const QString& backContent)
{
	m_ankiConnect->addNote(deckName, modelName, frontField, frontContent, backField, backContent, 
		[this](bool success, const QString& errorMsg) {
			emit ankiNoteAdded(success, errorMsg);
		});
}

