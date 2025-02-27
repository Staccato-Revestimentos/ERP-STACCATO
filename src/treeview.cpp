#include "treeview.h"

#include "application.h"
#include "user.h"

#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

TreeView::TreeView(QWidget *parent) : QTreeView(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);

  setEditTriggers(NoEditTriggers);
  setAlternatingRowColors(true);
  setUniformRowHeights(true);
  setSortingEnabled(true);

  header()->setResizeContentsPrecision(0);
  header()->setDefaultSectionSize(20);
  header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  setConnections();
}

void TreeView::setConnections() {
  if (not blockingSignals.isEmpty()) { blockingSignals.pop(); } // avoid crashing on first setConnections

  if (not blockingSignals.isEmpty()) { return; } // delay setting connections until last unset/set block

  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(this, &QTreeView::expanded, this, &TreeView::resizeAllColumns, connectionType);
  connect(this, &QTreeView::collapsed, this, &TreeView::resizeAllColumns, connectionType);
  connect(this, &QWidget::customContextMenuRequested, this, &TreeView::showContextMenu, connectionType);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TreeView::resizeAllColumns, connectionType);
}

void TreeView::unsetConnections() {
  blockingSignals.push(0);

  disconnect(this, &QTreeView::expanded, this, &TreeView::resizeAllColumns);
  disconnect(this, &QTreeView::collapsed, this, &TreeView::resizeAllColumns);
  disconnect(this, &QWidget::customContextMenuRequested, this, &TreeView::showContextMenu);
  disconnect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TreeView::resizeAllColumns);
}

void TreeView::showContextMenu(const QPoint pos) {
  QMenu contextMenu;

  QAction actionExpand("Expandir tudo", this);
  connect(&actionExpand, &QAction::triggered, this, &TreeView::expandAll);
  contextMenu.addAction(&actionExpand);

  QAction actionCollapse("Recolher tudo", this);
  connect(&actionCollapse, &QAction::triggered, this, &TreeView::collapseAll);
  contextMenu.addAction(&actionCollapse);

  QAction action1("Autodimensionar", this);
  action1.setCheckable(true);
  action1.setChecked(autoResize);
  connect(&action1, &QAction::triggered, this, &TreeView::setAutoResize);
  contextMenu.addAction(&action1);

  QAction action2("Copiar cabeçalhos", this);
  action2.setCheckable(true);
  action2.setChecked(copyHeaders);
  connect(&action2, &QAction::triggered, this, &TreeView::setCopyHeaders);
  contextMenu.addAction(&action2);

  contextMenu.exec(mapToGlobal(pos));
}

void TreeView::collapseAll() {
  unsetConnections();

  QTreeView::collapseAll();
  resizeAllColumns();

  setConnections();
}

void TreeView::expandAll() {
  unsetConnections();

  QTreeView::expandAll();
  resizeAllColumns();

  setConnections();
}

void TreeView::hideColumn(const QString &column) { QTreeView::hideColumn(columnIndex(column)); }

void TreeView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) { QTreeView::setItemDelegateForColumn(columnIndex(column), delegate); }

void TreeView::setModel(QAbstractItemModel *model) {
  if (auto *temp = qobject_cast<SqlTreeModel *>(model); temp and temp->proxyModel) {
    QTreeView::setModel(temp->proxyModel);
  } else {
    QTreeView::setModel(model);
  }

  baseModel = qobject_cast<SqlTreeModel *>(model);

  if (not baseModel) { throw RuntimeException("Sem baseModel!"); }

  resizeAllColumns();
}

void TreeView::resizeAllColumns() {
  if (autoResize) {
    for (int col = 0; col < model()->columnCount(); ++col) { resizeColumnToContents(col); }
  }
}

int TreeView::columnIndex(const QString &column) const {
  int columnIndex = -1;

  if (baseModel) { columnIndex = baseModel->fieldIndex(column); }

  if (columnIndex == -1 and column != "created" and column != "lastUpdated") { throw RuntimeException("Coluna '" + column + "' não encontrada!"); }

  return columnIndex;
}

void TreeView::setAutoResize(const bool value) {
  autoResize = value;

  header()->setSectionResizeMode(autoResize ? QHeaderView::ResizeToContents : QHeaderView::Interactive);

  if (autoResize) { resizeAllColumns(); }
}

void TreeView::mousePressEvent(QMouseEvent *event) {
  const QModelIndex item = indexAt(event->pos());

  // QTreeView don't emit when index is invalid, emit manually for widgets
  if (not item.isValid()) { clearSelection(); }

  QTreeView::mousePressEvent(event);
}

void TreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const {
  QTreeView::drawRow(painter, options, index);

  const QString tema = User::getSetting("User/tema").toString();

  const QColor color = (tema == "escuro") ? QColor(44, 44, 44) : QColor(200, 200, 200);
  painter->setPen(color);
  int top = options.rect.top();

  painter->save();
  painter->translate(visualRect(model()->index(0, 0)).left() - indentation() - .5, -.5);

  for (int sectionId = 0; sectionId < header()->count(); ++sectionId) {
    painter->translate(header()->sectionSize(sectionId), 0);
    painter->drawLine(0, top, 0, top + options.rect.height());
  }

  painter->restore();

  painter->drawLine(0, options.rect.bottom(), options.rect.width(), options.rect.bottom());
}

void TreeView::keyPressEvent(QKeyEvent *event) {
  if (event->matches(QKeySequence::Copy)) {
    QString headers;

    // dont copy headers if in single cell mode
    if (selectionBehavior() == SelectRows and copyHeaders) {
      for (int col = 0; col < model()->columnCount(); ++col) {
        if (isColumnHidden(col)) { continue; }

        headers += model()->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        headers += '\t';
      }

      headers += '\n';
    }

    //---------------------------------------

    QString text;

    if (selectionBehavior() == SelectItems) {
      const auto selection = selectionModel()->selectedIndexes();

      if (selection.isEmpty()) { return; }

      QVariant currentText = selection.first().data();

      if (currentText.userType() == QMetaType::QDateTime) { currentText = currentText.toString().replace("T", " ").replace(".000", ""); }
      if (currentText.userType() == QMetaType::Double) { currentText = QLocale(QLocale::Portuguese).toString(currentText.toDouble(), 'f', 2); }

      text += currentText.toString();
    }

    if (selectionBehavior() == SelectRows) {
      const auto selection = selectionModel()->selectedRows();

      if (selection.isEmpty()) { return; }

      for (const auto indexRow : selection) {
        for (int col = 0; col < model()->columnCount(); ++col) {
          if (isColumnHidden(col)) { continue; }

          QVariant currentText = indexRow.siblingAtColumn(col).data();

          if (currentText.userType() == QMetaType::QDateTime) { currentText = currentText.toString().replace("T", " ").replace(".000", ""); }
          if (currentText.userType() == QMetaType::Double) { currentText = QLocale(QLocale::Portuguese).toString(currentText.toDouble(), 'f', 2); }

          text += currentText.toString();
          text += '\t';
        }

        text += '\n';
      }
    }

    QApplication::clipboard()->setText(headers + text);
    return;
  }

  QTreeView::keyPressEvent(event);
}

void TreeView::setCopyHeaders(const bool newCopyHeaders) { copyHeaders = newCopyHeaders; }
