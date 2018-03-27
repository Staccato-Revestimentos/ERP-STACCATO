#ifndef CONTAS_H
#define CONTAS_H

#include "dialog.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class Contas;
}

class Contas final : public Dialog {
  Q_OBJECT

public:
  enum class Tipo { Pagar, Receber };
  explicit Contas(const Tipo tipo, QWidget *parent = nullptr);
  ~Contas();
  auto viewConta(const QString &idPagamento, const QString &contraparte) -> void;

private:
  // attributes
  const Tipo tipo;
  SqlRelationalTableModel modelPendentes;
  SqlRelationalTableModel modelProcessados;
  Ui::Contas *ui;
  // methods
  auto on_pushButtonSalvar_clicked() -> void;
  auto on_tablePendentes_entered(const QModelIndex &) -> void;
  auto on_tableProcessados_entered(const QModelIndex &) -> void;
  auto preencher(const QModelIndex &index) -> void;
  auto setupTables() -> void;
  auto validarData(const QModelIndex &index) -> void;
  auto verifyFields() -> bool;
};

#endif // CONTAS_H
