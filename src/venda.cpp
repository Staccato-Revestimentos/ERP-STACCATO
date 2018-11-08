#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

#include "application.h"
#include "cadastrocliente.h"
#include "checkboxdelegate.h"
#include "devolucao.h"
#include "doubledelegate.h"
#include "estoque.h"
#include "excel.h"
#include "impressao.h"
#include "logindialog.h"
#include "noeditdelegate.h"
#include "orcamento.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "searchdialogproxymodel.h"
#include "singleeditdelegate.h"
#include "ui_venda.h"
#include "usersession.h"
#include "venda.h"

Venda::Venda(QWidget *parent) : RegisterDialog("venda", "idVenda", parent), ui(new Ui::Venda) {
  ui->setupUi(this);

  Q_FOREACH (const auto &line, findChildren<QLineEdit *>()) { connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty); }

  setupTables();

  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxConsultor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxEnderecoFat->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));

  setupMapper();
  newRegister();

  ui->labelConsultor->hide();
  ui->itemBoxConsultor->hide();
  ui->itemBoxConsultor->setReadOnlyItemBox(true);

  ui->groupBoxFinanceiro->hide();
  ui->tableFluxoCaixa2->hide();

  for (auto &item : ui->frameRT->findChildren<QWidget *>()) { item->setHidden(true); }

  ui->splitter->setStretchFactor(0, 1);
  ui->splitter->setStretchFactor(1, 0);

  show();
}

Venda::~Venda() { delete ui; }

void Venda::setConnections() {
  connect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Venda::on_checkBoxFreteManual_clicked);
  connect(ui->checkBoxPontuacaoIsento, &QCheckBox::toggled, this, &Venda::on_checkBoxPontuacaoIsento_toggled);
  connect(ui->checkBoxPontuacaoPadrao, &QCheckBox::toggled, this, &Venda::on_checkBoxPontuacaoPadrao_toggled);
  connect(ui->checkBoxRT, &QCheckBox::toggled, this, &Venda::on_checkBoxRT_toggled);
  connect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &Venda::on_dateTimeEdit_dateTimeChanged);
  connect(ui->doubleSpinBoxDescontoGlobal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxDescontoGlobal_valueChanged);
  connect(ui->doubleSpinBoxDescontoGlobalReais, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  connect(ui->doubleSpinBoxFrete, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxFrete_valueChanged);
  connect(ui->doubleSpinBoxTotal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxTotal_valueChanged);
  connect(ui->doubleSpinBoxTotalPag, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxTotalPag_valueChanged);
  connect(ui->itemBoxProfissional, &ItemBox::textChanged, this, &Venda::on_itemBoxProfissional_textChanged);
  connect(ui->pushButtonAdicionarPagamento, &QPushButton::clicked, this, &Venda::on_pushButtonAdicionarPagamento_clicked);
  connect(ui->pushButtonCadastrarPedido, &QPushButton::clicked, this, &Venda::on_pushButtonCadastrarPedido_clicked);
  connect(ui->pushButtonCancelamento, &QPushButton::clicked, this, &Venda::on_pushButtonCancelamento_clicked);
  connect(ui->pushButtonCorrigirFluxo, &QPushButton::clicked, this, &Venda::on_pushButtonCorrigirFluxo_clicked);
  connect(ui->pushButtonDevolucao, &QPushButton::clicked, this, &Venda::on_pushButtonDevolucao_clicked);
  connect(ui->pushButtonFinanceiroSalvar, &QPushButton::clicked, this, &Venda::on_pushButtonFinanceiroSalvar_clicked);
  connect(ui->pushButtonFreteLoja, &QPushButton::clicked, this, &Venda::on_pushButtonFreteLoja_clicked);
  connect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Venda::on_pushButtonGerarExcel_clicked);
  connect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Venda::on_pushButtonImprimir_clicked);
  connect(ui->pushButtonLimparPag, &QPushButton::clicked, this, &Venda::on_pushButtonLimparPag_clicked);
  connect(ui->pushButtonPgtLoja, &QPushButton::clicked, this, &Venda::on_pushButtonPgtLoja_clicked);
  connect(ui->pushButtonVoltar, &QPushButton::clicked, this, &Venda::on_pushButtonVoltar_clicked);
  connect(ui->tableFluxoCaixa, &TableView::entered, this, &Venda::on_tableFluxoCaixa_entered);
  connect(ui->tableFluxoCaixa2, &TableView::entered, this, &Venda::on_tableFluxoCaixa2_entered);
}

void Venda::unsetConnections() {
  disconnect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Venda::on_checkBoxFreteManual_clicked);
  disconnect(ui->checkBoxPontuacaoIsento, &QCheckBox::toggled, this, &Venda::on_checkBoxPontuacaoIsento_toggled);
  disconnect(ui->checkBoxPontuacaoPadrao, &QCheckBox::toggled, this, &Venda::on_checkBoxPontuacaoPadrao_toggled);
  disconnect(ui->checkBoxRT, &QCheckBox::toggled, this, &Venda::on_checkBoxRT_toggled);
  disconnect(ui->dateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &Venda::on_dateTimeEdit_dateTimeChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxDescontoGlobal_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobalReais, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  disconnect(ui->doubleSpinBoxFrete, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxFrete_valueChanged);
  disconnect(ui->doubleSpinBoxTotal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxTotal_valueChanged);
  disconnect(ui->doubleSpinBoxTotalPag, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &Venda::on_doubleSpinBoxTotalPag_valueChanged);
  disconnect(ui->itemBoxProfissional, &ItemBox::textChanged, this, &Venda::on_itemBoxProfissional_textChanged);
  disconnect(ui->pushButtonAdicionarPagamento, &QPushButton::clicked, this, &Venda::on_pushButtonAdicionarPagamento_clicked);
  disconnect(ui->pushButtonCadastrarPedido, &QPushButton::clicked, this, &Venda::on_pushButtonCadastrarPedido_clicked);
  disconnect(ui->pushButtonCancelamento, &QPushButton::clicked, this, &Venda::on_pushButtonCancelamento_clicked);
  disconnect(ui->pushButtonCorrigirFluxo, &QPushButton::clicked, this, &Venda::on_pushButtonCorrigirFluxo_clicked);
  disconnect(ui->pushButtonDevolucao, &QPushButton::clicked, this, &Venda::on_pushButtonDevolucao_clicked);
  disconnect(ui->pushButtonFinanceiroSalvar, &QPushButton::clicked, this, &Venda::on_pushButtonFinanceiroSalvar_clicked);
  disconnect(ui->pushButtonFreteLoja, &QPushButton::clicked, this, &Venda::on_pushButtonFreteLoja_clicked);
  disconnect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Venda::on_pushButtonGerarExcel_clicked);
  disconnect(ui->pushButtonImprimir, &QPushButton::clicked, this, &Venda::on_pushButtonImprimir_clicked);
  disconnect(ui->pushButtonLimparPag, &QPushButton::clicked, this, &Venda::on_pushButtonLimparPag_clicked);
  disconnect(ui->pushButtonPgtLoja, &QPushButton::clicked, this, &Venda::on_pushButtonPgtLoja_clicked);
  disconnect(ui->pushButtonVoltar, &QPushButton::clicked, this, &Venda::on_pushButtonVoltar_clicked);
  disconnect(ui->tableFluxoCaixa, &TableView::entered, this, &Venda::on_tableFluxoCaixa_entered);
  disconnect(ui->tableFluxoCaixa2, &TableView::entered, this, &Venda::on_tableFluxoCaixa2_entered);
}

