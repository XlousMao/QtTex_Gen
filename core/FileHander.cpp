#include "FileHander.h"
#include <QFile>
#include <QTextStream>

QString CFileHandler::readFile(const QString& filePath)
{
	QFile file(filePath);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){return QString() ;};

	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);
	return in.readAll();
}

bool CFileHandler::saveFile(const QString& filePath, const QString& content)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { return false; };

	QTextStream out(&file);
	out.setEncoding(QStringConverter::Utf8);
	out << content;
	return true;
}


