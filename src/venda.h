#pragma once

#include "registerdialog.h"
#include "sqltreemodel.h"

namespace Ui {
class Venda;
}

class Venda final : public RegisterDialog {
  Q_OBJECT

public:
  explicit Venda(QWidget *parent);
  ~Venda();

  auto prepararVenda(const QString &idOrcamento) -> void;
  auto setFinanceiro() -> void;
  auto show() -> void;

private:
  // attributes
  QList<QSqlRecord> backupItem;
  bool financeiro = false;
  bool correcao = false;
  int idLoja;
  bool representacao;
  bool canChangeFrete = false;
  double minimoFrete;
  double porcFrete;
  SqlTableModel modelFluxoCaixa;
  SqlTableModel modelFluxoCaixa2;
  SqlTableModel modelItem;
  SqlTableModel modelItem2;
  SqlTreeModel modelTree;
  Ui::Venda *ui;
  // methods
  auto cadastrar() -> void final;
  auto clearFields() -> void final;
  auto registerMode() -> void final;
  auto savingProcedures() -> void final;
  auto setupMapper() -> void final;
  auto successMessage() -> void final;
  auto updateMode() -> void final;
  auto verifyFields() -> void final;
  auto viewRegister() -> bool final;

  auto atualizarCredito() -> void;
  auto calcPrecoGlobalTotal() -> void;
  auto cancelamento() -> void;
  auto copiaProdutosOrcamento() -> void;
  auto criarComissaoProfissional() -> void;
  auto criarConsumos() -> void;
  auto financeiroSalvar() -> void;
  auto generateId() -> void;
  auto montarFluxoCaixa() -> void;
  auto on_checkBoxFreteManual_clicked(const bool checked) -> void;
  auto on_checkBoxPontuacaoIsento_toggled(bool checked) -> void;
  auto on_checkBoxPontuacaoPadrao_toggled(bool checked) -> void;
  auto on_checkBoxRT_toggled(bool checked) -> void;
  auto on_dateTimeEdit_dateTimeChanged(const QDateTime &) -> void;
  auto on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double descontoReais) -> void;
  auto on_doubleSpinBoxDescontoGlobal_valueChanged(const double descontoPorc) -> void;
  auto on_doubleSpinBoxFrete_valueChanged(const double frete) -> void;
  auto on_doubleSpinBoxTotal_valueChanged(const double total) -> void;
  auto on_itemBoxProfissional_textChanged(const QString &) -> void;
  auto on_pushButtonAdicionarObservacao_clicked() -> void;
  auto on_pushButtonCadastrarPedido_clicked() -> void;
  auto on_pushButtonCancelamento_clicked() -> void;
  auto on_pushButtonComprovantes_clicked() -> void;
  auto on_pushButtonCorrigirFluxo_clicked() -> void;
  auto on_pushButtonDevolucao_clicked() -> void;
  auto on_pushButtonFinanceiroSalvar_clicked() -> void;
  auto on_pushButtonGerarExcel_clicked() -> void;
  auto on_pushButtonGerarPdf_clicked() -> void;
  auto on_pushButtonVoltar_clicked() -> void;
  auto setConnections() -> void;
  auto setTreeView() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto verificaDisponibilidadeEstoque() -> void;
};