void Venda::setupTables() {
  modelItem.setTable("venda_has_produto");
  modelItem.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelItem.setHeaderData("selecionado", "");
  modelItem.setHeaderData("fornecedor", "Fornecedor");
  modelItem.setHeaderData("produto", "Produto");
  modelItem.setHeaderData("obs", "Obs.");
  modelItem.setHeaderData("prcUnitario", "Preço/Un");
  modelItem.setHeaderData("caixas", "Caixas");
  modelItem.setHeaderData("quant", "Quant.");
  modelItem.setHeaderData("un", "Un.");
  modelItem.setHeaderData("unCaixa", "Un./Cx.");
  modelItem.setHeaderData("codComercial", "Código");
  modelItem.setHeaderData("formComercial", "Formato");
  modelItem.setHeaderData("parcial", "Subtotal");
  modelItem.setHeaderData("desconto", "Desc. %");
  modelItem.setHeaderData("parcialDesc", "Desc. Parc.");
  modelItem.setHeaderData("descGlobal", "Desc. Glob. %");
  modelItem.setHeaderData("total", "Total");
  modelItem.setHeaderData("status", "Status");
  modelItem.setHeaderData("dataPrevCompra", "Prev. Compra");
  modelItem.setHeaderData("dataRealCompra", "Data Compra");
  modelItem.setHeaderData("dataPrevConf", "Prev. Confirm.");
  modelItem.setHeaderData("dataRealConf", "Data Confirm.");
  modelItem.setHeaderData("dataPrevFat", "Prev. Fat.");
  modelItem.setHeaderData("dataRealFat", "Data Fat.");
  modelItem.setHeaderData("dataPrevColeta", "Prev. Coleta");
  modelItem.setHeaderData("dataRealColeta", "Data Coleta");
  modelItem.setHeaderData("dataPrevReceb", "Prev. Receb.");
  modelItem.setHeaderData("dataRealReceb", "Data Receb.");
  modelItem.setHeaderData("dataPrevEnt", "Prev. Ent.");
  modelItem.setHeaderData("dataRealEnt", "Data Ent.");

  modelItem.setFilter("0");

  if (not modelItem.select()) { return; }

  ui->tableProdutos->setModel(new SearchDialogProxyModel(&modelItem, this));
  ui->tableProdutos->hideColumn("idRelacionado");
  ui->tableProdutos->hideColumn("statusOriginal");
  ui->tableProdutos->hideColumn("recebeu");
  ui->tableProdutos->hideColumn("entregou");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("estoque");
  ui->tableProdutos->hideColumn("promocao");
  ui->tableProdutos->hideColumn("idCompra");
  ui->tableProdutos->hideColumn("idNFeSaida");
  ui->tableProdutos->hideColumn("idNFeFutura");
  ui->tableProdutos->hideColumn("idVendaProduto");
  ui->tableProdutos->hideColumn("selecionado");
  ui->tableProdutos->hideColumn("idVenda");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("comissao");
  ui->tableProdutos->hideColumn("reposicaoEntrega");
  ui->tableProdutos->hideColumn("reposicaoReceb");
  ui->tableProdutos->hideColumn("mostrarDesconto");

  ui->tableProdutos->setItemDelegate(new DoubleDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("quant", new DoubleDelegate(this, 4));
  ui->tableProdutos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcialDesc", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("desconto", new PorcentagemDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("descGlobal", new PorcentagemDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("total", new ReaisDelegate(this));

  //-----------------------------------------------------------------

  modelFluxoCaixa.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelFluxoCaixa.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa.setHeaderData("valor", "R$");
  modelFluxoCaixa.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa.setHeaderData("status", "Status");
  modelFluxoCaixa.setHeaderData("representacao", "Representação");

  modelFluxoCaixa.setFilter("0");

  if (not modelFluxoCaixa.select()) { return; }

  ui->tableFluxoCaixa->setModel(&modelFluxoCaixa);
  ui->tableFluxoCaixa->hideColumn("nfe");
  ui->tableFluxoCaixa->hideColumn("contraParte");
  ui->tableFluxoCaixa->hideColumn("idVenda");
  ui->tableFluxoCaixa->hideColumn("idLoja");
  ui->tableFluxoCaixa->hideColumn("idPagamento");
  ui->tableFluxoCaixa->hideColumn("dataEmissao");
  ui->tableFluxoCaixa->hideColumn("dataRealizado");
  ui->tableFluxoCaixa->hideColumn("valorReal");
  ui->tableFluxoCaixa->hideColumn("tipoReal");
  ui->tableFluxoCaixa->hideColumn("parcelaReal");
  ui->tableFluxoCaixa->hideColumn("contaDestino");
  ui->tableFluxoCaixa->hideColumn("tipoDet");
  ui->tableFluxoCaixa->hideColumn("centroCusto");
  ui->tableFluxoCaixa->hideColumn("grupo");
  ui->tableFluxoCaixa->hideColumn("subGrupo");
  ui->tableFluxoCaixa->hideColumn("comissao");
  ui->tableFluxoCaixa->hideColumn("taxa");
  ui->tableFluxoCaixa->hideColumn("desativado");
  ui->tableFluxoCaixa->setItemDelegate(new NoEditDelegate(this));
  ui->tableFluxoCaixa->setItemDelegateForColumn("representacao", new CheckBoxDelegate(this, true));
  ui->tableFluxoCaixa->setItemDelegateForColumn("observacao", new SingleEditDelegate(this));
  ui->tableFluxoCaixa->setItemDelegateForColumn("valor", new ReaisDelegate(this));

  //-----------------------------------------------------------------

  modelFluxoCaixa2.setTable("conta_a_receber_has_pagamento");
  modelFluxoCaixa2.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelFluxoCaixa2.setHeaderData("contraParte", "ContraParte");
  modelFluxoCaixa2.setHeaderData("tipo", "Tipo");
  modelFluxoCaixa2.setHeaderData("parcela", "Parcela");
  modelFluxoCaixa2.setHeaderData("valor", "R$");
  modelFluxoCaixa2.setHeaderData("dataPagamento", "Data");
  modelFluxoCaixa2.setHeaderData("observacao", "Obs.");
  modelFluxoCaixa2.setHeaderData("status", "Status");

  modelFluxoCaixa2.setFilter("0");

  if (not modelFluxoCaixa2.select()) { return; }

  ui->tableFluxoCaixa2->setModel(&modelFluxoCaixa2);
  ui->tableFluxoCaixa2->hideColumn("idPagamento");
  ui->tableFluxoCaixa2->hideColumn("dataEmissao");
  ui->tableFluxoCaixa2->hideColumn("idVenda");
  ui->tableFluxoCaixa2->hideColumn("idLoja");
  ui->tableFluxoCaixa2->hideColumn("representacao");
  ui->tableFluxoCaixa2->hideColumn("dataRealizado");
  ui->tableFluxoCaixa2->hideColumn("valorReal");
  ui->tableFluxoCaixa2->hideColumn("tipoReal");
  ui->tableFluxoCaixa2->hideColumn("parcelaReal");
  ui->tableFluxoCaixa2->hideColumn("contaDestino");
  ui->tableFluxoCaixa2->hideColumn("tipoDet");
  ui->tableFluxoCaixa2->hideColumn("centroCusto");
  ui->tableFluxoCaixa2->hideColumn("grupo");
  ui->tableFluxoCaixa2->hideColumn("subGrupo");
  ui->tableFluxoCaixa2->hideColumn("comissao");
  ui->tableFluxoCaixa2->hideColumn("taxa");
  ui->tableFluxoCaixa2->hideColumn("desativado");
  ui->tableFluxoCaixa2->setItemDelegateForColumn("valor", new ReaisDelegate(this));
}

void Venda::resetarPagamentos() {
  ui->doubleSpinBoxTotalPag->setValue(0);
  ui->doubleSpinBoxTotalPag->setMaximum(ui->doubleSpinBoxTotal->value());

  ui->widgetPgts->resetarPagamentos();

  montarFluxoCaixa();
}

void Venda::prepararVenda(const QString &idOrcamento) {
  // TODO: verificar se as quantidades de produto_estoque ainda estão disponiveis

  ui->lineEditIdOrcamento->setText(idOrcamento);
  ui->lineEditVenda->setText("Auto gerado");
  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  if (not copiaProdutosOrcamento()) { return; }

  // -------------------------------------------------------------------------

  QSqlQuery queryOrc;
  queryOrc.prepare("SELECT idUsuario, idLoja, idUsuarioConsultor, idCliente, idEnderecoEntrega, idProfissional, data, subTotalBru, subTotalLiq, frete, "
                   "freteManual, descontoPorc, descontoReais, total, status, observacao, prazoEntrega, representacao FROM orcamento WHERE idOrcamento = :idOrcamento");
  queryOrc.bindValue(":idOrcamento", idOrcamento);

  if (not queryOrc.exec() or not queryOrc.first()) { return qApp->enqueueError("Erro buscando orçamento: " + queryOrc.lastError().text()); }

  ui->itemBoxVendedor->setId(queryOrc.value("idUsuario"));
  ui->itemBoxConsultor->setId(queryOrc.value("idUsuarioConsultor"));
  ui->itemBoxEndereco->setFilter("idCliente = " + queryOrc.value("idCliente").toString() + " AND desativado = FALSE");
  ui->itemBoxEnderecoFat->setFilter("idCliente = " + queryOrc.value("idCliente").toString() + " AND desativado = FALSE");
  ui->itemBoxCliente->setId(queryOrc.value("idCliente"));
  ui->itemBoxProfissional->setId(queryOrc.value("idProfissional"));
  ui->itemBoxEndereco->setId(queryOrc.value("idEnderecoEntrega"));
  ui->dateTimeEditOrc->setDateTime(queryOrc.value("data").toDateTime());
  ui->plainTextEdit->setPlainText(queryOrc.value("observacao").toString());
  ui->doubleSpinBoxSubTotalBruto->setValue(queryOrc.value("subTotalBru").toDouble());
  ui->doubleSpinBoxSubTotalLiq->setValue(queryOrc.value("subTotalLiq").toDouble());
  ui->doubleSpinBoxFrete->setValue(queryOrc.value("frete").toDouble());
  ui->checkBoxFreteManual->setChecked(queryOrc.value("freteManual").toBool());
  ui->doubleSpinBoxDescontoGlobal->setValue(queryOrc.value("descontoPorc").toDouble());
  ui->doubleSpinBoxDescontoGlobalReais->setValue(queryOrc.value("descontoReais").toDouble());
  ui->doubleSpinBoxTotal->setValue(queryOrc.value("total").toDouble());
  ui->spinBoxPrazoEntrega->setValue(queryOrc.value("prazoEntrega").toInt());

  idLoja = queryOrc.value("idLoja").toInt();
  representacao = queryOrc.value("representacao").toBool();

  // -------------------------------------------------------------------------

  QSqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", idLoja);

  if (not queryFrete.exec() or not queryFrete.first()) { return qApp->enqueueError("Erro buscando parâmetros do frete: " + queryFrete.lastError().text()); }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();

  if (not representacao) {
    ui->pushButtonFreteLoja->hide();
    ui->pushButtonPgtLoja->hide();
    ui->tableFluxoCaixa->hideColumn("representacao");
  }

  // -------------------------------------------------------------------------

  QSqlQuery queryCredito;
  queryCredito.prepare("SELECT credito FROM cliente WHERE idCliente = :idCliente");
  queryCredito.bindValue(":idCliente", queryOrc.value("idCliente"));

  if (not queryCredito.exec() or not queryCredito.first()) { return qApp->enqueueError("Erro buscando crédito cliente: " + queryCredito.lastError().text()); }

  ui->doubleSpinBoxCreditoTotal->setValue(queryCredito.value("credito").toDouble());

  // -------------------------------------------------------------------------

  resetarPagamentos();

  ui->widgetPgts->setMinimumWidth(550);
  ui->tableProdutos->resizeColumnsToContents();

  if (ui->itemBoxConsultor->getId().isValid()) {
    ui->labelConsultor->show();
    ui->itemBoxConsultor->show();
  }

  setConnections();
}

bool Venda::verifyFields() {
  // TODO: pintar campos errados de vermelho
  // TODO: pintar campos necessarios de amarelo
  // TODO: pintar campos certos de verde
  // TODO: pintar totalPag de vermelho enquanto o total for diferente

  if (ui->framePagamentos_2->isHidden()) { return true; }

  if (not qFuzzyCompare(ui->doubleSpinBoxTotalPag->value(), ui->doubleSpinBoxTotal->value())) { return qApp->enqueueError(false, "Total dos pagamentos difere do total do pedido!"); }

  double sum = 0;

  Q_FOREACH (const auto &spinbox, ui->widgetPgts->listDoubleSpinPgt) { sum += spinbox->value(); }

  if (not qFuzzyCompare(sum, ui->doubleSpinBoxTotalPag->value())) { return qApp->enqueueError(false, "Valor dos pagamentos difere do total!"); }

  for (int i = 0; i < ui->widgetPgts->listCheckBoxRep.size(); ++i) {
    if (ui->widgetPgts->listComboPgt.at(i)->currentText() != "Escolha uma opção!" and ui->widgetPgts->listLinePgt.at(i)->text().isEmpty()) {
      ui->widgetPgts->listLinePgt.at(i)->setFocus();
      return qApp->enqueueError(false, "Faltou preencher observação do pagamento " + QString::number(i + 1) + "!");
    }

    if (ui->widgetPgts->listDoubleSpinPgt.at(i)->value() > 0 and ui->widgetPgts->listComboPgt.at(i)->currentText() == "Escolha uma opção!") {
      ui->widgetPgts->listComboPgt.at(i)->setFocus();
      return qApp->enqueueError(false, "Por favor escolha a forma de pagamento " + QString::number(i + 1) + "!");
    }

    if (qFuzzyIsNull(ui->widgetPgts->listDoubleSpinPgt.at(i)->value()) and ui->widgetPgts->listComboPgt.at(i)->currentText() != "Escolha uma opção!") {
      ui->widgetPgts->listDoubleSpinPgt.at(i)->setFocus();
      return qApp->enqueueError(false, "Pagamento " + QString::number(i + 1) + " está com valor 0!");
    }
  }

  if (ui->spinBoxPrazoEntrega->value() == 0) {
    ui->spinBoxPrazoEntrega->setFocus();
    return qApp->enqueueError(false, "Por favor preencha o prazo de entrega.");
  }

  if (ui->itemBoxEnderecoFat->text().isEmpty()) {
    ui->itemBoxEnderecoFat->setFocus();
    return qApp->enqueueError(false, "Deve selecionar um endereço de faturamento!");
  }

  if (ui->itemBoxProfissional->getId() != 1 and not ui->checkBoxPontuacaoIsento->isChecked() and not ui->checkBoxPontuacaoPadrao->isChecked()) {
    ui->checkBoxRT->setChecked(true);
    ui->doubleSpinBoxPontuacao->setFocus();
    return qApp->enqueueError(false, "Por favor preencha a pontuação!");
  }

  return true;
}

void Venda::calcPrecoGlobalTotal() {
  double subTotalBruto = 0.;
  double subTotalItens = 0.;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    const double itemBruto = modelItem.data(row, "quant").toDouble() * modelItem.data(row, "prcUnitario").toDouble();
    const double descItem = modelItem.data(row, "desconto").toDouble() / 100.;
    const double stItem = itemBruto * (1. - descItem);
    subTotalBruto += itemBruto;
    subTotalItens += stItem;
  }

  ui->doubleSpinBoxSubTotalBruto->setValue(subTotalBruto);
  ui->doubleSpinBoxSubTotalLiq->setValue(subTotalItens);
}

