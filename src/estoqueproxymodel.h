#pragma once

#include "sortfilterproxymodel.h"
#include "sqltablemodel.h"

class EstoqueProxyModel final : public SortFilterProxyModel {

public:
  explicit EstoqueProxyModel(SqlTableModel *model, QObject *parent = nullptr);
  ~EstoqueProxyModel() final = default;
  auto data(const QModelIndex &proxyIndex, const int role) const -> QVariant final;

private:
  const int quantUpdColumn = -1;
  enum class Status { Ok = 1, QuantDifere = 2, NaoEncontrado = 3, Consumo = 4, Devolucao = 5 };
};
