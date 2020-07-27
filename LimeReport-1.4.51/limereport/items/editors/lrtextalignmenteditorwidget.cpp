/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "lrtextalignmenteditorwidget.h"

namespace LimeReport {

TextAlignmentEditorWidget::TextAlignmentEditorWidget(ReportDesignWidget *reportEditor, const QString &title, QWidget *parent)
    : ItemEditorWidget(reportEditor, title, parent), m_textAttibutesIsChanging(false) {
  initEditor();
}

TextAlignmentEditorWidget::TextAlignmentEditorWidget(ReportDesignWidget *reportEditor, QWidget *parent) : ItemEditorWidget(reportEditor, parent), m_textAttibutesIsChanging(false) { initEditor(); }

TextAlignmentEditorWidget::TextAlignmentEditorWidget(PageDesignIntf *page, const QString &title, QWidget *parent) : ItemEditorWidget(page, title, parent), m_textAttibutesIsChanging(false) {
  initEditor();
}

TextAlignmentEditorWidget::TextAlignmentEditorWidget(PageDesignIntf *page, QWidget *parent) : ItemEditorWidget(page, parent), m_textAttibutesIsChanging(false) { initEditor(); }

void TextAlignmentEditorWidget::setItemEvent(BaseDesignIntf *item) {
  QVariant align = item->property("alignment");
  if (align.isValid()) {
    updateValues(Qt::Alignment(align.value<int>()));
    setEnabled(true);
  }
}

void TextAlignmentEditorWidget::initEditor() {
  m_textAliginLeft = new QAction(tr("Text align left"), this);
  m_textAliginLeft->setIcon(QIcon(":/report/images/textAlignHLeft"));
  m_textAliginLeft->setCheckable(true);
  connect(m_textAliginLeft, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextHAttribsChanged);
  addAction(m_textAliginLeft);

  m_textAliginHCenter = new QAction(tr("Text align center"), this);
  m_textAliginHCenter->setIcon(QIcon(":/report/images/textAlignHCenter"));
  m_textAliginHCenter->setCheckable(true);
  connect(m_textAliginHCenter, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextHAttribsChanged);
  addAction(m_textAliginHCenter);

  m_textAliginRight = new QAction(tr("Text align right"), this);
  m_textAliginRight->setIcon(QIcon(":/report/images/textAlignHRight"));
  m_textAliginRight->setCheckable(true);
  connect(m_textAliginRight, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextHAttribsChanged);
  addAction(m_textAliginRight);

  m_textAliginJustify = new QAction(tr("Text align justify"), this);
  m_textAliginJustify->setIcon(QIcon(":/report/images/textAlignHJustify"));
  m_textAliginJustify->setCheckable(true);
  connect(m_textAliginJustify, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextHAttribsChanged);
  addAction(m_textAliginJustify);

  addSeparator();

  m_textAliginTop = new QAction(tr("Text align top"), this);
  m_textAliginTop->setIcon(QIcon(":/report/images/textAlignVTop"));
  m_textAliginTop->setCheckable(true);
  connect(m_textAliginTop, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextVAttribsChanged);
  addAction(m_textAliginTop);

  m_textAliginVCenter = new QAction(tr("Text align center"), this);
  m_textAliginVCenter->setIcon(QIcon(":/report/images/textAlignVCenter"));
  m_textAliginVCenter->setCheckable(true);
  connect(m_textAliginVCenter, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextVAttribsChanged);
  addAction(m_textAliginVCenter);

  m_textAliginBottom = new QAction(tr("Text align bottom"), this);
  m_textAliginBottom->setIcon(QIcon(":/report/images/textAlignVBottom"));
  m_textAliginBottom->setCheckable(true);
  connect(m_textAliginBottom, &QAction::toggled, this, &TextAlignmentEditorWidget::slotTextVAttribsChanged);
  addAction(m_textAliginBottom);

  if (reportEditor()) {
    connect(reportEditor(), &ReportDesignWidget::itemPropertyChanged, this, &TextAlignmentEditorWidget::slotPropertyChanged);
  }
  if (page()) {
    connect(page(), &PageDesignIntf::itemPropertyChanged, this, &TextAlignmentEditorWidget::slotPropertyChanged);
  }
  setEnabled(false);
}

void TextAlignmentEditorWidget::updateValues(const Qt::Alignment &align) {
  m_textAttibutesIsChanging = true;
  m_textAliginLeft->setChecked((align & Qt::AlignLeft) == Qt::AlignLeft);
  m_textAliginRight->setChecked((align & Qt::AlignRight) == Qt::AlignRight);
  m_textAliginHCenter->setChecked((align & Qt::AlignHCenter) == Qt::AlignHCenter);
  m_textAliginJustify->setChecked((align & Qt::AlignJustify) == Qt::AlignJustify);
  m_textAliginTop->setChecked((align & Qt::AlignTop) == Qt::AlignTop);
  m_textAliginVCenter->setChecked((align & Qt::AlignVCenter) == Qt::AlignVCenter);
  m_textAliginBottom->setChecked((align & Qt::AlignBottom) == Qt::AlignBottom);
  m_textAttibutesIsChanging = false;
}

Qt::Alignment TextAlignmentEditorWidget::createAlignment() {
  Qt::Alignment align = {};
  if (m_textAliginLeft->isChecked()) align |= Qt::AlignLeft;
  if (m_textAliginHCenter->isChecked()) align |= Qt::AlignHCenter;
  if (m_textAliginRight->isChecked()) align |= Qt::AlignRight;
  if (m_textAliginJustify->isChecked()) align |= Qt::AlignJustify;
  if (m_textAliginTop->isChecked()) align |= Qt::AlignTop;
  if (m_textAliginVCenter->isChecked()) align |= Qt::AlignVCenter;
  if (m_textAliginBottom->isChecked()) align |= Qt::AlignBottom;
  return align;
}

void TextAlignmentEditorWidget::slotTextHAttribsChanged(bool) {
  if (m_textAttibutesIsChanging) return;
  m_textAttibutesIsChanging = true;

  m_textAliginLeft->setChecked(sender() == m_textAliginLeft);
  m_textAliginHCenter->setChecked(sender() == m_textAliginHCenter);
  m_textAliginRight->setChecked(sender() == m_textAliginRight);
  m_textAliginJustify->setChecked(sender() == m_textAliginJustify);

  int flag = 0;
  if (sender() == m_textAliginLeft) flag |= Qt::AlignLeft;
  if (sender() == m_textAliginHCenter) flag |= Qt::AlignHCenter;
  if (sender() == m_textAliginRight) flag |= Qt::AlignRight;
  if (sender() == m_textAliginJustify) flag |= Qt::AlignJustify;

  if (reportEditor()) reportEditor()->setTextAlign(true, Qt::AlignmentFlag(flag));
  if (page()) {
    // page()->setTextAlign(createAlignment());
    page()->changeSelectedGrpoupTextAlignPropperty(true, Qt::AlignmentFlag(flag));
  }
  m_textAttibutesIsChanging = false;
}

void TextAlignmentEditorWidget::slotTextVAttribsChanged(bool) {
  if (m_textAttibutesIsChanging) return;
  m_textAttibutesIsChanging = true;

  m_textAliginTop->setChecked(sender() == m_textAliginTop);
  m_textAliginVCenter->setChecked(sender() == m_textAliginVCenter);
  m_textAliginBottom->setChecked(sender() == m_textAliginBottom);

  int flag = 0;
  if (sender() == m_textAliginTop) flag |= Qt::AlignTop;
  if (sender() == m_textAliginVCenter) flag |= Qt::AlignVCenter;
  if (sender() == m_textAliginBottom) flag |= Qt::AlignBottom;

  if (reportEditor()) reportEditor()->setTextAlign(false, Qt::AlignmentFlag(flag));
  if (page()) page()->changeSelectedGrpoupTextAlignPropperty(false, Qt::AlignmentFlag(flag));
  m_textAttibutesIsChanging = false;
}

void TextAlignmentEditorWidget::slotPropertyChanged(const QString &objectName, const QString &property, const QVariant &oldValue, const QVariant &newValue) {
  Q_UNUSED(oldValue)
  Q_UNUSED(newValue)

  if (item() && (item()->objectName() == objectName) && (property == "alignment")) {
    updateValues(Qt::Alignment(item()->property("alignment").value<int>()));
  }
}

} // namespace LimeReport
