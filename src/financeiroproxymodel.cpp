#include <QApplication>
#include <QBrush>
#include <QDate>
#include <QDebug>
#include <QStyle>

#include "financeiroproxymodel.h"

FinanceiroProxyModel::FinanceiroProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), statusFinanceiro(model->fieldIndex("statusFinanceiro")), prazoEntrega(model->fieldIndex("prazoEntrega")), novoPrazoEntrega(model->fieldIndex("novoPrazoEntrega")) {
  setSourceModel(model);
}

QVariant FinanceiroProxyModel::data(const QModelIndex &proxyIndex, int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == this->statusFinanceiro) {
      const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->statusFinanceiro), Qt::DisplayRole).toString();

      if (status == "PENDENTE") return QBrush(Qt::red);
      if (status == "CONFERIDO") return QBrush(Qt::yellow);
      if (status == "LIBERADO") return QBrush(Qt::green);
    }

    if (proxyIndex.column() == this->prazoEntrega) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->prazoEntrega), Qt::DisplayRole).toDate();

      // TODO: 0se estiver a 5 dias pintar de amarelo
      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::red);
    }

    if (proxyIndex.column() == this->novoPrazoEntrega) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->novoPrazoEntrega), Qt::DisplayRole).toDate();

      // TODO: 0se estiver a 5 dias pintar de amarelo
      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::red);
    }
  }

  if (role == Qt::ForegroundRole) {
    if (proxyIndex.column() == this->statusFinanceiro) {
      const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->statusFinanceiro), Qt::DisplayRole).toString();

      if (status == "PENDENTE") return QBrush(Qt::black);
      if (status == "CONFERIDO") return QBrush(Qt::black);
      if (status == "LIBERADO") return QBrush(Qt::black);
    }

    if (proxyIndex.column() == this->prazoEntrega) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->prazoEntrega), Qt::DisplayRole).toDate();

      // TODO: 0se estiver a 5 dias pintar de amarelo
      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::black);
    }

    if (proxyIndex.column() == this->novoPrazoEntrega) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->novoPrazoEntrega), Qt::DisplayRole).toDate();

      // TODO: 0se estiver a 5 dias pintar de amarelo
      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::black);
    }

    return qApp->style()->objectName() == "fusion" ? QBrush(Qt::black) : QBrush(Qt::white);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
