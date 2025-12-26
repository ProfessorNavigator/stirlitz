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
#ifndef NONCLOSEMSG_H
#define NONCLOSEMSG_H

#include <QCloseEvent>
#include <QWidget>
#include <atomic>

class NonCloseMsg : public QWidget
{
  Q_OBJECT
public:
  NonCloseMsg(QWidget *parent = nullptr, const QString &msg_text = QString());

  void
  allowClose(const bool &allow);

protected:
  void
  closeEvent(QCloseEvent *event) override;

private:
  void
  creatWindow();

  QString msg_text;
  std::atomic<bool> accept_close;
};

#endif // NONCLOSEMSG_H
