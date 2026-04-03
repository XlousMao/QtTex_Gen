#include "LatexConverter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QString CLatexConverter::convertToAnkiLatex(const QString& text)
{
	// 1. 让正则匹配任意填空 {{c1::...}} ，不再强求里面必须有 $
	// 重点修复：使用 \}\}(?!\}) 负向先行断言。
	// 这意味着当遇到多个 } 连在一起时（如 AI 输出的 }}}}），正则不会在第一个 }} 处停下，
	// 而是会一直匹配到最后两个不被 } 跟随的 }}，完美提取出完整的公式！
	QString pattern = R"((\{\{c(?<cloze_num>\d+)::(?<cloze_content>.*?)\}\}(?!\}))|(\$(?<plain_math>.*?)\$))";

	QRegularExpression re(pattern);
	QString output;
	int lastPos = 0;

	auto it = re.globalMatch(text);
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// 将匹配项之前的普通文本加入结果
		output.append(text.mid(lastPos, match.capturedStart() - lastPos));

		QString clozeContent = match.captured("cloze_content");
		if (!clozeContent.isEmpty()) {
			// --- 处理填空 ---
			QString num = match.captured("cloze_num");

			// AI 可能会多加多余的空格或 $, 先清理掉
			clozeContent = clozeContent.trimmed();
			if (clozeContent.startsWith("$") && clozeContent.endsWith("$")) {
				clozeContent = clozeContent.mid(1, clozeContent.length() - 2);
			}

			// 判断内容是不是公式？包含 \ 或者 ^ 或者 _ 或者多于两个字母，大概率是公式
			bool isMath = clozeContent.contains('\\') || clozeContent.contains('^') ||
				clozeContent.contains('_') || clozeContent.contains(QRegularExpression("[a-zA-Z]{2,}"));

			clozeContent = fixBraceCollision(clozeContent);

			if (isMath) {
				// 如果是公式，包裹成 Anki 公式
				output.append(QString("{{c%1::[$]%2[/$]}}").arg(num, clozeContent));
			}
			else {
				// 如果只是纯文字 (比如 {{c1::奇函数}})
				output.append(QString("{{c%1::%2}}").arg(num, clozeContent));
			}
		}
		else {
			// --- 处理普通公式 $...$ ---
			QString plainMath = match.captured("plain_math");
			output.append(QString("[$]%1[/$]").arg(plainMath));
		}

		lastPos = match.capturedEnd();
	}

	output.append(text.mid(lastPos));

	// 额外清扫：AI 若输出 "$y = $ {{c1::...}}" 这种错误格式，把独立落单的 $ 强行修复
	// 这里通过简单的 replace 处理两个相邻公式或残留 $ 的格式问题
	output.replace("[$] [/$]", " ");
	output.replace("$ {{", "{{");

	return output;
}

QString CLatexConverter::fixBraceCollision(QString math)
{
	while (math.contains("}}")) {
		math.replace("}}", "} }");
	}
	return math;
}