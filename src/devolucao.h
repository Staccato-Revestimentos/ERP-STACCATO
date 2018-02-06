#ifndef DEVOLUCAO_H
#define DEVOLUCAO_H

#include <QDataWidgetMapper>

#include "dialog.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class Devolucao;
}

class Devolucao final : public Dialog {
  Q_OBJECT

public:
  explicit Devolucao(const QString &idVenda, QWidget *parent = nullptr);
  ~Devolucao();

private:
  // attributes
  bool createNewId = false;
  const QString idVenda;
  QDataWidgetMapper mapperItem;
  QString idDevolucao;
  SqlRelationalTableModel modelCliente;
  SqlRelationalTableModel modelDevolvidos;
  SqlRelationalTableModel modelPagamentos;
  SqlRelationalTableModel modelProdutos;
  SqlRelationalTableModel modelVenda;
  Ui::Devolucao *ui;
  // methods
  auto atualizarDevolucao() -> bool;
  auto calcPrecoItemTotal() -> void;
  auto criarContas() -> bool;
  auto criarDevolucao() -> bool;
  auto desvincularCompra() -> bool;
  auto determinarIdDevolucao() -> void;
  auto devolverItem(const QModelIndexList &list) -> bool;
  auto inserirItens(const QModelIndexList &list) -> bool;
  auto limparCampos() -> void;
  auto on_doubleSpinBoxCaixas_valueChanged(const double caixas) -> void;
  auto on_doubleSpinBoxQuant_editingFinished() -> void;
  auto on_doubleSpinBoxQuant_valueChanged(double) -> void;
  auto on_doubleSpinBoxTotalItem_valueChanged(double value) -> void;
  auto on_groupBoxCredito_toggled(bool) -> void;
  auto on_pushButtonDevolverItem_clicked() -> void;
  auto on_tableProdutos_clicked(const QModelIndex &index) -> void;
  auto salvarCredito() -> bool;
  auto setupTables() -> void;
};

#endif // DEVOLUCAO_H
