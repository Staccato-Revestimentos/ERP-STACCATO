#pragma once

#include "sqltablemodel.h"

#include <QDialog>

namespace Ui {
class Devolucao;
}

class Devolucao final : public QDialog {
  Q_OBJECT

public:
  explicit Devolucao(const QString &idVenda, const bool isRepresentacao, QWidget *parent);
  ~Devolucao();

private:
  // attributes
  bool const isRepresentacao;
  QString const idVenda;
  QString idDevolucao;
  SqlTableModel modelCliente;
  SqlTableModel modelCompra;
  SqlTableModel modelConsumos;
  SqlTableModel modelDevolvidos1;
  SqlTableModel modelPagamentos;
  SqlTableModel modelProdutos2;
  SqlTableModel modelVenda;
  Ui::Devolucao *ui;
  // methods
  auto alterarLinhaOriginal(const int currentRow) -> void;
  auto atualizarDevolucao() -> void;
  auto atualizarIdRelacionado(const int currentRow) -> void;
  auto copiarProdutoParaDevolucao(const int currentRow) -> void;
  auto criarContas() -> void;
  auto criarDevolucao() -> void;
  auto determinarIdDevolucao() -> void;
  auto devolverItem(const int currentRow, const int novoIdVendaProduto2) -> void;
  auto dividirCompra(const int currentRow, const int novoIdVendaProduto2) -> void;
  auto dividirConsumo(const int currentRow, const int novoIdVendaProduto2) -> void;
  auto dividirVenda(const int currentRow, const int novoIdVendaProduto2) -> void;
  auto inserirItens(const int currentRow, const int novoIdVendaProduto2) -> void;
  auto limparCampos() -> void;
  auto on_doubleSpinBoxCaixas_valueChanged(const double caixas) -> void;
  auto on_doubleSpinBoxCredito_valueChanged(const double credito) -> void;
  auto on_doubleSpinBoxPorcentagem_valueChanged(const double porcentagem) -> void;
  auto on_doubleSpinBoxQuant_valueChanged(const double quant) -> void;
  auto on_pushButtonDevolverItem_clicked() -> void;
  auto on_tableProdutos_clicked(const QModelIndex &index) -> void;
  auto salvarCredito() -> void;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
};
