#ifndef FILEHANDER_H
#define FILEHANDER_H

#include <QString>

class CFileHandler
{
private:
	CFileHandler()=default;
	~CFileHandler()=default;
public:
	static QString readFile(const QString& filePath);
	static bool saveFile(const QString& filePath, const QString& content);
};

#endif