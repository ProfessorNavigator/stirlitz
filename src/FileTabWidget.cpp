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

#include <FileTabWidget.h>
#include <NonCloseMsg.h>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <filesystem>
#include <thread>

#ifndef __ANDROID__
#include <iostream>
#else
#define APPNAME "Stirlitz"
#include <android/log.h>
#endif

FileTabWidget::FileTabWidget(QWidget *parent, Stirlitz *spy,
                             const std::shared_ptr<gcry_sexp> &key_pair,
                             const std::shared_ptr<gcry_sexp> &other_key)
    : QWidget(parent)
{
  this->spy = spy;
  this->key_pair = key_pair;
  this->other_key = other_key;
  connect(this, &FileTabWidget::signalMessage, this,
          &FileTabWidget::infoMessage);
  createWidget();
}

void
FileTabWidget::resetKeys(const std::shared_ptr<gcry_sexp> &key_pair,
                         const std::shared_ptr<gcry_sexp> &other_key)
{
  this->key_pair = key_pair;
  this->other_key = other_key;
}

void
FileTabWidget::createWidget()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  source_file = new QLineEdit;
  source_file->setPlaceholderText(tr("File to be encrypted or decrypted"));
  v_box->addWidget(source_file);

  QPushButton *open = new QPushButton;
  open->setText(tr("Open"));
  connect(open, &QPushButton::clicked, this, &FileTabWidget::openFileDialog);
  v_box->addWidget(open, 0, Qt::AlignVCenter | Qt::AlignRight);

  result_file = new QLineEdit;
  result_file->setPlaceholderText(tr("Resulting file path"));
  v_box->addWidget(result_file);

  QPushButton *save_as = new QPushButton;
  save_as->setText(tr("Save as..."));
  connect(save_as, &QPushButton::clicked, this,
          &FileTabWidget::saveFileDialog);
  v_box->addWidget(save_as, 0, Qt::AlignVCenter | Qt::AlignRight);

  v_box->addSpacing(20);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *encrypt = new QPushButton;
  encrypt->setText(tr("Encrypt"));
  connect(encrypt, &QPushButton::clicked, this, &FileTabWidget::encryptFile);
  h_box->addWidget(encrypt, 0, Qt::AlignCenter);

  QPushButton *decrypt = new QPushButton;
  decrypt->setText(tr("Decrypt"));
  connect(decrypt, &QPushButton::clicked, this, &FileTabWidget::decryptFile);
  h_box->addWidget(decrypt, 0, Qt::AlignCenter);

  v_box->addStretch();
}

void
FileTabWidget::openFileDialog()
{
  QFileDialog *fd = new QFileDialog(this->window());
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setDirectory(QDir::homePath());
  fd->setFileMode(QFileDialog::ExistingFile);
  fd->setOption(QFileDialog::ReadOnly);

  connect(fd, &QFileDialog::fileSelected, this,
          &FileTabWidget::openFileFunction);

  fd->show();
}

void
FileTabWidget::openFileFunction(const QString &filename)
{
  source_file->setText(filename);

  if(result_file->text().isEmpty())
    {
      std::filesystem::path p
          = std::filesystem::u8path(filename.toStdString());
      std::string str = p.stem().u8string();
      str += "_new" + p.extension().u8string();
      p = p.parent_path() / std::filesystem::u8path(str);
      result_file->setText(p.u8string().c_str());
    }
}

void
FileTabWidget::saveFileDialog()
{
  QFileDialog *fd = new QFileDialog(this->window());
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setAcceptMode(QFileDialog::AcceptSave);
  fd->setDirectory(QDir::homePath());

  connect(fd, &QFileDialog::fileSelected, this,
          &FileTabWidget::saveFileFunction);

  fd->show();
}

void
FileTabWidget::saveFileFunction(const QString &filename)
{
  result_file->setText(filename);
}

void
FileTabWidget::encryptFile()
{
  std::filesystem::path source
      = std::filesystem::u8path(source_file->text().toStdString());
  if(!std::filesystem::exists(source))
    {
      return void();
    }

  std::filesystem::path result
      = std::filesystem::u8path(result_file->text().toStdString());
  if(result.empty())
    {
      return void();
    }

  NonCloseMsg *msg
      = new NonCloseMsg(this->window(), tr("Operation in progress..."));
  msg->show();

  std::thread thr(
      [this, msg, source, result]
        {
          try
            {
              std::tuple<std::string, std::string> pass_tup
                  = spy->genUsernamePasswordEncryption(key_pair, other_key);

              spy->encryptFile(source, result, std::get<0>(pass_tup),
                               std::get<1>(pass_tup));
            }
          catch(std::exception &er)
            {
#ifndef __ANDROID__
              std::cout << "FileTabWidget::encryptFile: \"" << er.what()
                        << "\"" << std::endl;
#else
              __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                                  "FileTabWidget::encryptFile: \"%s\"",
                                  er.what());
#endif
              emit signalMessage(msg, MessageType::Error);
              return void();
            }
          emit signalMessage(msg, MessageType::Encrypted);
        });
  thr.detach();
}

void
FileTabWidget::decryptFile()
{
  std::filesystem::path source
      = std::filesystem::u8path(source_file->text().toStdString());
  if(!std::filesystem::exists(source))
    {
      return void();
    }

  std::filesystem::path result
      = std::filesystem::u8path(result_file->text().toStdString());
  if(result.empty())
    {
      return void();
    }

  NonCloseMsg *msg
      = new NonCloseMsg(this->window(), tr("Operation in progress..."));
  msg->show();

  std::thread thr(
      [this, msg, source, result]
        {
          try
            {
              std::tuple<std::string, std::string> pass_tup
                  = spy->genUsernamePasswordDecryption(key_pair, other_key);

              spy->decryptFile(source, result, std::get<0>(pass_tup),
                               std::get<1>(pass_tup));
            }
          catch(std::exception &er)
            {
#ifndef __ANDROID__
              std::cout << "FileTabWidget::decryptFile: \"" << er.what()
                        << "\"" << std::endl;
#else
              __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                                  "FileTabWidget::decryptFile: \"%s\"",
                                  er.what());
#endif
              emit signalMessage(msg, MessageType::Error);
              return void();
            }
          emit signalMessage(msg, MessageType::Decrypted);
        });
  thr.detach();
}

void
FileTabWidget::infoMessage(QWidget *progr_dialog, const MessageType &type)
{
  NonCloseMsg *ncm = dynamic_cast<NonCloseMsg *>(progr_dialog);
  if(ncm)
    {
      ncm->allowClose(true);
      ncm->close();
    }

  QMessageBox *msg = new QMessageBox(this->window());
  msg->setAttribute(Qt::WA_DeleteOnClose);
  msg->setWindowModality(Qt::WindowModal);

  switch(type)
    {
    case MessageType::Decrypted:
      {
        msg->setText(tr("File has been successfully decrypted"));
        msg->setIcon(QMessageBox::Information);
        break;
      }
    case MessageType::Encrypted:
      {
        msg->setText(tr("File has been successfully encrypted"));
        msg->setIcon(QMessageBox::Information);
        break;
      }
    case MessageType::Error:
      {
        msg->setText(tr("Error!"));
        msg->setIcon(QMessageBox::Critical);
        if(msg->button(QMessageBox::Close) == nullptr)
          {
            msg->addButton(QMessageBox::Close);
          }
        break;
      }
    default:
      break;
    }
  msg->show();
}