void Venda::clearFields() {}

void Venda::setupMapper() {
  addMapping(ui->checkBoxFreteManual, "freteManual");
  addMapping(ui->dateTimeEdit, "data");
  addMapping(ui->dateTimeEditOrc, "dataOrc");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->itemBoxCliente, "idCliente", "id");
  addMapping(ui->itemBoxConsultor, "idUsuarioConsultor", "id");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "id");
  addMapping(ui->itemBoxEnderecoFat, "idEnderecoFaturamento", "id");
  addMapping(ui->itemBoxProfissional, "idProfissional", "id");
  addMapping(ui->itemBoxVendedor, "idUsuario", "id");
  addMapping(ui->lineEditIdOrcamento, "idOrcamento");
  addMapping(ui->lineEditVenda, "idVenda");
  addMapping(ui->plainTextEdit, "observacao");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
}

void Venda::on_pushButtonCadastrarPedido_clicked() { save(); }

void Venda::on_doubleSpinBoxPgt_valueChanged() {
  double sum = 0;

  Q_FOREACH (const auto &spinbox, ui->widgetPgts->listDoubleSpinPgt) { sum += spinbox->value(); }

  ui->doubleSpinBoxTotalPag->setValue(sum);

  montarFluxoCaixa();
}

void Venda::on_comboBoxPgt_currentTextChanged(const int index, const QString &text) {
  if (text == "Escolha uma opção!") { return; }

  if (text == "Conta Cliente") {
    ui->widgetPgts->listDoubleSpinPgt.at(index)->setMaximum(ui->doubleSpinBoxCreditoTotal->value());
    ui->widgetPgts->listComboParc.at(index)->clear();
    ui->widgetPgts->listComboParc.at(index)->addItem("1x");
    montarFluxoCaixa();
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT parcelas FROM forma_pagamento WHERE pagamento = :pagamento");
  query.bindValue(":pagamento", ui->widgetPgts->listComboPgt.at(index)->currentText());

  if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro lendo formas de pagamentos: " + query.lastError().text()); }

  const int parcelas = query.value("parcelas").toInt();

  ui->widgetPgts->listComboParc.at(index)->clear();

  for (int i = 0; i < parcelas; ++i) { ui->widgetPgts->listComboParc.at(index)->addItem(QString::number(i + 1) + "x"); }

  ui->widgetPgts->listComboParc.at(index)->setEnabled(true);

  ui->widgetPgts->listDatePgt.at(index)->setEnabled(true);

  montarFluxoCaixa();
}

bool Venda::savingProcedures() {
  // TODO: remove novoPrazoEntrega from DB?

  if (not setData("status", todosProdutosSaoEstoque() ? "ESTOQUE" : "PENDENTE")) { return false; }
  if (not setData("data", ui->dateTimeEdit->dateTime())) { return false; }
  if (not setData("dataOrc", ui->dateTimeEditOrc->dateTime())) { return false; }
  if (not setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value())) { return false; }
  if (not setData("descontoReais", ui->doubleSpinBoxDescontoGlobalReais->value())) { return false; }
  if (not setData("frete", ui->doubleSpinBoxFrete->value())) { return false; }
  if (not setData("freteManual", ui->checkBoxFreteManual->isChecked())) { return false; }
  if (not setData("idCliente", ui->itemBoxCliente->getId())) { return false; }
  if (not setData("idEnderecoEntrega", ui->itemBoxEndereco->getId())) { return false; }
  if (not setData("idEnderecoFaturamento", ui->itemBoxEnderecoFat->getId())) { return false; }
  if (not setData("idLoja", idLoja)) { return false; }
  if (not setData("idOrcamento", ui->lineEditIdOrcamento->text())) { return false; }
  if (not setData("idProfissional", ui->itemBoxProfissional->getId())) { return false; }
  if (not setData("idUsuario", ui->itemBoxVendedor->getId())) { return false; }
  if (ui->itemBoxConsultor->getId().isValid() and not setData("idUsuarioConsultor", ui->itemBoxConsultor->getId())) { return false; }
  if (not setData("idVenda", ui->lineEditVenda->text())) { return false; }
  if (not setData("novoPrazoEntrega", ui->spinBoxPrazoEntrega->value())) { return false; }
  if (not setData("observacao", ui->plainTextEdit->toPlainText())) { return false; }
  if (not setData("prazoEntrega", ui->spinBoxPrazoEntrega->value())) { return false; }
  if (not setData("representacao", ui->lineEditVenda->text().endsWith("R") ? 1 : 0)) { return false; }
  if (not setData("rt", ui->doubleSpinBoxPontuacao->value())) { return false; }
  if (not setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value())) { return false; }
  if (not setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value())) { return false; }
  if (not setData("total", ui->doubleSpinBoxTotal->value())) { return false; }

  return true;
}

