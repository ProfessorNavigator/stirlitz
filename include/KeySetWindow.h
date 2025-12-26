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
#ifndef KEYSETWINDOW_H
#define KEYSETWINDOW_H

#include <QLineEdit>
#include <QWidget>

class KeySetWindow : public QWidget
{
  Q_OBJECT
public:
  KeySetWindow(QWidget *parent = nullptr);

  void
  createWindow();

private:
  void
  checkInput();

  QLineEdit *key;

public:
signals:
  void
  signalKey(const std::string &key);
};

#endif // KEYSETWINDOW_H
