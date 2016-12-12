#include <QBrush>

#include "estoqueproxymodel.h"

EstoqueProxyModel::EstoqueProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), quantUpd(model->fieldIndex("quantUpd")) {
  setSourceModel(model);
}

EstoqueProxyModel::~EstoqueProxyModel() {}

QVariant EstoqueProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    const int quantUpd = QIdentityProxyModel::data(index(proxyIndex.row(), this->quantUpd), Qt::DisplayRole).toInt();

    if (quantUpd == 1) return QBrush(Qt::green);         // Ok
    if (quantUpd == 2) return QBrush(Qt::yellow);        // Quant difere
    if (quantUpd == 3) return QBrush(Qt::red);           // Não encontrado
    if (quantUpd == 4) return QBrush(QColor(0, 190, 0)); // Consumo
    if (quantUpd == 5) return QBrush(Qt::cyan);          // Devolução
  }

  if (role == Qt::ForegroundRole) return QBrush(Qt::black);

  return QIdentityProxyModel::data(proxyIndex, role);
}
