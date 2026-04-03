#include "LatexConverter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QString CLatexConverter::convertToAnkiLatex(const QString& text)
{
	// 1. 定义正则表达式 (使用原始字符串 R"(...)" 避免繁琐的转义)
	// 格式：(填空公式)|(普通公式)
	// 分组名与 Python 脚本保持一致：cloze_num, cloze_math, plain_math
	QString pattern = R"((\{\{c(?<cloze_num>\d+)::\$(?<cloze_math>.*?)\$\}\})|(\$(?<plain_math>.*?)\$))";

	QRegularExpression re(pattern);
	QString output;
	int lastPos = 0;

	// 2. 使用迭代器模拟 Python 的 re.sub 回调逻辑
	auto it = re.globalMatch(text);
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();

		// 将匹配项之前的普通文本加入结果
		output.append(text.mid(lastPos, match.capturedStart() - lastPos));

		// 3. 逻辑判断：是填空公式还是普通公式
		QString clozeMath = match.captured("cloze_math");
		if (!clozeMath.isEmpty()) {
			// --- 处理填空类公式 {{c1::$...$}} ---
			QString num = match.captured("cloze_num");

			// 调用辅助函数处理 }} 冲突
			QString fixedMath = fixBraceCollision(clozeMath);

			output.append(QString("{{c%1::[$]%2[/$]}}").arg(num, fixedMath));
		}
		else {
			// --- 处理普通公式 $...$ ---
			QString plainMath = match.captured("plain_math");
			output.append(QString("[$]%1[/$]").arg(plainMath));
		}

		lastPos = match.capturedEnd();
	}

	// 4. 加入最后一段文本
	output.append(text.mid(lastPos));

	return output;
}

QString CLatexConverter::fixBraceCollision(QString math)
{
	// 文档建议：必须增加一个循环替换步骤，将捕获到的公式字符串内部所有的 }} 替换为 } }
	// 这是为了防止 LaTeX 的闭合括号与 Anki 填空标签 }} 产生解析冲突
	while (math.contains("}}")) {
		math.replace("}}", "} }");
	}
	return math;
}