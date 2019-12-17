#pragma once

#include <QAbstractProxyModel>
#include <QSqlQueryModel>

class SqlQueryModel final : public QSqlQueryModel {
  Q_OBJECT

public:
  explicit SqlQueryModel(QObject *parent = nullptr);
  auto data(const int row, const int column) const -> QVariant;
  auto data(const QModelIndex &index, const QString &column) const -> QVariant;
  auto data(const int row, const QString &column) const -> QVariant;
  auto setHeaderData(const QString &column, const QVariant &value) -> bool;
  auto setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase()) -> bool;

  QAbstractProxyModel *proxyModel = nullptr;

private:
  using QSqlQueryModel::data;
  using QSqlQueryModel::setHeaderData;
  using QSqlQueryModel::setQuery;
};
