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

#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <TextTabWidget.h>
#include <filesystem>
#include <fstream>

#ifndef __ANDROID__
#include <iostream>
#else
#include <android/log.h>
#define APPNAME "Stirlitz"
#endif

TextTabWidget::TextTabWidget(QWidget *parent, Stirlitz *spy,
                             const std::shared_ptr<gcry_sexp> &key_pair,
                             const std::shared_ptr<gcry_sexp> &other_key)
    : QWidget(parent)
{
  this->spy = spy;
  this->key_pair = key_pair;
  this->other_key = other_key;
  createWidget();
}

void
TextTabWidget::resetKeys(const std::shared_ptr<gcry_sexp> &key_pair,
                         const std::shared_ptr<gcry_sexp> &other_key)
{
  this->key_pair = key_pair;
  this->other_key = other_key;
}

void
TextTabWidget::createWidget()
{
  QVBoxLayout *v_box = new QVBoxLayout;
  this->setLayout(v_box);

  source_text = new QPlainTextEdit;
  source_text->setPlaceholderText(
      tr("Text to be encrypted or text to be decrypted"));
  v_box->addWidget(source_text);

  QHBoxLayout *h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *encrypt = new QPushButton;
  encrypt->setText(tr("Encrypt"));
  connect(encrypt, &QPushButton::clicked, this, &TextTabWidget::encryptText);
  h_box->addWidget(encrypt, 0, Qt::AlignCenter);

  QPushButton *decrypt = new QPushButton;
  decrypt->setText(tr("Decrypt"));
  connect(decrypt, &QPushButton::clicked, this, &TextTabWidget::decryptText);
  h_box->addWidget(decrypt, 0, Qt::AlignCenter);

#ifdef __ANDROID__
  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);
#endif

  QPushButton *paste = new QPushButton;
  paste->setText(tr("Paste"));
  connect(paste, &QPushButton::clicked, source_text,
          [this]
            {
              QClipboard *cpb = QGuiApplication::clipboard();
              source_text->setPlainText(cpb->text());
            });
  h_box->addWidget(paste, 0, Qt::AlignCenter);

  QPushButton *clear = new QPushButton;
  clear->setText(tr("Clear"));
  connect(clear, &QPushButton::clicked, source_text,
          [this]
            {
              source_text->setPlainText("");
            });
  h_box->addWidget(clear, 0, Qt::AlignCenter);

  QPushButton *load = new QPushButton;
  load->setText(tr("Load from file"));
  connect(load, &QPushButton::clicked, this, &TextTabWidget::openFileDialog);
  h_box->addWidget(load, 0, Qt::AlignCenter);

#ifndef __ANDROID__
  h_box->addStretch();
#endif

  encrypted_text = new QPlainTextEdit;
  encrypted_text->setPlaceholderText(tr("Result"));
  encrypted_text->setReadOnly(true);
  v_box->addWidget(encrypted_text);

  h_box = new QHBoxLayout;
  v_box->addLayout(h_box);

  QPushButton *copy_result = new QPushButton;
  copy_result->setText(tr("Copy result"));
  connect(copy_result, &QPushButton::clicked, encrypted_text,
          [this]
            {
              QClipboard *cpb = QGuiApplication::clipboard();
              cpb->setText(encrypted_text->document()->toPlainText());
            });
  h_box->addWidget(copy_result, 0, Qt::AlignCenter);

  QPushButton *save_result = new QPushButton;
  save_result->setText(tr("Save result"));
  connect(save_result, &QPushButton::clicked, this,
          &TextTabWidget::saveFileDialog);
  h_box->addWidget(save_result, 0, Qt::AlignCenter);

  h_box->addStretch();
}

void
TextTabWidget::encryptText()
{
  QString source = source_text->document()->toRawText();
  if(source.isEmpty())
    {
      return void();
    }
  try
    {
      std::tuple<std::string, std::string> pass_tup
          = spy->genUsernamePasswordEncryption(key_pair, other_key);
      std::string enc = spy->encryptData(
          std::get<0>(pass_tup), std::get<1>(pass_tup), source.toStdString());

      enc = spy->toHex(enc);

      encrypted_text->setPlainText(enc.c_str());
    }
  catch(std::exception &er)
    {
#ifndef __ANDROID__
      std::cout << "TextTabWidget::encryptText: \"" << er.what() << "\""
                << std::endl;
#else
      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                          "TextTabWidget::encryptText: \"%s\"", er.what());
#endif
    }
}

void
TextTabWidget::decryptText()
{
  QString source = source_text->document()->toRawText();
  if(source.isEmpty())
    {
      return void();
    }

  std::string enc = spy->fromHex(source.toStdString());
  try
    {
      std::tuple<std::string, std::string> pass_tup
          = spy->genUsernamePasswordDecryption(key_pair, other_key);
      enc = spy->decryptData(std::get<0>(pass_tup), std::get<1>(pass_tup),
                             enc);

      encrypted_text->setPlainText(enc.c_str());
    }
  catch(std::exception &er)
    {
#ifndef __ANDROID__
      std::cout << "TextTabWidget::decryptText: \"" << er.what() << "\""
                << std::endl;
#else
      __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,
                          "TextTabWidget::decryptText: \"%s\"", er.what());
#endif
    }
}

void
TextTabWidget::saveFileDialog()
{
  if(!encrypted_text->toPlainText().isEmpty())
    {
      QFileDialog *fd = new QFileDialog(this->window());
      fd->setAttribute(Qt::WA_DeleteOnClose);
      fd->setWindowModality(Qt::WindowModal);

      fd->setDirectory(QDir::homePath());
      fd->setAcceptMode(QFileDialog::AcceptSave);

      connect(fd, &QFileDialog::fileSelected, this,
              &TextTabWidget::saveFunction);

      fd->show();
    }
}

void
TextTabWidget::saveFunction(const QString &filename)
{
  std::string buf = encrypted_text->toPlainText().toStdString();
  if(!buf.empty())
    {
      std::filesystem::path p
          = std::filesystem::u8path(filename.toStdString());
      std::filesystem::remove_all(p);

      std::fstream f;
      f.open(p, std::ios_base::out | std::ios_base::binary);
      if(f.is_open())
        {
          f.write(buf.c_str(), buf.size());
          f.close();
        }
    }
}

void
TextTabWidget::openFileDialog()
{
  QFileDialog *fd = new QFileDialog(this->window());
  fd->setAttribute(Qt::WA_DeleteOnClose);
  fd->setWindowModality(Qt::WindowModal);

  fd->setDirectory(QDir::homePath());
  fd->setAcceptMode(QFileDialog::AcceptOpen);
  fd->setFileMode(QFileDialog::ExistingFile);
  fd->setOption(QFileDialog::ReadOnly, true);

  connect(fd, &QFileDialog::fileSelected, this, &TextTabWidget::openFunction);

  fd->show();
}

void
TextTabWidget::openFunction(const QString &filename)
{
  std::filesystem::path p = std::filesystem::u8path(filename.toStdString());
  std::fstream f;
  f.open(p, std::ios_base::in | std::ios_base::binary);
  if(f.is_open())
    {
      std::string buf;
      f.seekg(0, std::ios_base::end);
      buf.resize(f.tellg());
      f.seekg(0, std::ios_base::beg);
      f.read(buf.data(), buf.size());
      f.close();

      source_text->setPlainText(buf.c_str());
    }
}
