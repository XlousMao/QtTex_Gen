#include <QString>


class CAnkiCard
{
public:
	explicit CAnkiCard(const QString& FrontText = "", const QString& BackExtra = "", const QString& tags = "") 
		: m_FrontText(FrontText)
		, m_BackExtra(BackExtra)
		, m_tags(tags)
	{
	}
	CAnkiCard(const CAnkiCard& another)
		: m_FrontText(another.m_FrontText)
		, m_BackExtra(another.m_BackExtra)
		, m_tags(another.m_tags)
	{
	}
	CAnkiCard& operator=(const CAnkiCard& another)
	{	
		if (this == &another) {
			return *this;
		}
		m_tags = another.m_tags;
		m_FrontText = another.m_FrontText;
		m_BackExtra = another.m_BackExtra;
		return *this;
	}
	
	QString toString() 
	{
		//Todo:将单个卡片的这三个内容按特定格式组成一个字符串，方便后续保存到文件中
	};

private:
	QString GetFrontText() const { return m_FrontText; }
	QString GetBackExtra() const { return m_BackExtra; }
	QString GetTags() const { return m_tags; }
	QString SetFrontText(const QString& text) { m_FrontText = text; return m_FrontText; }
	QString SetBackExtra(const QString& text) { m_BackExtra = text; return m_BackExtra; }
	QString SetTags(const QString& text) { m_tags = text; return m_tags; }
private:
	QString m_FrontText{}; // 正面文本
	QString m_BackExtra{};  // 背面额外
	QString m_tags{};//标签
};