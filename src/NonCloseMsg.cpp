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

#include <NonCloseMsg.h>
#include <QLabel>
#include <QVBoxLayout>

NonCloseMsg::NonCloseMsg(QWidget *parent, const QString &msg_text)
    : QWidget(parent)
{
  this->msg_text = msg_text;
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);
  accept_close.store(false, std::memory_order_relaxed);
  creatWindow();
}

void
NonCloseMsg::allowClose(const bool &allow)
{
  accept_close.store(allow, std::memory_order_relaxed);
}

void
NonCloseMsg::closeEvent(QCloseEvent *event)
{
  if(accept_close.load(std::memory_order_relaxed))
    {
      event->accept();
    }
  else
    {
      event->ignore();
    }
}

void
NonCloseMsg::creatWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  QLabel *lab = new QLabel;
  lab->setText(msg_text);
  v_box->addWidget(lab, 0, Qt::AlignCenter);
}
