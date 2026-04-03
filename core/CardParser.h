#include <QString>
#include <QList>
#include "AnkiCard.h"

/*
md文档理解读一系列的文本，解析成CAnkiCard对象

*/
class CAnkiCardParser  
{
	QList<CAnkiCard>parseMarkdown(const QString& rawText);





private:
	QList<CAnkiCard>m_AnkiCardList;
};



