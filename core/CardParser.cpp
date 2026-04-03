#include "CardParser.h"
#include <QRegularExpression>

QList<CAnkiCard> CAnkiCardParser::parseMarkdown(const QString& rawText)
{
	QList<CAnkiCard> cardList;

	// 1. 按照至少 3 个连字符将整个文档切分成多个片段
	// 使用 QRegularExpression("-{3,}") 兼容 "---" 和 "------" 等多种情况
	QStringList chunks = rawText.split(QRegularExpression("-{3,}"), Qt::SkipEmptyParts);

	// 准备正则表达式，使用 DOTALL 模式 (QRegularExpression::DotMatchesEverythingOption)
	// 使用 [*\s]* 代表能匹配并忽略任意数量的星号和空白符。
	// 这样不论 AI 返回的是 "**文字：**"、"文字：" 还是 "文字： (带空格)" 都能被完美兼容。
	QRegularExpression re(
		R"([*\s]*文字：[*\s]*(.*?)[*\s]*背面额外：[*\s]*(.*?)[*\s]*标签：[*\s]*(.*))", 
		QRegularExpression::DotMatchesEverythingOption
	);

	// 2. 遍历每一个片段
	for (const QString& chunk : chunks) {
		// 先去掉片段首尾的冗余空白，如果为空就跳过
		QString trimmedChunk = chunk.trimmed();
		if (trimmedChunk.isEmpty()) {
			continue;
		}

		// 3. 匹配格式 "文字：... 背面额外：... 标签：..."
		QRegularExpressionMatch match = re.match(trimmedChunk);

		if (match.hasMatch()) {
			// 提取第一组（文字）、第二组（背面）、第三组（标签）
			// 并使用 trimmed() 清除多余的头尾换行或空格
			QString front = match.captured(1).trimmed();
			QString back = match.captured(2).trimmed();
			QString tags = match.captured(3).trimmed();

			// 存入列表
			cardList.append(CAnkiCard(front, back, tags));
		}
		else {
			// 容错性设计：如果某一段格式不对，不让程序崩溃。
		}
	}

	return cardList;
}

