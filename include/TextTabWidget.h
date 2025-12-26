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
#ifndef TEXTTABWIDGET_H
#define TEXTTABWIDGET_H

#include <QPlainTextEdit>
#include <QWidget>
#include <Stirlitz.h>
#include <memory>

class TextTabWidget : public QWidget
{
  Q_OBJECT
public:
  TextTabWidget(QWidget *parent, Stirlitz *spy,
                const std::shared_ptr<gcry_sexp> &key_pair,
                const std::shared_ptr<gcry_sexp> &other_key);

  void
  resetKeys(const std::shared_ptr<gcry_sexp> &key_pair,
            const std::shared_ptr<gcry_sexp> &other_key);

private:
  void
  createWidget();

  void
  encryptText();

  void
  decryptText();

  void
  saveFileDialog();

  void
  saveFunction(const QString &filename);

  void
  openFileDialog();

  void
  openFunction(const QString &filename);

  Stirlitz *spy;
  std::shared_ptr<gcry_sexp> key_pair;
  std::shared_ptr<gcry_sexp> other_key;

  QPlainTextEdit *source_text;
  QPlainTextEdit *encrypted_text;
};

#endif // TEXTTABWIDGET_H
