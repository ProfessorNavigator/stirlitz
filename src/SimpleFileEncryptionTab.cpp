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

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <SimpleFileEncryptionTab.h>
#include <filesystem>

#ifndef __ANDROID__
#include <iostream>
#else
#include <android/log.h>
#define APPNAME "Stirlitz"
#endif

SimpleFileEncryptionTab::SimpleFileEncryptionTab(QWidget *parent,
                                                 Stirlitz *spy,
                                                 const bool &show_as_window)
    : QWidget(parent)
{
  this->spy = spy;
  this->show_as_window = show_as_window;
  if(show_as_window)
    {
      this->setAttribute(Qt::WA_DeleteOnClose, true);
      this->setWindowTitle(tr("Local file encryption"));
      this->setWindowFlag(Qt::Window, true);
      this->setWindowModality(Qt::WindowModality::WindowModal);
    }
  else
    {
      creatWidget();
    }
}

void
SimpleFileEncryptionTab::creatWidget()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  src_file = new QLineEdit;
  src_file->setPlaceholderText(tr("Source file"));
  v_box->addWidget(src_file);

  QPushButton *open = new QPushButton;
  open->setText(tr("Open"));
  connect(open, &QPushButton::clicked, this,
          &SimpleFileEncryptionTab::openFileDialog);
  v_box->addWidget(open, 0, Qt::AlignVCenter | Qt::AlignRight);

  out_file = new QLineEdit;
  out_file->setPlaceholderText(tr("Out file"));
  v_box->addWidget(out_file);

  QPushButton *save_as = new QPushButton;
  save_as->setText(tr("Save as..."));
  connect(save_as, &QPushButton::clicked, this,
          &SimpleFileEncryptionTab::saveFileDialog);
  v_box->addWidget(save_as, 0, Qt::AlignVCenter | Qt::AlignRight);

  username = new QLineEdit;
  username->setPlaceholderText(tr("User name"));
  v_box->addWidget(username);

  password = new QLineEdit;
  password->setPlaceholderText(tr("Password"));
  v_box->addWidget(password);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *encrypt = new QPushButton;
  encrypt->setText(tr("Encrypt"));
  connect(encrypt, &QPushButton::clicked, this,
          [this]
            {
              resultFunc(true);
            });
  h_box->addWidget(encrypt, 0, Qt::AlignCenter);

  QPushButton *decrypt = new QPushButton;
  decrypt->setText(tr("Decrypt"));
  connect(decrypt, &QPushButton::clicked, this,
          [this]
            {
              resultFunc(false);
            });
  h_box->addWidget(decrypt, 0, Qt::AlignCenter);

  if(show_as_window)
    {
      QPushButton *close = new QPushButton;
      close->setText(tr("Close"));
      connect(close, &QPushButton::clicked, this, &QWidget::close);
      v_box->addWidget(close, Qt::AlignCenter);
    }

  v_box->addStretch();
}

void
SimpleFileEncryptionTab::openFileDialog()
{
  QFileDialog *fd;
  if(show_as_window)
    {
      fd = new QFileDialog(this);
    }
  else
    {
      fd = new QFileDialog(this->window());
    }
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::ExistingFile);
  fd->setOption(QFileDialog::ReadOnly);

  connect(fd, &QFileDialog::fileSelected, src_file,
          [this](const QString &text)
            {
              src_file->setText(text);
              std::filesystem::path p
                  = std::filesystem::u8path(text.toStdString());
              if(!p.empty())
                {
                  p = p.parent_path()
                      / std::filesystem::u8path(p.stem().u8string() + "_new"
                                                + p.extension().u8string());
                  out_file->setText(p.u8string().c_str());
                }
            });

  fd->show();
}

void
SimpleFileEncryptionTab::saveFileDialog()
{
  QFileDialog *fd;
  if(show_as_window)
    {
      fd = new QFileDialog(this);
    }
  else
    {
      fd = new QFileDialog(this->window());
    }
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptSave);
  QString str = src_file->text();
  if(str.isEmpty())
    {
      fd->setDirectory(QDir::homePath());
    }
  else
    {
      std::filesystem::path p = std::filesystem::u8path(str.toStdString());
      fd->setDirectory(p.parent_path().u8string().c_str());
    }

  connect(fd, &QFileDialog::fileSelected, out_file, &QLineEdit::setText);

  fd->show();
}

void
SimpleFileEncryptionTab::resultFunc(const bool &encrypt)
{
  std::filesystem::path s_path
      = std::filesystem::u8path(src_file->text().toStdString());
  if(!std::filesystem::exists(s_path))
    {
      errorDialog(ErrorType::SourceNotExists);
      return void();
    }

  std::filesystem::path r_path
      = std::filesystem::u8path(out_file->text().toStdString());
  if(r_path.empty())
    {
      errorDialog(ErrorType::ResultEmpty);
      return void();
    }

  if(s_path == r_path)
    {
      errorDialog(ErrorType::PathsEqual);
      return void();
    }

  std::string unm = username->text().toStdString();
  if(unm.empty())
    {
      errorDialog(ErrorType::UsernameEmpty);
      return void();
    }

  std::string passwd = password->text().toStdString();
  if(passwd.empty())
    {
      errorDialog(ErrorType::PasswordEmpty);
      return void();
    }

  try
    {
      if(encrypt)
        {
          spy->encryptFile(s_path, r_path, unm, passwd);
        }
      else
        {
          spy->decryptFile(s_path, r_path, unm, passwd);
        }
      errorDialog(ErrorType::Success);
    }
  catch(std::exception &er)
    {
#ifndef __ANDROID__
      std::cout << "SimpleFileEncryptionTab::resultFunc: " << er.what()
                << std::endl;
#else
      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                          "SimpleFileEncryptionTab::resultFunc: %s",
                          er.what());
#endif
      errorDialog(ErrorType::Error);
    }
}

void
SimpleFileEncryptionTab::errorDialog(const ErrorType &er)
{
  QMessageBox *msg;
  if(show_as_window)
    {
      msg = new QMessageBox(this);
    }
  else
    {
      msg = new QMessageBox(this->window());
    }
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModal);

  msg->setIcon(QMessageBox::Critical);
  msg->addButton(QMessageBox::Close);

  switch(er)
    {
    case ErrorType::SourceNotExists:
      {
        msg->setText(tr("Source file does not exist!"));
        break;
      }
    case ErrorType::ResultEmpty:
      {
        msg->setText(tr("Out file path is empty!"));
        break;
      }
    case ErrorType::PathsEqual:
      {
        msg->setText(
            tr("Source file path and out file path cannot be equal!"));
        break;
      }
    case ErrorType::UsernameEmpty:
      {
        msg->setText(tr("User name is empty!"));
        break;
      }
    case ErrorType::PasswordEmpty:
      {
        msg->setText(tr("Password is empty!"));
        break;
      }
    case ErrorType::Error:
      {
        msg->setText(tr("Error! See system log for details."));
        break;
      }
    case ErrorType::Success:
      {
        msg->setIcon(QMessageBox::Information);
        msg->setText(tr("Operation successfully completed!"));
        break;
      }
    default:
      break;
    }

  msg->show();
}
