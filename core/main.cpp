/*
Copyright 2011 Bastian Loeher, Roland Wirth

This file is part of GECKO.

GECKO is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

GECKO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
