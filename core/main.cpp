#include <QtGui/QApplication>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QTranslator>

#include "scopemainwindow.h"

int main(int argc, char *argv[])
{
    // Setup application
    QApplication a(argc, argv);
    a.setApplicationName("GECKO");
    a.setOrganizationName("Institut für Kernphysik, TU Darmstadt");
    a.setApplicationVersion("0.6");

    QT_REQUIRE_VERSION(argc, argv, "4.4")

    QTranslator qtTr;
    qtTr.load ("qt_" + QLocale::system ().name (), QLibraryInfo::location (QLibraryInfo::TranslationsPath));
    a.installTranslator (&qtTr);

    QTranslator scopeTr;
    scopeTr.load ("gecko_" + QLocale::system ().name ());
    a.installTranslator (&scopeTr);

    // Open windows
    ScopeMainWindow s;
    s.show();

    return a.exec();
}
