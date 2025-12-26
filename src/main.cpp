/*
 * Copyright (C) 2025 Yury Bobylev <bobilev_yury@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <MainWindow.h>
#include <QApplication>
#include <QTranslator>
#include <filesystem>

#ifndef __ANDROID__
#include <iostream>
#else
#define APPNAME "Stirlitz"
#include <android/log.h>
#endif

int
main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  QTranslator translator;
  if(translator.load(QLocale::system(), "Stirlitz", "_", ":/translations"))
    {
      a.installTranslator(&translator);
    }
  else
    {
#ifndef __ANDROID__
      std::cout << "Strilitz: translation file not found" << std::endl;
#else
      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Strilitz: %s",
                          "translation file not found");
#endif
    }

  MainWindow w;
  w.show();
  return a.exec();
}