bool Venda::todosProdutosSaoEstoque() {
  bool temEstoque = true;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (not modelItem.data(row, "estoque").toBool()) {
      temEstoque = false;
      break;
    }
  }

  return temEstoque;
}

void Venda::registerMode() {
  ui->framePagamentos->show();
  ui->pushButtonGerarExcel->hide();
  ui->pushButtonImprimir->hide();
  ui->pushButtonCancelamento->hide();
  ui->pushButtonDevolucao->hide();
  ui->pushButtonCadastrarPedido->show();
  ui->pushButtonVoltar->show();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(false);
  ui->doubleSpinBoxDescontoGlobal->setFrame(true);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->doubleSpinBoxTotal->setReadOnly(false);
  ui->doubleSpinBoxTotal->setFrame(true);
  ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::UpDownArrows);
  ui->checkBoxFreteManual->show();
}

void Venda::updateMode() {
  ui->framePagamentos_2->hide();
  ui->pushButtonGerarExcel->show();
  ui->pushButtonImprimir->show();
  ui->pushButtonCadastrarPedido->hide();
  ui->pushButtonVoltar->hide();
  ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobal->setFrame(false);
  ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxDescontoGlobalReais->setReadOnly(true);
  ui->doubleSpinBoxDescontoGlobalReais->setFrame(false);
  ui->doubleSpinBoxDescontoGlobalReais->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->doubleSpinBoxTotal->setReadOnly(true);
  ui->doubleSpinBoxTotal->setFrame(false);
  ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::NoButtons);
  ui->checkBoxFreteManual->hide();
}

bool Venda::viewRegister() {
  unsetConnections();

  const auto load = [&] {
    ui->doubleSpinBoxDescontoGlobalReais->setMinimum(-9999999);

    modelItem.setFilter("idVenda = '" + model.data(0, "idVenda").toString() + "'");

    if (not modelItem.select()) { return false; }

    if (not RegisterDialog::viewRegister()) { return false; }

    calcPrecoGlobalTotal();

    const QString idCliente = ui->itemBoxCliente->getId().toString();

    ui->itemBoxEndereco->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");
    ui->itemBoxEnderecoFat->setFilter("idCliente = " + idCliente + " AND desativado = FALSE");

    modelFluxoCaixa.setFilter("idVenda = '" + ui->lineEditVenda->text() + "' AND status != 'CANCELADO' AND status != 'SUBSTITUIDO' AND comissao = FALSE AND taxa = FALSE");

    if (not modelFluxoCaixa.select()) { return false; }

    for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) { ui->tableFluxoCaixa->openPersistentEditor(row, "representacao"); }

    if (financeiro) {
      // TODO: 1quando estiver tudo pago bloquear correcao de fluxo
      modelFluxoCaixa2.setFilter("idVenda = '" + ui->lineEditVenda->text() + "' AND status != 'CANCELADO' AND status != 'SUBSTITUIDO' AND (comissao = TRUE OR taxa = TRUE)");

      if (not modelFluxoCaixa2.select()) { return false; }

      ui->comboBoxFinanceiro->setCurrentText(model.data(0, "statusFinanceiro").toString());
    }

    ui->tableProdutos->resizeColumnsToContents();
    ui->tableFluxoCaixa->resizeColumnsToContents();
    ui->tableFluxoCaixa2->resizeColumnsToContents();

    ui->tableFluxoCaixa->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->spinBoxPrazoEntrega->setReadOnly(true);

    ui->itemBoxCliente->setReadOnlyItemBox(true);
    ui->itemBoxEndereco->setReadOnlyItemBox(true);
    ui->itemBoxEnderecoFat->setReadOnlyItemBox(true);
    ui->itemBoxProfissional->setReadOnlyItemBox(true);
    ui->itemBoxVendedor->setReadOnlyItemBox(true);

    ui->dateTimeEdit->setReadOnly(true);

    ui->plainTextEdit->setReadOnly(true);
    ui->plainTextEdit->setPlainText(data("observacao").toString());

    ui->pushButtonCancelamento->show();
    ui->pushButtonDevolucao->show();

    if (data("status") == "CANCELADO" or data("status") == "DEVOLUÇÃO") {
      ui->pushButtonCancelamento->hide();
      ui->pushButtonDevolucao->hide();
    }

    const QString tipoUsuario = UserSession::tipoUsuario();

    if (not tipoUsuario.contains("GERENTE") and tipoUsuario != "DIRETOR" and tipoUsuario != "ADMINISTRADOR") {
      ui->pushButtonDevolucao->hide();
      ui->pushButtonCancelamento->hide();
    }

    ui->framePagamentos->adjustSize();

    ui->checkBoxRT->setChecked(false);
    ui->checkBoxRT->setHidden(true);

    if (data("devolucao").toBool()) { ui->pushButtonDevolucao->hide(); }

    if (data("idUsuarioConsultor").toInt() != 0) {
      ui->labelConsultor->show();
      ui->itemBoxConsultor->show();
    }

    return true;
  }();

  setConnections();

  return load;
}

