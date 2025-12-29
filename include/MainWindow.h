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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <FileTabWidget.h>
#include <QComboBox>
#include <QLineEdit>
#include <QMainWindow>
#include <QScrollArea>
#include <QShowEvent>
#include <Stirlitz.h>
#include <TextTabWidget.h>
#include <filesystem>
#include <thread>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  void
  enterWindow();

  void
  enterWindowCheckInput(QComboBox *profiles, QLineEdit *username,
                        QLineEdit *password);

  void
  mainWindow(const std::string &profile, const std::string &username,
             const std::string &password);

  QWidget *
  ownKeyWidget(const std::filesystem::path &prof_p);

  void
  resetOwnKey(const std::filesystem::path &prof_p, QScrollArea *scrl);

  QWidget *
  otherKeyWidget(const std::filesystem::path &prof_p);

  void
  removeProfile(const std::filesystem::path &prof_dir_path);

  std::filesystem::path home_p;

  std::thread *warn_thread = nullptr;

  Stirlitz *spy;
  std::shared_ptr<gcry_sexp> key_pair;
  std::shared_ptr<gcry_sexp> other_key;

  char *u_name = nullptr;
  size_t u_name_sz = 0;
  char *passwd = nullptr;
  size_t passwd_sz = 0;

  TextTabWidget *text_tab;
  FileTabWidget *file_tab;

  bool one_time_profile = false;

signals:
  void
  showWarning();
};
#endif // MAINWINDOW_H
