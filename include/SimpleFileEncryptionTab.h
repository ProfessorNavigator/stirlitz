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
#ifndef SIMPLEFILEENCRYPTIONTAB_H
#define SIMPLEFILEENCRYPTIONTAB_H

#include <QLineEdit>
#include <QWidget>
#include <Stirlitz.h>

class SimpleFileEncryptionTab : public QWidget
{
  Q_OBJECT
public:
  SimpleFileEncryptionTab(QWidget *parent, Stirlitz *spy,
                          const bool &show_as_window);

  void
  creatWidget();

private:
  void
  openFileDialog();

  void
  saveFileDialog();

  void
  resultFunc(const bool &encrypt);

  enum ErrorType
  {
    SourceNotExists,
    ResultEmpty,
    PathsEqual,
    UsernameEmpty,
    PasswordEmpty,
    Error,
    Success
  };

  void
  errorDialog(const ErrorType &er);

  Stirlitz *spy;
  bool show_as_window;

  QLineEdit *src_file;
  QLineEdit *out_file;

  QLineEdit *username;
  QLineEdit *password;
};

#endif // SIMPLEFILEENCRYPTIONTAB_H
