#ifndef _CARDPARSER_H
#define _CARDPARSER_H



#include <QString>
#include <QList>
#include "AnkiCard.h"

//把md格式的Qstring解析成CAnkiCard对象列表
class CAnkiCardParser  
{
public:
	QList<CAnkiCard>parseMarkdown(const QString& rawText);

private:
	QList<CAnkiCard>m_AnkiCardList;
};

#endif


