#include <QApplication>
#include <QBrush>
#include <QDate>
#include <QStyle>

#include "estoqueprazoproxymodel.h"

EstoquePrazoProxyModel::EstoquePrazoProxyModel(SqlTableModel *model, QObject *parent) : QIdentityProxyModel(parent), dias(model->fieldIndex("prazoEntrega")) { setSourceModel(model); }

QVariant EstoquePrazoProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole) {
    if (proxyIndex.column() == this->dias) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toDate();

      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::red);
    }
  }

  if (role == Qt::ForegroundRole) {

    // those paint the text as black if the background is colored

    if (proxyIndex.column() == this->dias) {
      const QDate prazo = QIdentityProxyModel::data(index(proxyIndex.row(), this->dias), Qt::DisplayRole).toDate();

      if (prazo < QDate::currentDate() and not prazo.isNull()) return QBrush(Qt::black);
    }

    //

    return qApp->style()->objectName() == "fusion" ? QBrush(Qt::black) : QBrush(Qt::white);
  }

  return QIdentityProxyModel::data(proxyIndex, role);
}