void Venda::on_pushButtonVoltar_clicked() {
  auto *orcamento = new Orcamento(parentWidget());
  orcamento->viewRegisterById(ui->lineEditIdOrcamento->text());

  orcamento->show();

  close();
}

void Venda::montarFluxoCaixa() {
  // TODO: 0nao calcular comissao para profissional 'NAO HA'
  // TODO: verificar se esta funcao nao está rodando mais de uma vez por operacao
  // TODO: lancamento de credito deve ser marcado direto como 'recebido' e statusFinanceiro == liberado

  if (ui->framePagamentos_2->isHidden()) { return; }

  if (not modelFluxoCaixa.select()) { return; }

  if (not modelFluxoCaixa2.select()) { return; }

  if (financeiro) {
    for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
      if (not modelFluxoCaixa.setData(row, "status", "SUBSTITUIDO")) { return; }
    }

    for (int row = 0; row < modelFluxoCaixa2.rowCount(); ++row) {
      if (not modelFluxoCaixa2.setData(row, "status", "SUBSTITUIDO")) { return; }
    }
  }

  for (int i = 0; i < ui->widgetPgts->listCheckBoxRep.size(); ++i) {
    if (ui->widgetPgts->listComboPgt.at(i)->currentText() != "Escolha uma opção!") {
      const int parcelas = ui->widgetPgts->listComboParc.at(i)->currentIndex() + 1;
      const double valor = ui->widgetPgts->listDoubleSpinPgt.at(i)->value();
      const int cartao = ui->widgetPgts->listComboPgt.at(i)->currentText() == "Cartão de crédito" ? 1 : 0;

      const double part1 = valor / parcelas;
      const int part2 = static_cast<int>(part1 * 100); // truncate with 2 decimals
      const double parcela = static_cast<double>(part2) / 100;
      const double resto = valor - (parcela * parcelas);

      const QDate dataEmissao = correcao ? modelFluxoCaixa.data(0, "dataEmissao").toDate() : QDate::currentDate();

      for (int x = 0, y = parcelas - 1; x < parcelas; ++x, --y) {
        const int row = modelFluxoCaixa.rowCount();
        modelFluxoCaixa.insertRow(row);
        if (not modelFluxoCaixa.setData(row, "contraParte", ui->itemBoxCliente->text())) { return; }
        if (not modelFluxoCaixa.setData(row, "dataEmissao", dataEmissao)) { return; }
        if (not modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text())) { return; }
        if (not modelFluxoCaixa.setData(row, "idLoja", idLoja)) { return; }
        if (not modelFluxoCaixa.setData(row, "dataPagamento", ui->widgetPgts->listDatePgt.at(i)->date().addMonths(x + cartao))) { return; }
        if (not modelFluxoCaixa.setData(row, "valor", parcela + (x == 0 ? resto : 0))) { return; }
        if (not modelFluxoCaixa.setData(row, "tipo", QString::number(i + 1) + ". " + ui->widgetPgts->listComboPgt.at(i)->currentText())) { return; }
        if (not modelFluxoCaixa.setData(row, "parcela", parcelas - y)) { return; }
        if (not modelFluxoCaixa.setData(row, "observacao", ui->widgetPgts->listLinePgt.at(i)->text())) { return; }
        if (not modelFluxoCaixa.setData(row, "representacao", ui->widgetPgts->listCheckBoxRep.at(i)->isChecked())) { return; }
        if (not modelFluxoCaixa.setData(row, "grupo", "Produtos - Venda")) { return; }
        if (not modelFluxoCaixa.setData(row, "subGrupo", "")) { return; }
        if (not modelFluxoCaixa.setData(row, "centroCusto", idLoja)) { return; }
      }

      // calculo comissao
      for (int z = 0, total = modelFluxoCaixa.rowCount(); z < total; ++z) {
        if (not modelFluxoCaixa.data(z, "representacao").toBool()) { continue; }
        if (not modelFluxoCaixa.data(z, "tipo").toString().contains(QString::number(i + 1))) { continue; }
        if (modelFluxoCaixa.data(z, "status").toString() == "SUBSTITUIDO") { continue; }

        const QString fornecedor = modelItem.data(0, "fornecedor").toString();
        const double totalVenda = ui->doubleSpinBoxTotal->value();
        const double frete = ui->doubleSpinBoxFrete->value();

        QSqlQuery query;
        query.prepare("SELECT comissaoLoja FROM fornecedor WHERE razaoSocial = :razaoSocial");
        query.bindValue(":razaoSocial", fornecedor);

        if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro buscando comissão: " + query.lastError().text()); }

        const double taxaComissao = query.value("comissaoLoja").toDouble() / 100;
        const double valorComissao = modelFluxoCaixa.data(z, "valor").toDouble();
        double valorAjustado = taxaComissao * (valorComissao - (valorComissao / totalVenda * frete));

        if (modelFluxoCaixa.data(0, "observacao").toString() == "FRETE") { valorAjustado = valorComissao * taxaComissao; }

        const int row = modelFluxoCaixa2.rowCount();
        modelFluxoCaixa2.insertRow(row);

        if (not modelFluxoCaixa2.setData(row, "contraParte", modelItem.data(0, "fornecedor"))) { return; }
        if (not modelFluxoCaixa2.setData(row, "dataEmissao", dataEmissao)) { return; }
        if (not modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text())) { return; }
        if (not modelFluxoCaixa2.setData(row, "idLoja", idLoja)) { return; }
        if (not modelFluxoCaixa2.setData(row, "dataPagamento", modelFluxoCaixa.data(z, "dataPagamento").toDate().addMonths(1))) { return; }
        if (not modelFluxoCaixa2.setData(row, "valor", valorAjustado)) { return; }
        if (not modelFluxoCaixa2.setData(row, "tipo", QString::number(i + 1) + ". Comissão")) { return; }
        if (not modelFluxoCaixa2.setData(row, "parcela", modelFluxoCaixa.data(z, "parcela").toString())) { return; }
        if (not modelFluxoCaixa2.setData(row, "comissao", 1)) { return; }
        if (not modelFluxoCaixa2.setData(row, "centroCusto", idLoja)) { return; }
        if (not modelFluxoCaixa2.setData(row, "grupo", "Comissão Representação")) { return; }
      }

      // calculo taxas cartao
      for (int z = 0, total = modelFluxoCaixa.rowCount(); z < total; ++z) {
        if (not modelFluxoCaixa.data(z, "tipo").toString().contains(QString::number(i + 1))) { continue; }
        if (modelFluxoCaixa.data(z, "status").toString() == "SUBSTITUIDO") { continue; }
        if (modelFluxoCaixa.data(z, "representacao").toBool()) { continue; }
        if (modelFluxoCaixa.data(z, "tipo").toString().contains("Conta Cliente")) { continue; }

        QSqlQuery query;
        query.prepare("SELECT taxa FROM forma_pagamento fp LEFT JOIN forma_pagamento_has_taxa fpt ON fp.idPagamento = fpt.idPagamento WHERE pagamento = :pagamento AND parcela = :parcela");
        query.bindValue(":pagamento", ui->widgetPgts->listComboPgt.at(i)->currentText());
        query.bindValue(":parcela", ui->widgetPgts->listComboParc.at(i)->currentText().remove("x").toInt());

        if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro buscando taxa: " + query.lastError().text()); }

        if (qFuzzyIsNull(query.value("taxa").toDouble())) { continue; }

        const double taxa = query.value("taxa").toDouble() / 100;

        const int row = modelFluxoCaixa2.rowCount();
        modelFluxoCaixa2.insertRow(row);
        if (not modelFluxoCaixa2.setData(row, "contraParte", "Administradora Cartão")) { return; }
        if (not modelFluxoCaixa2.setData(row, "dataEmissao", dataEmissao)) { return; }
        if (not modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text())) { return; }
        if (not modelFluxoCaixa2.setData(row, "idLoja", idLoja)) { return; }
        if (not modelFluxoCaixa2.setData(row, "dataPagamento", modelFluxoCaixa.data(z, "dataPagamento"))) { return; }
        if (not modelFluxoCaixa2.setData(row, "valor", taxa * modelFluxoCaixa.data(z, "valor").toDouble() * -1)) { return; }
        if (not modelFluxoCaixa2.setData(row, "tipo", QString::number(i + 1) + ". Taxa Cartão")) { return; }
        if (not modelFluxoCaixa2.setData(row, "parcela", modelFluxoCaixa.data(z, "parcela").toString())) { return; }
        if (not modelFluxoCaixa2.setData(row, "taxa", 1)) { return; }
        if (not modelFluxoCaixa2.setData(row, "centroCusto", idLoja)) { return; }
        if (not modelFluxoCaixa2.setData(row, "grupo", "Tarifas Cartão")) { return; }
      }
    }
  }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) { ui->tableFluxoCaixa->openPersistentEditor(row, "representacao"); }

  ui->tableFluxoCaixa->resizeColumnsToContents();
  ui->tableFluxoCaixa2->resizeColumnsToContents();
}

