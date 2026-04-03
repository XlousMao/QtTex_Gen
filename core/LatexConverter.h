#ifndef LATEXCONVERTER_H
#define LATEXCONVERTER_H

#include <QString>

class CLatexConverter
{
public:
	// 核心转换方法
	static QString convertToAnkiLatex(const QString& text);

private:
	// 处理 Anki }} 冲突的内部辅助函数
	static QString fixBraceCollision(QString math);
};

#endif // LATEXCONVERTER_H