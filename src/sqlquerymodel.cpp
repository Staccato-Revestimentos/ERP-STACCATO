#include <QSqlError>
#include <QSqlRecord>

#include "application.h"
#include "sqlquerymodel.h"

SqlQueryModel::SqlQueryModel(QObject *parent) : QSqlQueryModel(parent) {}

QVariant SqlQueryModel::data(const int row, const int column) const {
  if (row == -1 or column == -1) { return qApp->enqueueError(false, "Erro: linha/coluna -1 SqlQueryModel"); }

  if (proxyModel) { return proxyModel->data(proxyModel->index(row, column)); }

  return QSqlQueryModel::data(QSqlQueryModel::index(row, column));
}

QVariant SqlQueryModel::data(const QModelIndex &index, const QString &column) const { return data(index.row(), column); }

QVariant SqlQueryModel::data(const int row, const QString &column) const { return data(row, QSqlQueryModel::record().indexOf(column)); }

bool SqlQueryModel::setHeaderData(const QString &column, const QVariant &value) {
  const int index = QSqlQueryModel::record().indexOf(column);

  if (index == -1) { return qApp->enqueueError(false, "Coluna '" + column + "' não encontrada na tabela!"); }

  return QSqlQueryModel::setHeaderData(index, Qt::Horizontal, value);
}

bool SqlQueryModel::setQuery(const QString &query, const QSqlDatabase &db) {
  // TODO: redo places that use this function

  QSqlQueryModel::setQuery(query, db);

  if (lastError().isValid()) { return qApp->enqueueError(false, "Erro lendo dados: " + lastError().text()); }

  return true;
}