void Venda::on_pushButtonLimparPag_clicked() { resetarPagamentos(); }

void Venda::on_doubleSpinBoxTotal_valueChanged(const double total) {
  const double liq = ui->doubleSpinBoxSubTotalLiq->value();
  const double frete = ui->doubleSpinBoxFrete->value();

  unsetConnections();

  ui->doubleSpinBoxDescontoGlobal->setValue((liq + frete - total) / liq * 100);
  ui->doubleSpinBoxDescontoGlobalReais->setValue(liq + frete - total);

  resetarPagamentos();

  setConnections();
}

void Venda::on_checkBoxFreteManual_clicked(const bool checked) {
  ui->doubleSpinBoxFrete->setFrame(checked);
  ui->doubleSpinBoxFrete->setReadOnly(not checked);
  ui->doubleSpinBoxFrete->setButtonSymbols(checked ? QDoubleSpinBox::UpDownArrows : QDoubleSpinBox::NoButtons);

  ui->doubleSpinBoxFrete->setValue(ui->checkBoxFreteManual->isChecked() ? ui->doubleSpinBoxFrete->value() : qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete));
}

void Venda::on_doubleSpinBoxFrete_valueChanged(const double frete) {
  const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desconto = ui->doubleSpinBoxDescontoGlobalReais->value();

  unsetConnections();

  ui->doubleSpinBoxTotal->setValue(subTotalLiq - desconto + frete);

  setConnections();
}

void Venda::on_doubleSpinBoxDescontoGlobal_valueChanged(const double desconto) {
  unsetConnections();

  [=]() {
    for (int row = 0; row < modelItem.rowCount(); ++row) {
      if (not modelItem.setData(row, "descGlobal", desconto)) { return; }

      const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
      if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / 100)))) { return; }
    }

    calcPrecoGlobalTotal();

    const double liq = ui->doubleSpinBoxSubTotalLiq->value();
    const double frete = ui->doubleSpinBoxFrete->value();

    ui->doubleSpinBoxDescontoGlobalReais->setValue(liq * desconto / 100);
    ui->doubleSpinBoxTotal->setValue(liq * (1 - (desconto / 100)) + frete);

    resetarPagamentos();
  }();

  setConnections();
}

void Venda::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double desconto) {
  unsetConnections();

  [=]() { // REFAC: put this on totalChanged too
    const double liq = ui->doubleSpinBoxSubTotalLiq->value();

    for (int row = 0; row < modelItem.rowCount(); ++row) {
      if (not modelItem.setData(row, "descGlobal", desconto / liq * 100)) { return; }

      const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
      if (not modelItem.setData(row, "total", parcialDesc * (1 - (desconto / liq)))) { return; }
    }

    calcPrecoGlobalTotal();

    const double liq2 = ui->doubleSpinBoxSubTotalLiq->value();
    const double frete = ui->doubleSpinBoxFrete->value();

    ui->doubleSpinBoxDescontoGlobal->setValue(desconto / liq2 * 100);
    ui->doubleSpinBoxTotal->setValue(liq2 - desconto + frete);

    resetarPagamentos();
  }();

  setConnections();
}

void Venda::on_pushButtonImprimir_clicked() {
  Impressao impressao(data("idVenda").toString());
  impressao.print();
}

void Venda::successMessage() { qApp->enqueueInformation((tipo == Tipo::Atualizar) ? "Cadastro atualizado!" : "Venda cadastrada com sucesso!"); }

void Venda::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditVenda->text());
  excel.gerarExcel();
}

bool Venda::atualizarCredito() {
  double creditoRestante = ui->doubleSpinBoxCreditoTotal->value();
  bool update = false;

  for (int i = 0; i < ui->widgetPgts->listCheckBoxRep.size(); ++i) {
    if (ui->widgetPgts->listComboPgt.at(i)->currentText() == "Conta Cliente") {
      creditoRestante -= ui->widgetPgts->listDoubleSpinPgt.at(i)->value();
      update = true;
    }
  }

  if (update) {
    QSqlQuery query;
    query.prepare("UPDATE cliente SET credito = :credito WHERE idCliente = :idCliente");
    query.bindValue(":credito", creditoRestante);
    query.bindValue(":idCliente", ui->itemBoxCliente->getId());

    if (not query.exec()) { return qApp->enqueueError(false, "Erro atualizando crédito do cliente: " + query.lastError().text()); }
  }

  return true;
}

bool Venda::cadastrar() {
  currentRow = (tipo == Tipo::Atualizar) ? mapper.currentIndex() : model.rowCount();

  if (currentRow == -1) { return qApp->enqueueError(false, "Erro linha -1"); }

  if (tipo == Tipo::Atualizar) { return qApp->enqueueError(false, "Erro venda atualizar"); }

  if (not generateId()) { return false; }

  if (not model.insertRow(currentRow)) { return qApp->enqueueError(false, "Erro inserindo linha na tabela"); }

  if (not savingProcedures()) { return false; }

  if (not atualizarCredito()) { return false; }

  // inserir rt em contas_pagar

  QSqlQuery query;

  if (ui->checkBoxPontuacaoPadrao->isChecked()) {
    const QDate date = ui->dateTimeEdit->date();
    const double valor = (ui->doubleSpinBoxSubTotalLiq->value() - ui->doubleSpinBoxDescontoGlobalReais->value()) * ui->doubleSpinBoxPontuacao->value() / 100;

    query.prepare("INSERT INTO conta_a_pagar_has_pagamento (dataEmissao, contraParte, idLoja, centroCusto, valor, tipo, dataPagamento, grupo) VALUES (:dataEmissao, :contraParte, :idLoja, "
                  ":centroCusto, :valor, :tipo, :dataPagamento, :grupo)");
    query.bindValue(":dataEmissao", date);
    query.bindValue(":contraParte", ui->itemBoxProfissional->text());
    query.bindValue(":idLoja", idLoja);
    query.bindValue(":centroCusto", idLoja);
    query.bindValue(":valor", valor);
    query.bindValue(":tipo", "1. Dinheiro");
    // 01-15 paga dia 30, 16-30 paga prox dia 15
    query.bindValue(":dataPagamento", date.day() <= 15 ? QDate(date.year(), date.month(), 30 > date.daysInMonth() ? date.daysInMonth() : 30) : QDate(date.year(), date.month() + 1, 15));
    query.bindValue(":grupo", "RT's");

    if (not query.exec()) { return qApp->enqueueError(false, "Erro cadastrando pontuação: " + query.lastError().text()); }
  }
  // -------------------------------------------------------------------------

  if (not model.submitAll()) { return false; }

  primaryId = ui->lineEditVenda->text();

  if (primaryId.isEmpty()) { return qApp->enqueueError(false, "Id vazio!"); }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    if (not modelFluxoCaixa.setData(row, "idVenda", ui->lineEditVenda->text())) { return false; }
  }

  if (not modelFluxoCaixa.submitAll()) { return false; }

  for (int row = 0; row < modelFluxoCaixa2.rowCount(); ++row) {
    if (not modelFluxoCaixa2.setData(row, "idVenda", ui->lineEditVenda->text())) { return false; }
  }

  if (not modelFluxoCaixa2.submitAll()) { return false; }

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (not modelItem.setData(row, "idVenda", ui->lineEditVenda->text())) { return false; }
  }

  if (not modelItem.submitAll()) { return false; }

  // TODO: dont reuse query
  query.prepare("SELECT p.idEstoque, vp.idVendaProduto, vp.quant FROM venda_has_produto vp LEFT JOIN produto p on p.idProduto = vp.idProduto WHERE vp.idVenda = :idVenda AND vp.estoque > 0");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro buscando produtos estoque: " + query.lastError().text()); }

  while (query.next()) {
    auto *estoque = new Estoque(query.value("idEstoque").toString(), false, this);

    if (not estoque->criarConsumo(query.value("idVendaProduto").toInt(), query.value("quant").toDouble())) { return false; }
  }

  if (not atualizaQuantEstoque()) { return false; }

  query.prepare("UPDATE orcamento SET status = 'FECHADO' WHERE idOrcamento = :idOrcamento");
  query.bindValue(":idOrcamento", ui->lineEditIdOrcamento->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro marcando orçamento como 'FECHADO': " + query.lastError().text()); }

  return true;
}

