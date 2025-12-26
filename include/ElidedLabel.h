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
#ifndef ELIDEDLABEL_H
#define ELIDEDLABEL_H

#include <QLabel>
#include <QResizeEvent>
#include <QShowEvent>

class ElidedLabel : public QLabel
{
  Q_OBJECT
public:
  ElidedLabel(const QString &full_text = QString(),
              const Qt::TextElideMode &elide_mode = Qt::ElideRight);

  QString
  getFullText();

  void
  setFullText(const QString &text);

  Qt::TextElideMode
  getElideMode();

protected:
  void
  resizeEvent(QResizeEvent *event) override;

  void
  showEvent(QShowEvent *event) override;

private:
  QString full_text;
  Qt::TextElideMode elide_mode;
};

#endif // ELIDEDLABEL_H
