#include "tableview.h"

#include "application.h"
#include "sqlquerymodel.h"
#include "sqltablemodel.h"

#include <QClipboard>
#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QSqlRecord>

TableView::TableView(QWidget *parent) : QTableView(parent) {
  setContextMenuPolicy(Qt::CustomContextMenu);

  verticalHeader()->setResizeContentsPrecision(0);
  horizontalHeader()->setResizeContentsPrecision(0);

  verticalHeader()->setDefaultSectionSize(20);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  setAlternatingRowColors(true);

  setConnections();
}

void TableView::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(this, &QWidget::customContextMenuRequested, this, &TableView::showContextMenu, connectionType);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TableView::resizeColumnsToContents, connectionType);
}

void TableView::resizeColumnsToContents() {
  if (autoResize) { QTableView::resizeColumnsToContents(); }
}

int TableView::columnCount() const { return model()->columnCount(); }

void TableView::showContextMenu(const QPoint pos) {
  QMenu contextMenu;

  QAction action1("Autodimensionar", this);
  action1.setCheckable(true);
  action1.setChecked(autoResize);
  connect(&action1, &QAction::triggered, this, &TableView::setAutoResize);
  contextMenu.addAction(&action1);

  QAction action2("Copiar cabeçalhos", this);
  action2.setCheckable(true);
  action2.setChecked(copyHeaders);
  connect(&action2, &QAction::triggered, this, &TableView::setCopyHeaders);
  contextMenu.addAction(&action2);

  contextMenu.exec(mapToGlobal(pos));
}

void TableView::resizeEvent(QResizeEvent *event) {
  redoView();

  QTableView::resizeEvent(event);
}

void TableView::setCopyHeaders(const bool newCopyHeaders) { copyHeaders = newCopyHeaders; }

int TableView::columnIndex(const QString &column) const { return columnIndex(column, false); }

int TableView::columnIndex(const QString &column, const bool silent) const {
  int columnIndex = -1;

  if (baseModel) { columnIndex = baseModel->record().indexOf(column); }

  if (columnIndex == -1 and not silent and column != "created" and column != "lastUpdated") { throw RuntimeException("Coluna '" + column + "' não encontrada!"); }

  return columnIndex;
}

void TableView::hideColumn(const QString &column) { QTableView::hideColumn(columnIndex(column)); }

void TableView::showColumn(const QString &column) { QTableView::showColumn(columnIndex(column)); }

void TableView::setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate) { QTableView::setItemDelegateForColumn(columnIndex(column), delegate); }

void TableView::openPersistentEditor(const int row, const QString &column) { QTableView::openPersistentEditor(QTableView::model()->index(row, columnIndex(column))); }

void TableView::resort() { model()->sort(horizontalHeader()->sortIndicatorSection(), horizontalHeader()->sortIndicatorOrder()); }

void TableView::sortByColumn(const QString &column, Qt::SortOrder order) { QTableView::sortByColumn(columnIndex(column), order); }

int TableView::rowCount() const { return model()->rowCount(); }

void TableView::storeSelection() {
  const auto selection = selectionModel()->selectedRows();

  if (selection.isEmpty()) { return; }

  selectedRows.clear();

  for (auto index : selection) { selectedRows << index.row(); }
}

void TableView::restoreSelection() {
  if (selectedRows.isEmpty()) { return; }

  QSignalBlocker blocker(selectionModel());

  for (auto row : selectedRows) { selectRow(row); }
}

void TableView::redoView() {
  if (persistentColumns.isEmpty()) { return; }
  if (rowCount() == 0) { return; }

  const QRect rect = viewport()->rect();
  const int x = rect.x() + 5;
  const int y = rect.y() + 5;
  const int height = rect.height() - 5;

  const int firstRowIndex = indexAt(QPoint(x, y)).row();
  int lastRowIndex = indexAt(QPoint(x, height)).row();

  if (firstRowIndex == -1) { return; }

  int count = 0;

  // subtrair uma linha de altura por vez até achar uma linha
  while (lastRowIndex == -1) {
    int xpos = x;
    int ypos = height - (rowHeight(0) * count);

    if (ypos < 0) { return; }

    lastRowIndex = indexAt(QPoint(xpos, ypos)).row();
    ++count;
  }

  for (int row = firstRowIndex; row <= lastRowIndex; ++row) {
    for (const auto &column : qAsConst(persistentColumns)) { openPersistentEditor(row, column); }
  }
}

void TableView::setModel(QAbstractItemModel *model) {
  if (auto *queryModel = qobject_cast<SqlQueryModel *>(model); queryModel and queryModel->proxyModel) {
    QTableView::setModel(queryModel->proxyModel);
  } else if (auto *tableModel = qobject_cast<SqlTableModel *>(model); tableModel and tableModel->proxyModel) {
    QTableView::setModel(tableModel->proxyModel);
  } else {
    QTableView::setModel(model);
  }

  baseModel = qobject_cast<QSqlQueryModel *>(model);

  if (not baseModel) { throw RuntimeException("TableView model não implementado!", this); }

  //---------------------------------------

  // TODO: reativar
  //  connect(baseModel, &QSqlQueryModel::modelAboutToBeReset, this, [=] {
  //    setDisabled(true);
  //    repaint();
  //  });

  //  connect(baseModel, &QSqlQueryModel::modelReset, this, [=] { setEnabled(true); });

  connect(baseModel, &QSqlQueryModel::modelAboutToBeReset, this, &TableView::storeSelection);
  connect(baseModel, &QSqlQueryModel::modelReset, this, &TableView::restoreSelection);

  connect(baseModel, &QSqlQueryModel::modelReset, this, &TableView::redoView);
  connect(baseModel, &QSqlQueryModel::dataChanged, this, &TableView::redoView);
  connect(baseModel, &QSqlQueryModel::rowsRemoved, this, &TableView::redoView);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &TableView::redoView);

  //---------------------------------------

  hideColumn("created");
  hideColumn("lastUpdated");

  redoView();
}

void TableView::mousePressEvent(QMouseEvent *event) {
  const QModelIndex item = indexAt(event->pos());

  if (not item.isValid()) {
    clearSelection();

    emit clicked(item); // QTableView don't emit when index is invalid, emit manually for widgets
  }

  QTableView::mousePressEvent(event);
}

void TableView::setAutoResize(const bool value) {
  autoResize = value;

  horizontalHeader()->setSectionResizeMode(autoResize ? QHeaderView::ResizeToContents : QHeaderView::Interactive);

  if (autoResize) { resizeColumnsToContents(); }
}

void TableView::setPersistentColumns(const QStringList &value) { persistentColumns = value; }

void TableView::keyPressEvent(QKeyEvent *event) {
  if (event->matches(QKeySequence::Copy)) {
    const auto selection = selectionModel()->selectedIndexes();

    if (selection.isEmpty()) { return; }

    //---------------------------------------

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
      QVariant currentText = selection.first().data();

      if (currentText.userType() == QMetaType::QDateTime) { currentText = currentText.toString().replace("T", " ").replace(".000", ""); }
      if (currentText.userType() == QMetaType::Double) { currentText = QLocale(QLocale::Portuguese).toString(currentText.toDouble(), 'f', 2); }

      text += currentText.toString();
    }

    if (selectionBehavior() == SelectRows) {
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

  QTableView::keyPressEvent(event);
}
