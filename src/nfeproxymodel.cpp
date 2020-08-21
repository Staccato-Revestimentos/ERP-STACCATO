#include "nfeproxymodel.h"

#include "application.h"
#include "usersession.h"

#include <QDate>

NFeProxyModel::NFeProxyModel(SqlTableModel *model, QObject *parent)
    : SortFilterProxyModel(model, parent), statusColumn(model->fieldIndex("statusDistribuicao")), dataColumn(model->fieldIndex("dataDistribuicao")) {}

QVariant NFeProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (role == Qt::BackgroundRole or role == Qt::ForegroundRole) {
    if (statusColumn != -1 and dataColumn != -1) {
      const QString statusEvento = proxyIndex.siblingAtColumn(statusColumn).data().toString();
      const QDate dataEvento = proxyIndex.siblingAtColumn(dataColumn).data().toDate();

      // prazo limite é de 180 dias

      if (statusEvento == "CIÊNCIA" and qApp->serverDate() > dataEvento.addDays(60)) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::red); }
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      } else if (statusEvento == "CIÊNCIA" and qApp->serverDate() > dataEvento.addDays(30)) {
        if (role == Qt::BackgroundRole) { return QBrush(Qt::yellow); }
        if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
      }
    }

    if (role == Qt::ForegroundRole) {
      const QString tema = UserSession::getSetting("User/tema").value_or("claro").toString();

      return (tema == "claro") ? QBrush(Qt::black) : QBrush(Qt::white);
    }
  }

  return QSortFilterProxyModel::data(proxyIndex, role);
}
