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

#include <KeySetWindow.h>
#include <QClipboard>
#include <QGridLayout>
#include <QGuiApplication>
#include <QMessageBox>
#include <QPushButton>
#include <algorithm>

KeySetWindow::KeySetWindow(QWidget *parent) : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(tr("Key"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);
}

void
KeySetWindow::createWindow()
{
  QGridLayout *grid = new QGridLayout;
  this->setLayout(grid);

  key = new QLineEdit;
  key->setPlaceholderText(tr("Opponent key"));
  grid->addWidget(key, 0, 0, 1, 3);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  connect(apply, &QPushButton::clicked, this,
          [this]
            {
              QMessageBox *msg = new QMessageBox(this);
              msg->setAttribute(Qt::WA_DeleteOnClose);
              msg->setWindowModality(Qt::WindowModal);

              msg->setIcon(QMessageBox::Question);
              msg->setText(tr("Are you sure?"));
              msg->addButton(QMessageBox::Yes);
              msg->addButton(QMessageBox::No);

              connect(msg, &QMessageBox::buttonClicked, this,
                      [msg, this](QAbstractButton *button)
                        {
                          if(msg->buttonRole(button) == QMessageBox::YesRole)
                            {
                              checkInput();
                            }
                          msg->close();
                        });

              msg->show();
            });
  grid->addWidget(apply, 1, 0, 1, 1, Qt::AlignCenter);

  QPushButton *paste = new QPushButton;
  paste->setText(tr("Paste"));
  connect(paste, &QPushButton::clicked, key,
          [this]
            {
              QClipboard *cpb = QGuiApplication::clipboard();
              key->setText(cpb->text());
            });
  grid->addWidget(paste, 1, 1, 1, 1, Qt::AlignCenter);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  connect(cancel, &QPushButton::clicked, this, &QWidget::close);
  grid->addWidget(cancel, 1, 2, 1, 1, Qt::AlignCenter);
}

void
KeySetWindow::checkInput()
{
  std::string res = key->text().toStdString();
  res.erase(std::remove_if(res.begin(), res.end(),
                           [](const char &el)
                             {
                               switch(el)
                                 {
                                 case 48 ... 57:
                                 case 65 ... 70:
                                 case 97 ... 102:
                                   {
                                     return false;
                                   }
                                 default:
                                   return true;
                                 }
                             }),
            res.end());
  if(res.size() != 64)
    {
      QMessageBox *msg = new QMessageBox(this);
      msg->setAttribute(Qt::WA_DeleteOnClose);
      msg->setWindowModality(Qt::WindowModal);

      msg->setIcon(QMessageBox::Critical);
      msg->setText(tr("Incorrect key!"));

      msg->addButton(QMessageBox::Close);

      msg->show();

      return void();
    }

  emit signalKey(res);
  this->close();
}