bool Venda::atualizaQuantEstoque() {
  QSqlQuery query;
  query.prepare("UPDATE produto p, view_estoque2 v SET p.estoqueRestante = v.restante, descontinuado = IF(v.restante = 0, TRUE, FALSE) WHERE p.idEstoque = v.idEstoque AND p.idProduto = :idProduto");

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (modelItem.data(row, "estoque").toInt() > 0) {
      query.bindValue(":idProduto", modelItem.data(row, "idProduto"));

      if (not query.exec()) { return qApp->enqueueError(false, "Erro atualizando quant. estoque: " + query.lastError().text()); }
    }
  }

  return true;
}

bool Venda::cancelamento() {
  const QString idOrcamento = ui->lineEditIdOrcamento->text();

  QSqlQuery query;

  if (not idOrcamento.isEmpty()) {
    query.prepare("UPDATE orcamento SET status = 'ATIVO' WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idOrcamento", idOrcamento);

    if (not query.exec()) { return qApp->enqueueError(false, "Erro reativando orçamento: " + query.lastError().text()); }
  }

  // -------------------------------------------------------------------------

  query.prepare("UPDATE venda SET status = 'CANCELADO' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro marcando venda como cancelada: " + query.lastError().text()); }

  // -------------------------------------------------------------------------

  query.prepare("UPDATE pedido_fornecedor_has_produto SET idVenda = NULL, idVendaProduto = NULL WHERE idVendaProduto IN (SELECT idVendaProduto FROM venda_has_produto WHERE idVenda = :idVenda) AND "
                "status NOT IN ('CANCELADO', 'DEVOLVIDO')");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro removendo vínculo da compra: " + query.lastError().text()); }

  // -------------------------------------------------------------------------

  query.prepare("UPDATE venda_has_produto SET status = 'CANCELADO' WHERE idVenda = :idVenda AND status NOT IN ('CANCELADO', 'DEVOLVIDO')");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro marcando produtos da venda como cancelados: " + query.lastError().text()); }

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    if (modelFluxoCaixa.data(row, "tipo").toString().contains("Conta Cliente")) {
      if (modelFluxoCaixa.data(row, "status").toString() == "CANCELADO") { continue; }
      const double credito = modelFluxoCaixa.data(row, "valor").toDouble();

      query.prepare("UPDATE cliente SET credito = credito + :valor WHERE idCliente = :idCliente");
      query.bindValue(":valor", credito);
      query.bindValue(":idCliente", model.data(0, "idCliente"));

      if (not query.exec()) { return qApp->enqueueError(false, "Erro voltando credito do cliente: " + query.lastError().text()); }
    }
  }

  // TODO: 0nao deixar cancelar se tiver ocorrido algum evento de conta

  query.prepare("UPDATE conta_a_receber_has_pagamento SET status = 'CANCELADO' WHERE idVenda = :idVenda");
  query.bindValue(":idVenda", ui->lineEditVenda->text());

  if (not query.exec()) { return qApp->enqueueError(false, "Erro marcando contas como canceladas: " + query.lastError().text()); }

  return true;
}

void Venda::on_pushButtonCancelamento_clicked() {
  // TODO: perguntar e salvar motivo do cancelamento

  bool ok = true;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (modelItem.data(row, "status").toString() != "PENDENTE") {
      ok = false;
      break;
    }
  }

  if (not ok) { return qApp->enqueueError("Um ou mais produtos não estão pendentes!"); }

  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar venda");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() != QMessageBox::Yes) { return; }

  if (not qApp->startTransaction()) { return; }

  if (not cancelamento()) { return qApp->rollbackTransaction(); }

  if (not qApp->endTransaction()) { return; }

  const QString idOrcamento = ui->lineEditIdOrcamento->text();

  qApp->enqueueInformation(idOrcamento.isEmpty() ? "Não havia Orçamento associado para ativar!" : "Orçamento " + idOrcamento + " reativado!");

  qApp->enqueueInformation("Venda cancelada!");
  close();
}

bool Venda::generateId() {
  const auto siglaLoja = UserSession::fromLoja("sigla", ui->itemBoxVendedor->text());

  if (not siglaLoja) { return qApp->enqueueError(false, "Erro buscando sigla da loja!"); }

  const auto idLoja = UserSession::fromLoja("loja.idLoja", ui->itemBoxVendedor->text());

  if (not idLoja) { return qApp->enqueueError(false, "Erro buscando idLoja!"); }

  QString id = siglaLoja.value().toString() + "-" + ui->dateTimeEdit->date().toString("yy");

  QSqlQuery query;
  query.prepare("SELECT MAX(idVenda) AS idVenda FROM venda WHERE idVenda LIKE :id");
  query.bindValue(":id", id + "%");

  if (not query.exec()) { return qApp->enqueueError(false, "Erro na query: " + query.lastError().text()); }

  const int last = query.first() ? query.value("idVenda").toString().remove(id).leftRef(4).toInt() : 0;

  id += QString("%1").arg(last + 1, 4, 10, QChar('0'));

  if (id.size() != 11) { return qApp->enqueueError(false, "Ocorreu algum erro ao gerar id: " + id); }

  id += representacao ? "R" : "";

  ui->lineEditVenda->setText(id);

  return true;
}

void Venda::on_pushButtonDevolucao_clicked() {
  auto *devolucao = new Devolucao(data("idVenda").toString(), this);
  connect(devolucao, &Devolucao::finished, [=] { this->viewRegisterById(ui->lineEditVenda->text()); });
  devolucao->show();
}

void Venda::on_tableFluxoCaixa_entered(const QModelIndex &) { ui->tableFluxoCaixa->resizeColumnsToContents(); }

void Venda::on_tableFluxoCaixa2_entered(const QModelIndex &) { ui->tableFluxoCaixa2->resizeColumnsToContents(); }

void Venda::on_dateTimeEdit_dateTimeChanged(const QDateTime &) { resetarPagamentos(); }

void Venda::on_pushButtonFreteLoja_clicked() {
  if (qFuzzyIsNull(ui->doubleSpinBoxFrete->value())) { return qApp->enqueueError("Não há frete!"); }

  resetarPagamentos();

  on_pushButtonAdicionarPagamento_clicked();

  ui->widgetPgts->listCheckBoxRep.at(0)->setChecked(false);
  ui->widgetPgts->listLinePgt.at(0)->setText("Frete");
  ui->widgetPgts->listLinePgt.at(0)->setReadOnly(true);
  ui->widgetPgts->listDoubleSpinPgt.at(0)->setValue(ui->doubleSpinBoxFrete->value());
  ui->widgetPgts->listDoubleSpinPgt.at(0)->setReadOnly(true);

  on_pushButtonAdicionarPagamento_clicked();
}

