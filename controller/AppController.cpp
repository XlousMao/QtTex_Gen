#include "AppController.h"
#include <QUrl>


AppController::AppController(QObject* parent /*= nullptr*/)
	: QObject(parent)
	, m_currentRawText()
	, m_processedText()
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

