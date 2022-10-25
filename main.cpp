#pragma execution_character_set("utf-8")
#include "baumer_22_3_8.h"
#include <QtWidgets/QApplication>
#include <QTextCodec>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	QTextCodec* codec = QTextCodec::codecForName("utf-8");
	QTextCodec::setCodecForLocale(codec);

	baumer_22_3_8 w;
	w.show();
	return a.exec();
}
