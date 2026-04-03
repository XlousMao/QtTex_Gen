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
	CAnkiCard(const CAnkiCard& another) = default;
	CAnkiCard& operator=(const CAnkiCard& another) = default;
	
	QString toString() const 
	{
		return QString("**文字：** %1\n\n**背面额外：** %2\n\n**标签：** %3\n")
			.arg(m_FrontText)
			.arg(m_BackExtra)
			.arg(m_tags);
	};

public:
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