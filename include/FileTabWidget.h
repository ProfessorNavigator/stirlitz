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
#ifndef FILETABWIDGET_H
#define FILETABWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <Stirlitz.h>

class FileTabWidget : public QWidget
{
  Q_OBJECT
public:
  FileTabWidget(QWidget *parent, Stirlitz *spy,
                const std::shared_ptr<gcry_sexp> &key_pair,
                const std::shared_ptr<gcry_sexp> &other_key);

  void
  resetKeys(const std::shared_ptr<gcry_sexp> &key_pair,
            const std::shared_ptr<gcry_sexp> &other_key);

private:
  void
  createWidget();

  void
  openFileDialog();

  void
  openFileFunction(const QString &filename);

  void
  saveFileDialog();

  void
  saveFileFunction(const QString &filename);

  void
  encryptFile();

  void
  decryptFile();

  enum MessageType
  {
    Decrypted,
    Encrypted,
    Error
  };

  void
  infoMessage(QWidget *progr_dialog, const MessageType &type);

  Stirlitz *spy;
  std::shared_ptr<gcry_sexp> key_pair;
  std::shared_ptr<gcry_sexp> other_key;

  QLineEdit *source_file;
  QLineEdit *result_file;

signals:
  void
  signalMessage(QWidget *progr_dialog, const MessageType &type);
};

#endif // FILETABWIDGET_H
