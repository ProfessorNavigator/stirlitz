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

#include <CreateProfileWindow.h>
#include <QDir>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CreateProfileWindow::CreateProfileWindow(QWidget *parent) : QWidget(parent)
{
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(tr("Profile creation"));
  this->setWindowFlag(Qt::Window, true);
  this->setWindowModality(Qt::WindowModality::WindowModal);
}

void
CreateProfileWindow::creatWindow()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  profname = new QLineEdit;
  profname->setPlaceholderText(tr("Profile name"));
  v_box->addWidget(profname);

  username = new QLineEdit;
  username->setPlaceholderText(tr("User name"));
  v_box->addWidget(username);

  password = new QLineEdit;
  password->setPlaceholderText(tr("Password"));
  password->setEchoMode(QLineEdit::Password);
  v_box->addWidget(password);

  password_confirmation = new QLineEdit;
  password_confirmation->setPlaceholderText(tr("Password confirmation"));
  password_confirmation->setEchoMode(QLineEdit::Password);
  v_box->addWidget(password_confirmation);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *apply = new QPushButton;
  apply->setText(tr("Apply"));
  connect(apply, &QPushButton::clicked, this,
          &CreateProfileWindow::checkInput);
  v_box->addWidget(apply);

  QPushButton *cancel = new QPushButton;
  cancel->setText(tr("Cancel"));
  connect(cancel, &QPushButton::clicked, this, &QWidget::close);
  v_box->addWidget(cancel);
}

void
CreateProfileWindow::checkInput()
{
  std::string prof_nm = profname->text().toStdString();
  if(prof_nm.empty())
    {
      errorDialog(ErrorType::EmptyProfile);
      return void();
    }

  std::filesystem::path p
      = std::filesystem::u8path(QDir::homePath().toStdString())
        / std::filesystem::u8path(".local") / std::filesystem::u8path("share")
        / std::filesystem::u8path("Stirlitz")
        / std::filesystem::u8path(prof_nm);
  if(std::filesystem::exists(p))
    {
      errorDialog(ErrorType::ProfileExists);
      return void();
    }

  std::string user_nm = username->text().toStdString();
  if(user_nm.empty())
    {
      errorDialog(ErrorType::EmptyUsername);
      return void();
    }

  std::string passwd = password->text().toStdString();
  if(passwd.empty())
    {
      errorDialog(ErrorType::EmptyPassword);
      return void();
    }

  std::string ch_passwd = password_confirmation->text().toStdString();
  if(passwd != ch_passwd)
    {
      errorDialog(ErrorType::NotEqualPasswords);
      return void();
    }
  emit signalProfile(prof_nm, user_nm, passwd);

  this->close();
}

void
CreateProfileWindow::errorDialog(const ErrorType &er_type)
{
  QMessageBox *msg = new QMessageBox(this);
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModality::WindowModal);

  msg->setIcon(QMessageBox::Icon::Critical);

  switch(er_type)
    {
    case ErrorType::EmptyProfile:
      {
        msg->setText(tr("Profile name can not be empty!"));
        break;
      }
    case ErrorType::ProfileExists:
      {
        msg->setText(tr("Profile already exists!"));
        break;
      }
    case ErrorType::EmptyUsername:
      {
        msg->setText(tr("User name can not be empty!"));
        break;
      }
    case ErrorType::EmptyPassword:
      {
        msg->setText(tr("Password can not be empty!"));
        break;
      }
    case ErrorType::NotEqualPasswords:
      {
        msg->setText(tr("Passwords are not equal!"));
        break;
      }
    default:
      break;
    }

  QPushButton *close = msg->addButton(QMessageBox::StandardButton::Close);
  connect(close, &QPushButton::clicked, msg, &QMessageBox::close);

  msg->show();
}
