#include <QApplication>

#include <QTranslator>

#include "qtbleditor.h"
#include "qtcompat.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("kambala");
    app.setApplicationName("QTblEditor");
    app.setApplicationVersion("1.5");
#ifdef OS_MACOS
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif


    QString locale = QLocale::system().name(), translationsPath =
#ifdef OS_MACOS
                     qApp->applicationDirPath() + "/../Resources/" +
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