void Venda::on_pushButtonPgtLoja_clicked() {
  LoginDialog dialog(LoginDialog::Tipo::Autorizacao, this);

  if (dialog.exec() == QDialog::Rejected) { return; }

  Q_FOREACH (auto item, ui->widgetPgts->listCheckBoxRep) { item->setChecked(false); }
}

void Venda::setFinanceiro() {
  ui->groupBoxFinanceiro->show();
  ui->tableFluxoCaixa2->show();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR" and UserSession::tipoUsuario() != "GERENTE DEPARTAMENTO") { ui->pushButtonCorrigirFluxo->hide(); }

  ui->frameButtons->hide();
  financeiro = true;
}

bool Venda::financeiroSalvar() {
  // REFAC: if false return
  atualizarCredito();

  if (not model.setData(mapper.currentIndex(), "statusFinanceiro", ui->comboBoxFinanceiro->currentText())) { return false; }

  if (not model.setData(mapper.currentIndex(), "dataFinanceiro", QDateTime::currentDateTime())) { return false; }

  if (not model.submitAll()) { return false; }

  if (not modelFluxoCaixa.submitAll()) { return false; }

  if (not modelFluxoCaixa2.submitAll()) { return false; }

  return true;
}

void Venda::on_pushButtonFinanceiroSalvar_clicked() {
  if (not verifyFields()) { return; }

  if (not qApp->startTransaction()) { return; }

  if (not financeiroSalvar()) { return qApp->rollbackTransaction(); }

  if (not qApp->endTransaction()) { return; }

  qApp->enqueueInformation("Dados salvos com sucesso!");
  close();
}

void Venda::on_pushButtonCorrigirFluxo_clicked() {
  QSqlQuery queryPag;
  queryPag.prepare("SELECT credito FROM cliente WHERE idCliente = :idCliente");
  queryPag.bindValue(":idCliente", data("idCliente"));

  if (not queryPag.exec() or not queryPag.first()) { return qApp->enqueueError("Erro lendo credito cliente: " + queryPag.lastError().text()); }

  double credito = queryPag.value("credito").toDouble();

  for (int row = 0; row < modelFluxoCaixa.rowCount(); ++row) {
    if (modelFluxoCaixa.data(row, "tipo").toString().contains("Conta Cliente")) { credito += modelFluxoCaixa.data(row, "valor").toDouble(); }
  }

  ui->doubleSpinBoxCreditoTotal->setValue(credito);

  // -------------------------------------------------------------------------

  if (not data("representacao").toBool()) {
    ui->pushButtonFreteLoja->hide();
    ui->pushButtonPgtLoja->hide();
    ui->tableFluxoCaixa->hideColumn("representacao");
  }

  ui->framePagamentos_2->show();
  resetarPagamentos();

  correcao = true;
}

void Venda::on_checkBoxPontuacaoPadrao_toggled(bool checked) {
  if (checked) {
    QSqlQuery query;
    query.prepare("SELECT comissao FROM profissional WHERE idProfissional = :idProfissional");
    query.bindValue(":idProfissional", ui->itemBoxProfissional->getId());

    if (not query.exec() or not query.first()) { return qApp->enqueueError("Erro buscando pontuação: " + query.lastError().text()); }

    ui->checkBoxPontuacaoIsento->setChecked(false);
    ui->doubleSpinBoxPontuacao->setMaximum(query.value("comissao").toDouble());
    ui->doubleSpinBoxPontuacao->setValue(query.value("comissao").toDouble());
    ui->doubleSpinBoxPontuacao->setEnabled(true);
  }

  // TODO: 5criar linha em contas_pagar de comissao/rt
}

void Venda::on_checkBoxPontuacaoIsento_toggled(bool checked) {
  if (checked) {
    ui->checkBoxPontuacaoPadrao->setChecked(false);
    ui->doubleSpinBoxPontuacao->setValue(0);
    ui->doubleSpinBoxPontuacao->setDisabled(true);
  } else {
    ui->doubleSpinBoxPontuacao->setEnabled(true);
  }
}

void Venda::on_checkBoxRT_toggled(bool checked) {
  for (auto &item : ui->frameRT->findChildren<QWidget *>()) { item->setVisible(checked); }

  ui->checkBoxRT->setText(checked ? "Pontuação" : "");
}

void Venda::on_itemBoxProfissional_textChanged(const QString &) { on_checkBoxPontuacaoPadrao_toggled(ui->checkBoxPontuacaoPadrao->isChecked()); }

void Venda::on_pushButtonAdicionarPagamento_clicked() {
  ui->widgetPgts->adicionarPagamentoVenda(representacao, ui->lineEditIdOrcamento->text(), ui->doubleSpinBoxCreditoTotal->value(), ui->doubleSpinBoxTotal->value() - ui->doubleSpinBoxTotalPag->value());

  connect(ui->widgetPgts, &WidgetPagamentos::montarFluxoCaixa, this, &Venda::montarFluxoCaixa);
  connect(ui->widgetPgts, &WidgetPagamentos::valueChanged, this, &Venda::on_doubleSpinBoxPgt_valueChanged);

  on_doubleSpinBoxPgt_valueChanged();
}

void Venda::on_doubleSpinBoxTotalPag_valueChanged(double) {
  if (ui->widgetPgts->listDoubleSpinPgt.size() <= 1) { return; }

  double sumWithoutLast = 0;

  Q_FOREACH (const auto &item, ui->widgetPgts->listDoubleSpinPgt) {
    item->setMaximum(ui->doubleSpinBoxTotal->value());
    sumWithoutLast += item->value();
  }

  const auto lastSpinBox = ui->widgetPgts->listDoubleSpinPgt.at(ui->widgetPgts->listDoubleSpinPgt.size() - 1);

  sumWithoutLast -= lastSpinBox->value();

  lastSpinBox->setMaximum(ui->doubleSpinBoxTotal->value() - sumWithoutLast);
  lastSpinBox->setValue(ui->doubleSpinBoxTotal->value() - sumWithoutLast);
}

bool Venda::copiaProdutosOrcamento() {
  QSqlQuery queryProdutos;
  queryProdutos.prepare("SELECT * FROM orcamento_has_produto WHERE idOrcamento = :idOrcamento");
  queryProdutos.bindValue(":idOrcamento", ui->lineEditIdOrcamento->text());

  if (not queryProdutos.exec()) { return qApp->enqueueError(false, "Erro buscando produtos: " + queryProdutos.lastError().text()); }

  while (queryProdutos.next()) {
    const int rowItem = modelItem.rowCount();
    modelItem.insertRow(rowItem);

    for (int column = 0, columnCount = queryProdutos.record().count(); column < columnCount; ++column) {
      const QString field = queryProdutos.record().fieldName(column);

      if (field == "created") { continue; }
      if (field == "lastUpdated") { continue; }
      if (modelItem.fieldIndex(field) == -1) { continue; }

      if (not modelItem.setData(rowItem, field, queryProdutos.value(field))) { return false; }
    }

    if (not modelItem.setData(rowItem, "status", modelItem.data(rowItem, "estoque").toBool() ? "ESTOQUE" : "PENDENTE")) { return false; }
  }

  return true;
}

// TODO: 0hide 'nfe' field from tables that use conta_a_receber
// TODO: 0nao gerar RT quando o total for zero (e apagar os zerados quando nao houver profissional)
// TODO: 0se o pedido estiver cancelado ou devolvido bloquear os botoes correspondentes
// TODO: 0no corrigir fluxo esta mostrando os botoes de 'frete pago a loja' e 'pagamento total a loja' em pedidos que nao
// sao de representacao
// TODO: 5verificar se um pedido nao deveria ter seu 'statusFinanceiro' alterado para 'liberado'
// ao ter todos os pagamentos recebidos ('status' e 'statusFinanceiro' deveriam ser vinculados?)
// TODO: 0quando for 'MATR' nao criar fluxo caixa
// NOTE: prazoEntrega por produto
// NOTE: bloquear desconto maximo por classe de funcionario
// TODO: 2no caso de reposicao colocar formas de pagamento diferenciado ou nao usar pagamento?
// REFAC: em vez de ter uma caixinha 'un', concatenar em 'quant', 'minimo' e 'un/cx'
// TODO: usar coluna 'idRelacionado' para vincular a linha quebrada/reposicao com a correspondente
