#include "LatexConverter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QString CLatexConverter::convertToAnkiLatex(const QString& text)
{
	QString output;
	int n = text.length();
	int pos = 0;
	
	while (pos < n) {
		int idxB = text.indexOf("{{c", pos);
		int idxD = text.indexOf("$", pos);
		
		if (idxB == -1 && idxD == -1) {
			output.append(text.mid(pos));
			break;
		}
		
		int firstIdx = -1;
		bool isCloze = false;
		if (idxB != -1 && idxD != -1) {
			if (idxB < idxD) { firstIdx = idxB; isCloze = true; }
			else { firstIdx = idxD; isCloze = false; }
		} else if (idxB != -1) {
			firstIdx = idxB; isCloze = true;
		} else {
			firstIdx = idxD; isCloze = false;
		}
		
		output.append(text.mid(pos, firstIdx - pos));
		pos = firstIdx;
		
		if (isCloze) {
			int p = firstIdx + 3;
			QString numStr;
			while (p < n && text[p].isDigit()) {
				numStr += text[p];
				p++;
			}
			if (p < n - 2 && text[p] == ':' && text[p+1] == ':' && !numStr.isEmpty()) {
				p += 2;
				int braceCount = 2;
				int startContent = p;
				while (p < n) {
					if (text[p] == '{') {
						braceCount++;
					} else if (text[p] == '}') {
						braceCount--;
						if (braceCount == 0 && p > 0 && text[p-1] == '}') {
							break;
						}
					}
					p++;
				}
				if (braceCount == 0 && p > 0 && text[p-1] == '}') {
					QString clozeContent = text.mid(startContent, p - 1 - startContent);
					pos = p + 1;
					
					clozeContent = clozeContent.trimmed();
					if (clozeContent.startsWith("$") && clozeContent.endsWith("$")) {
						clozeContent = clozeContent.mid(1, clozeContent.length() - 2);
					}
					
					bool isMath = clozeContent.contains('\\') || clozeContent.contains('^') ||
						clozeContent.contains('_') || clozeContent.contains(QRegularExpression("[a-zA-Z]{2,}"));
					
					clozeContent = fixBraceCollision(clozeContent);
					
					if (isMath) {
						output.append(QString("{{c%1::[$]%2[/$]}}").arg(numStr, clozeContent));
					}
					else {
						output.append(QString("{{c%1::%2}}").arg(numStr, clozeContent));
					}
				} else {
					output.append(text.mid(firstIdx, 3));
					pos = firstIdx + 3;
				}
			} else {
				output.append(text.mid(firstIdx, 3));
				pos = firstIdx + 3;
			}
		} else {
			int endD = text.indexOf("$", pos + 1);
			if (endD != -1) {
				QString plainMath = text.mid(pos + 1, endD - pos - 1);
				output.append(QString("[$]%1[/$]").arg(plainMath));
				pos = endD + 1;
			} else {
				output.append("$");
				pos++;
            }
		}
	}

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