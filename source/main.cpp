/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>

#include "main_app.h"
#include "version.h"
#include <glut.h>
#include <QtGui>

#ifdef WIN32 // TODO: cross platform support.
#include <windows.h>
#endif // WIN32

//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
#ifdef WIN32 // TODO: cross platform support.
    ::ShowWindow(::GetConsoleWindow(), SW_MINIMIZE);
#endif // WIN32

    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Confluence");
    QCoreApplication::setOrganizationDomain("confluencerd.com");
    QCoreApplication::setApplicationName("binary_visualizer");

    QString absPath = QDir(QApplication::applicationDirPath()).filePath("settings/style.ini");
    QSettings settings(absPath, QSettings::IniFormat);
    QVariant darkMode = settings.value("theme/darkMode", "0");
    const char* style_set = darkMode.toBool() ? ":/qstyle/dark/style.qss" : ":/qstyle/light/style.qss";

    CMain::loadStyle(style_set);
    CMain a;

    a.setWindowTitle(base_caption);
	a.resize(1200, 700);
	a.setWindowFlags(Qt::Window);
	a.show();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

    if (argc > 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        if (!a.loadFile(argv[1])) {
            exit(EXIT_FAILURE);
        }
    }

    return a.exec();
}
