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
#ifndef CREATEPROFILEWINDOW_H
#define CREATEPROFILEWINDOW_H

#include <QLineEdit>
#include <QWidget>

class CreateProfileWindow : public QWidget
{
  Q_OBJECT
public:
  CreateProfileWindow(QWidget *parent = nullptr);

  void
  creatWindow();

private:
  void
  checkInput();

  enum ErrorType
  {
    EmptyProfile,
    ProfileExists,
    EmptyUsername,
    EmptyPassword,
    NotEqualPasswords
  };

  void
  errorDialog(const ErrorType &er_type);

  QLineEdit *profname;
  QLineEdit *username;
  QLineEdit *password;
  QLineEdit *password_confirmation;

public:
signals:
  void
  signalProfile(const std::string &profile, const std::string &username,
                const std::string &password);
};

#endif // CREATEPROFILEWINDOW_H
