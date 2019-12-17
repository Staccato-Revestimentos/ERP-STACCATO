#include <QDebug>

#include "application.h"
#include "treeview.h"

TreeView::TreeView(QWidget *parent) : QTreeView(parent) {
  connect(this, &QTreeView::expanded, this, &TreeView::resizeAllColumns);
  connect(this, &QTreeView::collapsed, this, &TreeView::resizeAllColumns);
}

void TreeView::hideColumn(const QString &column) { QTreeView::hideColumn(columnIndex(column)); }

void TreeView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) { QTreeView::setItemDelegateForColumn(columnIndex(column), delegate); }

void TreeView::setModel(QAbstractItemModel *model) {
  if (auto temp = qobject_cast<SqlTreeModel *>(model); temp and temp->proxyModel) {
    QTreeView::setModel(temp->proxyModel);
  } else {
    QTreeView::setModel(model);
  }

  baseModel = qobject_cast<SqlTreeModel *>(model);

  if (not baseModel) { qApp->enqueueError("Sem baseModel!"); }

  resizeAllColumns();
}

void TreeView::resizeAllColumns() {
  for (int col = 0; col < model()->columnCount(); ++col) { resizeColumnToContents(col); }
}

int TreeView::columnIndex(const QString &column) const {
  int columnIndex = -1;

  if (baseModel) { columnIndex = baseModel->fieldIndex(column); }

  if (columnIndex == -1 and column != "created" and column != "lastUpdated") { qApp->enqueueError("Coluna '" + column + "' não encontrada!"); }

  return columnIndex;
}

// TODO: set stylesheet for coloring subrows
// TODO: verify that last column with a number
