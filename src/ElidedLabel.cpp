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

#include <ElidedLabel.h>

ElidedLabel::ElidedLabel(const QString &full_text,
                         const Qt::TextElideMode &elide_mode)
{
  this->full_text = full_text;
  this->elide_mode = elide_mode;
  this->setAlignment(Qt::Alignment());
}

QString
ElidedLabel::getFullText()
{
  return full_text;
}

void
ElidedLabel::setFullText(const QString &text)
{
  full_text = text;
  QString elided
      = this->fontMetrics().elidedText(full_text, elide_mode, this->width());
  this->setText(elided);
}

Qt::TextElideMode
ElidedLabel::getElideMode()
{
  return elide_mode;
}

void
ElidedLabel::resizeEvent(QResizeEvent *event)
{
  QSize sz = event->size();
  QString elided
      = this->fontMetrics().elidedText(full_text, elide_mode, sz.width());
  this->setText(elided);
}

void
ElidedLabel::showEvent(QShowEvent *event)
{
  this->setMinimumWidth(this->size().width());
}
