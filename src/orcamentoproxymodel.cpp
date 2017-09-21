#include <QApplication>
#include <QBrush>
#include <QStyle>
#include <ciso646>

#include "orcamentoproxymodel.h"

OrcamentoProxyModel::OrcamentoProxyModel(SqlTableModel *model, QObject *parent)
    : QIdentityProxyModel(parent), dias(model->fieldIndex("Dias restantes")), status(model->fieldIndex("status")), followup(model->fieldIndex("Observação")), semaforo(model->fieldIndex("semaforo")) {
  setSourceModel(model);
}

QVariant OrcamentoProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == this->dias) {
      const int dias = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toInt();
      const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();

      if (dias >= 5 or status == "FECHADO") return QBrush(Qt::green);
      if (dias >= 3 or status == "CANCELADO") return QBrush(Qt::yellow);
      if (dias < 3) return QBrush(Qt::red);
    }

    if (proxyIndex.column() == this->followup) {
      const int semaforo = QIdentityProxyModel::data(index(proxyIndex.row(), this->semaforo), Qt::DisplayRole).toInt();

      if (semaforo == Quente) return QBrush(QColor(255, 66, 66));
      if (semaforo == Morno) return QBrush(QColor(255, 170, 0));
      if (semaforo == Frio) return QBrush(QColor(70, 113, 255));
    }

    const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();
    if (status == "FECHADO") return QBrush(Qt::green);
    if (status == "CANCELADO") return QBrush(Qt::yellow);
    if (status == "PERDIDO") return QBrush(Qt::yellow);
  }

  if (role == Qt::ForegroundRole) {
    if (proxyIndex.column() == this->dias) {
      const int dias = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toInt();
      const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();

      if (dias >= 5 or status == "FECHADO") return QBrush(Qt::black);
      if (dias >= 3 or status == "CANCELADO") return QBrush(Qt::black);
      if (dias < 3) return QBrush(Qt::red);
    }

    if (proxyIndex.column() == this->followup) {
      const int semaforo = QIdentityProxyModel::data(index(proxyIndex.row(), this->semaforo), Qt::DisplayRole).toInt();

      if (semaforo == Quente) return QBrush(Qt::black);
      if (semaforo == Morno) return QBrush(Qt::black);
      if (semaforo == Frio) return QBrush(Qt::black);
    }

    const QString status = QIdentityProxyModel::data(index(proxyIndex.row(), this->status), Qt::DisplayRole).toString();
    if (status == "FECHADO") return QBrush(Qt::black);
    if (status == "CANCELADO") return QBrush(Qt::black);
    if (status == "PERDIDO") return QBrush(Qt::black);

    return qApp->style()->objectName() == "fusion" ? QBrush(Qt::black) : QBrush(Qt::white);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
