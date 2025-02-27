#include "importaprodutosproxymodel.h"

#include "application.h"
#include "user.h"

#include <QSqlRecord>

ImportaProdutosProxyModel::ImportaProdutosProxyModel(QSqlQueryModel *model, QObject *parent) : SortFilterProxyModel(model, parent), descontinuadoColumn(model->record().indexOf("descontinuado")) {
  if (descontinuadoColumn == -1) { throw RuntimeException("ImportaProdutosProxyModel Coluna -1!"); }
}

QVariant ImportaProdutosProxyModel::data(const QModelIndex &proxyIndex, const int role) const {
  if (descontinuadoColumn != -1 and (role == Qt::BackgroundRole or role == Qt::ForegroundRole)) {
    const bool descontinuado = proxyIndex.siblingAtColumn(descontinuadoColumn).data().toBool();

    if (descontinuado) {
      if (role == Qt::BackgroundRole) { return QBrush(Qt::cyan); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    const Status value = static_cast<Status>(proxyIndex.siblingAtColumn(proxyIndex.column() + 1).data().toInt());

    if (value == Status::Novo) {
      if (role == Qt::BackgroundRole) { return QBrush(Qt::green); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    if (value == Status::Atualizado) {
      if (role == Qt::BackgroundRole) { return QBrush(Qt::yellow); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    if (value == Status::ForaPadrao) {
      if (role == Qt::BackgroundRole) { return QBrush(Qt::gray); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }

    if (value == Status::Errado) {
      if (role == Qt::BackgroundRole) { return QBrush(Qt::red); }
      if (role == Qt::ForegroundRole) { return QBrush(Qt::black); }
    }
  }

  if (role == Qt::ForegroundRole) {
    const QString tema = User::getSetting("User/tema").toString();

    return (tema == "escuro") ? QBrush(Qt::white) : QBrush(Qt::black);
  }

  return SortFilterProxyModel::data(proxyIndex, role);
}
