#include <QApplication>

#include <QTranslator>

#include "qtbleditor.h"


int main(int argc, char *argv[])
{
	QApplication::setOrganizationName("kambala");
	QApplication::setApplicationName("QTblEditor");
	QApplication::setApplicationVersion("1.1.1");
	QApplication app(argc, argv);


	QString locale = QLocale::system().name(), translationsPath =
#ifdef Q_OS_MAC
					 QApplication::applicationDirPath() + "/../Resources/" +
#endif
					 "Translations/" + locale;

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + locale, translationsPath);
	app.installTranslator(&qtTranslator);

	QTranslator myappTranslator;
	myappTranslator.load("qtbleditor_" + locale, translationsPath);
	app.installTranslator(&myappTranslator);


	QTblEditor w;
	w.show();

	return app.exec();
}
