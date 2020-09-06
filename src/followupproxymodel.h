#pragma once

#include <QIdentityProxyModel>
#include <QSqlQueryModel>

class FollowUpProxyModel final : public QIdentityProxyModel {

public:
  explicit FollowUpProxyModel(QSqlQueryModel *model, QObject *parent);
  ~FollowUpProxyModel() final = default;
  auto data(const QModelIndex &proxyIndex, int role) const -> QVariant final;

private:
  const int semaforoColumn;
  enum class FieldColors { Quente = 1, Morno = 2, Frio = 3 };
};
