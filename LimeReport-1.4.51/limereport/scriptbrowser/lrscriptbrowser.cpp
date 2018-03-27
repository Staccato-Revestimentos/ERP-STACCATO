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
#include "lrscriptbrowser.h"
#include "ui_lrscriptbrowser.h"

#ifdef HAVE_UI_LOADER
#include <QFileDialog>
#include <QUiLoader>
#endif
#include <QMessageBox>

namespace LimeReport {

ScriptBrowser::ScriptBrowser(QWidget *parent) : QWidget(parent), ui(new Ui::ScriptBrowser) {
  ui->setupUi(this);
#ifndef HAVE_UI_LOADER
  ui->tpDialogs->setVisible(false);
  ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tpDialogs));
#endif
}

ScriptBrowser::~ScriptBrowser() { delete ui; }

void ScriptBrowser::setReportEditor(ReportDesignWidget *report) {
  m_report = report;
  connect(m_report, &ReportDesignWidget::cleared, this, &ScriptBrowser::slotClear);
  connect(m_report, &ReportDesignWidget::loaded, this, &ScriptBrowser::slotUpdate);
  updateFunctionTree();
}

void ScriptBrowser::updateFunctionTree() {
  ui->twFunctions->clear();
  ScriptEngineManager *sm = reportEditor()->scriptManager();
  QMap<QString, QTreeWidgetItem *> categ;
  for (ScriptFunctionDesc fd : sm->functionsDescriber()) {
    QString functionCategory = (fd.category != "") ? fd.category : tr("NO CATEGORY");
    if (categ.contains(functionCategory)) {
      QTreeWidgetItem *item = new QTreeWidgetItem(categ.value(fd.category), QStringList(fd.name));
      item->setIcon(0, QIcon(":/report/images/function"));
    } else {
      QTreeWidgetItem *categItem = new QTreeWidgetItem(ui->twFunctions, QStringList(functionCategory));
      categItem->setIcon(0, QIcon(":/report/images/folder"));
      categ.insert(functionCategory, categItem);
      QTreeWidgetItem *item = new QTreeWidgetItem(categItem, QStringList(fd.name));
      item->setIcon(0, QIcon(":/report/images/function"));
    }
  }
}

#ifdef HAVE_UI_LOADER
void ScriptBrowser::fillProperties(QTreeWidgetItem *objectItem, QObject *item) {
  for (int i = 0; i < item->metaObject()->propertyCount(); ++i) {
    QStringList row;
    row << item->metaObject()->property(i).typeName() << item->metaObject()->property(i).name();
    /*QTreeWidgetItem* propItem = */ new QTreeWidgetItem(objectItem, row);
  }
}

void ScriptBrowser::fillDialog(QTreeWidgetItem *dialogItem, const QString &description) {

  QUiLoader loader;
  QByteArray baDesc = description.toUtf8();
  QBuffer buff(&baDesc);
  buff.open(QIODevice::ReadOnly);
  QDialog *dialog = dynamic_cast<QDialog *>(loader.load(&buff));
  if (dialog) {
    for (QObject *child : dialog->children()) {
      if (!child->objectName().isEmpty()) {
        QStringList row;
        row << child->metaObject()->className() << child->objectName();
        QTreeWidgetItem *item = new QTreeWidgetItem(dialogItem, row);
        item->setIcon(0, QIcon(":/scriptbrowser/images/item"));
        fillProperties(item, child);
      }
    }
    delete dialog;
  }
}

void ScriptBrowser::updateDialogsTree() {
  ui->twDialogs->clear();
  ScriptEngineContext *sc = reportEditor()->scriptContext();
  for (DialogDescriber::Ptr dc : sc->dialogsDescriber()) {
    QTreeWidgetItem *dialogItem = new QTreeWidgetItem(ui->twDialogs, QStringList(dc->name()));
    dialogItem->setIcon(0, QIcon(":/scriptbrowser/images/dialog"));
    fillDialog(dialogItem, dc->description());
  }
}
#endif
void ScriptBrowser::slotClear() {
  ui->twDialogs->clear();
  ui->twFunctions->clear();
}

void ScriptBrowser::slotUpdate() {
#ifdef HAVE_UI_LOADER
  updateDialogsTree();
#endif
  updateFunctionTree();
}

#ifdef HAVE_UI_LOADER
void ScriptBrowser::on_tbAddDialog_clicked() {
  QFileDialog fileDialog(this);
  if (fileDialog.exec() == QDialog::Accepted) {
    QStringList fileNames = fileDialog.selectedFiles();
    QUiLoader loader;

    if (!fileNames.isEmpty()) {
      for (QString fileName : fileNames) {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        if (file.isOpen()) {
          QWidget *widget = loader.load(&file);
          QDialog *dialog = dynamic_cast<QDialog *>(widget);
          if (dialog) {
            if (!m_report->scriptContext()->containsDialog(dialog->objectName())) {
              file.seek(0);
              m_report->scriptContext()->addDialog(dialog->objectName(), file.readAll());
              updateDialogsTree();
            } else {
              QMessageBox::critical(this, tr("Error"), tr("Dialog with name: %1 already exists").arg(dialog->objectName()));
            }
          } else {
            if (widget)
              QMessageBox::critical(this, tr("Error"), tr("ui file must cointain QDialog instead QWidget or QMainWindow"));
            else
              QMessageBox::critical(this, tr("Error"), tr("wrong file format"));
          }
          if (widget) delete widget;
        }
      }
    }
  }
}

void ScriptBrowser::on_tbRunDialog_clicked() {
  if (ui->twDialogs->currentItem() && ui->twDialogs->currentItem()->parent() == nullptr) {
    m_report->scriptContext()->previewDialog(ui->twDialogs->currentItem()->text(0));
  }
}

void ScriptBrowser::on_tbDeleteDialog_clicked() {
  if (ui->twDialogs->currentItem() && ui->twDialogs->currentItem()->parent() == nullptr) {
    m_report->scriptContext()->deleteDialog(ui->twDialogs->currentItem()->text(0));
    updateDialogsTree();
  }
}

#endif

} // namespace LimeReport
