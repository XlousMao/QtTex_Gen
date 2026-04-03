#include "AppController.h"
#include <QUrl>
#include <QRegularExpression>
#include <memory>

namespace {
QStringList parseTags(const QString& rawTags)
{
	QString normalized = rawTags;
   normalized.replace(QString::fromUtf8("，"), " ");
	normalized.replace(',', ' ');
	normalized.replace(';', ' ');
   normalized.replace(QString::fromUtf8("；"), " ");

	QStringList tags = normalized.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
	for (QString& t : tags) {
		t = t.trimmed();
	}
	tags.removeAll(QString());
	return tags;
}
}


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

void AppController::fetchModelFieldNames(const QString& modelName)
{
	m_ankiConnect->getModelFieldNames(modelName, [this](bool success, const QStringList& fieldNames, const QString& errorMsg) {
		emit ankiModelFieldNamesFetched(success, fieldNames, errorMsg);
	});
}

void AppController::addCardToAnki(const QString& deckName, 
                                  const QString& modelName, 
                                  const QString& frontField, const QString& frontContent, 
                                  const QString& backField, const QString& backContent)
{
	m_ankiConnect->addNote(deckName, modelName, frontField, frontContent, backField, backContent, 
     QStringList(),
		[this](bool success, const QString& errorMsg) {
			emit ankiNoteAdded(success, errorMsg);
		});
}

void AppController::importCardsToAnki(const QString& deckName, 
                                      const QString& modelName,
                                      const QString& frontField, 
                                      const QString& backField)
{
	if(m_currentRawText.isEmpty()){
		emit errorMessage("无法导入：没有加载内容！");
		return;
	}

	CAnkiCardParser parser;
	QList<CAnkiCard> cards = parser.parseMarkdown(m_currentRawText);

	if(cards.isEmpty()) {
		emit errorMessage("未检测到任何卡片，无法导入。");
		return;
	}

	emit errorMessage(QString("开始导入 %1 张卡片...").arg(cards.size()));

	auto completedCount = std::make_shared<int>(0);
	auto successCount = std::make_shared<int>(0);
	const int totalCount = cards.size();

	for (int i = 0; i < cards.size(); ++i) {
		CAnkiCard card = cards[i];

		// 转换 LaTeX (注意，如果没文字可能会被 AnkiConnect 拦截并判定 empty, 确保传入的不是完全空字符串)
		QString front = CLatexConverter::convertToAnkiLatex(card.GetFrontText());
		QString back = CLatexConverter::convertToAnkiLatex(card.GetBackExtra());
		QStringList tags = parseTags(card.GetTags());

		// 处理有些卡片背面额外区域可能为空，但 Anki 要求字段不能是 null 或者不提供（某些模板可能因为其中一个必填而报错）
		if (front.trimmed().isEmpty()) front = " ";
		if (back.trimmed().isEmpty()) back = " ";

		// 因为网络请求是异步的，我们在 lambda 当中用卡片序号来输出日志记录
       m_ankiConnect->addNote(deckName, modelName, frontField, front, backField, back, tags,
			[this, i, frontField, backField, completedCount, successCount, totalCount](bool success, const QString& errorMsg) {
				if (success) {
                  (*successCount)++;
					// Add note 的 result 为新笔记的 ID，这里不记录了，直接发信号提示单张卡片成功
					emit ankiNoteAdded(true, QString("Card %1 导入成功").arg(i + 1));
				} else {
                   QString lower = errorMsg.toLower();
					if (lower.contains("duplicate")) {
						emit ankiNoteAdded(false, QString("Card %1 跳过：检测到重复笔记 [%2]").arg(i + 1).arg(errorMsg));
					}
					else if (lower.contains("empty")) {
						emit ankiNoteAdded(false, QString("Card %1 导入失败 [%2] \n  (检查选中的 Model 是否真的有 '%3' 和 '%4' 这两个字段)").arg(i + 1).arg(errorMsg).arg(frontField).arg(backField));
					}
					else {
						emit ankiNoteAdded(false, QString("Card %1 导入失败 [%2]").arg(i + 1).arg(errorMsg));
					}
				}

				(*completedCount)++;
				if (*completedCount == totalCount) {
					int failed = totalCount - *successCount;
					if (failed == 0) {
						emit errorMessage(QString("导入完成：%1 张卡片成功！").arg(*successCount));
					} else {
						emit errorMessage(QString("导入完成：成功 %1，失败 %2（共 %3）").arg(*successCount).arg(failed).arg(totalCount));
					}
				}
			});
	}
}

