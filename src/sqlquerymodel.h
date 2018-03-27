#ifndef SQLQUERYMODEL_H
#define SQLQUERYMODEL_H

#include <QSqlQueryModel>

class SqlQueryModel final : public QSqlQueryModel {
  Q_OBJECT

public:
  explicit SqlQueryModel(QObject *parent = nullptr);
  auto data(const int row, const QString &column) const -> QVariant;
  auto setHeaderData(const QString &column, const QVariant &value) -> bool;
  virtual auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant override;

private:
  using QSqlQueryModel::data;
  using QSqlQueryModel::setHeaderData;
};

#endif // SQLQUERYMODEL_H
