#include "orcamento.h"
#include "ui_orcamento.h"

#include "application.h"
#include "baixaorcamento.h"
#include "cadastrocliente.h"
#include "cadastroprofissional.h"
#include "calculofrete.h"
#include "doubledelegate.h"
#include "excel.h"
#include "file.h"
#include "log.h"
#include "pdf.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "searchdialogproxymodel.h"
#include "usersession.h"
#include "venda.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSqlError>
#include <QtMath>

Orcamento::Orcamento(QWidget *parent) : RegisterDialog("orcamento", "idOrcamento", parent), ui(new Ui::Orcamento) {
  ui->setupUi(this);

  setupTables();
  connectLineEditsToDirty();
  setItemBoxes();
  setupMapper();
  newRegister();

  if (UserSession::tipoUsuario == "ADMINISTRADOR" or UserSession::tipoUsuario == "ADMINISTRATIVO") {
    ui->dataEmissao->setReadOnly(false);
    ui->dataEmissao->setCalendarPopup(true);
  }

  if (UserSession::tipoUsuario == "VENDEDOR") { buscarParametrosFrete(); }

  setConnections();
}

Orcamento::~Orcamento() { delete ui; }

void Orcamento::setItemBoxes() {
  ui->itemBoxCliente->setRegisterDialog(new CadastroCliente(this));
  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));
  ui->itemBoxConsultor->setSearchDialog(SearchDialog::vendedor(this));
  ui->itemBoxEndereco->setSearchDialog(SearchDialog::enderecoCliente(this));
  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(false, false, false, false, this));
  ui->itemBoxProfissional->setRegisterDialog(new CadastroProfissional(this));
  ui->itemBoxProfissional->setSearchDialog(SearchDialog::profissional(true, this));
  ui->itemBoxVendedor->setSearchDialog(SearchDialog::vendedor(this));
}

void Orcamento::show() {
  RegisterDialog::show();

  ui->groupBoxInfo->adjustSize();
  ui->groupBoxDados->adjustSize();

  ui->groupBoxInfo->setMaximumHeight(ui->groupBoxInfo->height());
  ui->groupBoxDados->setMaximumHeight(ui->groupBoxDados->height());
}

void Orcamento::on_tableProdutos_clicked(const QModelIndex &index) {
  if (isReadOnly) { return; }

  if (not index.isValid()) { return novoItem(); }

  ui->pushButtonAtualizarItem->show();
  ui->pushButtonRemoverItem->show();
  ui->pushButtonLimparSelecao->show();

  ui->pushButtonAdicionarItem->hide();

  mapperItem.setCurrentModelIndex(index);
  currentRowItem = index.row();
}

void Orcamento::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked, connectionType);
  connect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled, connectionType);
  connect(ui->dataEmissao, &QDateTimeEdit::dateChanged, this, &Orcamento::on_dataEmissao_dateChanged, connectionType);
  connect(ui->doubleSpinBoxCaixas, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged, connectionType);
  connect(ui->doubleSpinBoxDesconto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged, connectionType);
  connect(ui->doubleSpinBoxDescontoGlobal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged, connectionType);
  connect(ui->doubleSpinBoxDescontoGlobalReais, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged, connectionType);
  connect(ui->doubleSpinBoxFrete, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged, connectionType);
  connect(ui->doubleSpinBoxQuant, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged, connectionType);
  connect(ui->doubleSpinBoxTotal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged, connectionType);
  connect(ui->doubleSpinBoxTotalItem, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged, connectionType);
  connect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged, connectionType);
  connect(ui->itemBoxProduto, &ItemBox::idChanged, this, &Orcamento::on_itemBoxProduto_idChanged, connectionType);
  connect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged, connectionType);
  connect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked, connectionType);
  connect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked, connectionType);
  connect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked, connectionType);
  connect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked, connectionType);
  connect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked, connectionType);
  connect(ui->pushButtonCalculadora, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalculadora_clicked, connectionType);
  connect(ui->pushButtonCalcularFrete, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalcularFrete_clicked, connectionType);
  connect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked, connectionType);
  connect(ui->pushButtonGerarPdf, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarPdf_clicked, connectionType);
  connect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked, connectionType);
  connect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::novoItem, connectionType);
  connect(ui->pushButtonModelo3d, &QPushButton::clicked, this, &Orcamento::on_pushButtonModelo3d_clicked, connectionType);
  connect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked, connectionType);
  connect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked, connectionType);
  connect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked, connectionType);
}

void Orcamento::unsetConnections() {
  disconnect(ui->checkBoxFreteManual, &QCheckBox::clicked, this, &Orcamento::on_checkBoxFreteManual_clicked);
  disconnect(ui->checkBoxRepresentacao, &QCheckBox::toggled, this, &Orcamento::on_checkBoxRepresentacao_toggled);
  disconnect(ui->dataEmissao, &QDateTimeEdit::dateChanged, this, &Orcamento::on_dataEmissao_dateChanged);
  disconnect(ui->doubleSpinBoxCaixas, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxCaixas_valueChanged);
  disconnect(ui->doubleSpinBoxDesconto, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDesconto_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged);
  disconnect(ui->doubleSpinBoxDescontoGlobalReais, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged);
  disconnect(ui->doubleSpinBoxFrete, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxFrete_valueChanged);
  disconnect(ui->doubleSpinBoxQuant, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxQuant_valueChanged);
  disconnect(ui->doubleSpinBoxTotal, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotal_valueChanged);
  disconnect(ui->doubleSpinBoxTotalItem, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Orcamento::on_doubleSpinBoxTotalItem_valueChanged);
  disconnect(ui->itemBoxCliente, &ItemBox::textChanged, this, &Orcamento::on_itemBoxCliente_textChanged);
  disconnect(ui->itemBoxProduto, &ItemBox::idChanged, this, &Orcamento::on_itemBoxProduto_idChanged);
  disconnect(ui->itemBoxVendedor, &ItemBox::textChanged, this, &Orcamento::on_itemBoxVendedor_textChanged);
  disconnect(ui->pushButtonAdicionarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAdicionarItem_clicked);
  disconnect(ui->pushButtonApagarOrc, &QPushButton::clicked, this, &Orcamento::on_pushButtonApagarOrc_clicked);
  disconnect(ui->pushButtonAtualizarItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarItem_clicked);
  disconnect(ui->pushButtonAtualizarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonAtualizarOrcamento_clicked);
  disconnect(ui->pushButtonCadastrarOrcamento, &QPushButton::clicked, this, &Orcamento::on_pushButtonCadastrarOrcamento_clicked);
  disconnect(ui->pushButtonCalculadora, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalculadora_clicked);
  disconnect(ui->pushButtonCalcularFrete, &QPushButton::clicked, this, &Orcamento::on_pushButtonCalcularFrete_clicked);
  disconnect(ui->pushButtonGerarExcel, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarExcel_clicked);
  disconnect(ui->pushButtonGerarPdf, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarPdf_clicked);
  disconnect(ui->pushButtonGerarVenda, &QPushButton::clicked, this, &Orcamento::on_pushButtonGerarVenda_clicked);
  disconnect(ui->pushButtonLimparSelecao, &QPushButton::clicked, this, &Orcamento::novoItem);
  disconnect(ui->pushButtonModelo3d, &QPushButton::clicked, this, &Orcamento::on_pushButtonModelo3d_clicked);
  disconnect(ui->pushButtonRemoverItem, &QPushButton::clicked, this, &Orcamento::on_pushButtonRemoverItem_clicked);
  disconnect(ui->pushButtonReplicar, &QPushButton::clicked, this, &Orcamento::on_pushButtonReplicar_clicked);
  disconnect(ui->tableProdutos, &TableView::clicked, this, &Orcamento::on_tableProdutos_clicked);
}

bool Orcamento::viewRegister() {
  unsetConnections();

  auto load = [&] {
    if (not RegisterDialog::viewRegister()) { return false; }

    //-----------------------------------------------------------------

    modelItem.setFilter("idOrcamento = '" + model.data(0, "idOrcamento").toString() + "'");

    modelItem.select();

    //-----------------------------------------------------------------

    buscarParametrosFrete();

    novoItem();

    const int validade = data("validade").toInt();
    ui->spinBoxValidade->setMaximum(validade);
    ui->spinBoxValidade->setValue(validade);

    const QString status = data("status").toString();

    if (status == "FECHADO" or status == "PERDIDO") { ui->pushButtonApagarOrc->hide(); }

    if (status == "PERDIDO" or status == "CANCELADO") {
      ui->labelBaixa->show();
      ui->plainTextEditBaixa->show();
    }

    if (status == "ATIVO") { ui->pushButtonReplicar->hide(); }

    const bool expirado = ui->dataEmissao->dateTime().addDays(data("validade").toInt()).date() < qApp->serverDate();

    if (expirado or status != "ATIVO") {
      isReadOnly = true;

      ui->pushButtonReplicar->show();

      ui->frameProduto->hide();

      ui->pushButtonGerarVenda->hide();
      ui->pushButtonAtualizarOrcamento->hide();

      ui->checkBoxFreteManual->setDisabled(true);

      ui->itemBoxCliente->setReadOnlyItemBox(true);
      ui->itemBoxEndereco->setReadOnlyItemBox(true);
      ui->itemBoxProduto->setReadOnlyItemBox(true);
      ui->itemBoxProfissional->setReadOnlyItemBox(true);
      ui->itemBoxVendedor->setReadOnlyItemBox(true);

      ui->dataEmissao->setReadOnly(true);
      ui->doubleSpinBoxDesconto->setReadOnly(true);
      ui->doubleSpinBoxDescontoGlobal->setReadOnly(true);
      ui->doubleSpinBoxDescontoGlobalReais->setReadOnly(true);
      ui->doubleSpinBoxFrete->setReadOnly(true);
      ui->doubleSpinBoxQuant->setReadOnly(true);
      ui->doubleSpinBoxSubTotalBruto->setReadOnly(true);
      ui->doubleSpinBoxSubTotalLiq->setReadOnly(true);
      ui->doubleSpinBoxTotal->setReadOnly(true);
      ui->doubleSpinBoxTotalItem->setReadOnly(true);
      ui->plainTextEditObs->setReadOnly(true);
      ui->spinBoxPrazoEntrega->setReadOnly(true);

      ui->dataEmissao->setReadOnly(true);
      ui->dataEmissao->setCalendarPopup(false);

      ui->spinBoxValidade->setReadOnly(true);
      ui->spinBoxValidade->setButtonSymbols(QSpinBox::NoButtons);

      ui->doubleSpinBoxDescontoGlobal->setButtonSymbols(QDoubleSpinBox::NoButtons);
      ui->doubleSpinBoxDescontoGlobalReais->setButtonSymbols(QDoubleSpinBox::NoButtons);
      ui->doubleSpinBoxFrete->setButtonSymbols(QDoubleSpinBox::NoButtons);
      ui->doubleSpinBoxTotal->setButtonSymbols(QDoubleSpinBox::NoButtons);
    } else {
      ui->pushButtonGerarVenda->show();
    }

    const bool freteManual = ui->checkBoxFreteManual->isChecked();

    canChangeFrete = freteManual;

    ui->doubleSpinBoxFrete->setMinimum(freteManual ? 0 : ui->doubleSpinBoxFrete->value());

    ui->doubleSpinBoxDescontoGlobalReais->setMaximum(ui->doubleSpinBoxSubTotalLiq->value());

    if (ui->checkBoxRepresentacao->isChecked()) { ui->itemBoxProduto->setRepresentacao(true); }

    if (not data("replicadoDe").toString().isEmpty()) {
      ui->labelReplicaDe->show();
      ui->lineEditReplicaDe->show();
    }

    if (not data("replicadoEm").toString().isEmpty()) {
      ui->labelReplicadoEm->show();
      ui->lineEditReplicadoEm->show();
    }

    if (data("idUsuarioConsultor").toInt() != 0) {
      ui->labelConsultor->show();
      ui->itemBoxConsultor->show();
    } else {
      ui->labelConsultor->hide();
      ui->itemBoxConsultor->hide();
    }

    if (ui->lineEditOrcamento->text() != "Auto gerado") {
      const QString idLoja = UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()).toString();
      ui->itemBoxVendedor->setFilter("idLoja = " + idLoja);
    }

    return true;
  }();

  setConnections();

  return load;
}

void Orcamento::novoItem() {
  ui->pushButtonAdicionarItem->show();

  ui->pushButtonAtualizarItem->hide();
  ui->pushButtonRemoverItem->hide();
  ui->pushButtonLimparSelecao->hide();

  ui->itemBoxProduto->clear();
  ui->tableProdutos->clearSelection();

  // -----------------------

  ui->doubleSpinBoxCaixas->setDisabled(true);
  ui->doubleSpinBoxDesconto->setDisabled(true);
  ui->doubleSpinBoxQuant->setDisabled(true);
  ui->doubleSpinBoxTotalItem->setDisabled(true);
  ui->lineEditPrecoUn->setDisabled(true);
  ui->lineEditUn->setDisabled(true);
  ui->spinBoxMinimo->setDisabled(true);
  ui->spinBoxQuantCx->setDisabled(true);

  ui->doubleSpinBoxCaixas->setSingleStep(1.);
  ui->doubleSpinBoxQuant->setSingleStep(1.);

  ui->doubleSpinBoxCaixas->clear();
  ui->doubleSpinBoxDesconto->clear();
  ui->doubleSpinBoxQuant->clear();
  ui->doubleSpinBoxTotalItem->clear();
  ui->lineEditCodComercial->clear();
  ui->lineEditEstoque->clear();
  ui->lineEditFormComercial->clear();
  ui->lineEditFornecedor->clear();
  ui->lineEditObs->clear();
  ui->lineEditPrecoUn->clear();
  ui->lineEditUn->clear();
  ui->spinBoxMinimo->clear();
  ui->spinBoxQuantCx->clear();
}

void Orcamento::setupMapper() {
  addMapping(ui->checkBoxFreteManual, "freteManual");
  addMapping(ui->checkBoxRepresentacao, "representacao");
  addMapping(ui->dataEmissao, "data");
  addMapping(ui->doubleSpinBoxDescontoGlobal, "descontoPorc");
  addMapping(ui->doubleSpinBoxDescontoGlobalReais, "descontoReais");
  addMapping(ui->doubleSpinBoxFrete, "frete");
  addMapping(ui->doubleSpinBoxSubTotalBruto, "subTotalBru");
  addMapping(ui->doubleSpinBoxSubTotalLiq, "subTotalLiq");
  addMapping(ui->doubleSpinBoxTotal, "total");
  addMapping(ui->itemBoxCliente, "idCliente", "id");
  addMapping(ui->itemBoxEndereco, "idEnderecoEntrega", "id");
  addMapping(ui->itemBoxProfissional, "idProfissional", "id");
  addMapping(ui->itemBoxVendedor, "idUsuario", "id");
  addMapping(ui->itemBoxConsultor, "idUsuarioConsultor", "id");
  addMapping(ui->lineEditOrcamento, "idOrcamento");
  addMapping(ui->lineEditReplicaDe, "replicadoDe");
  addMapping(ui->lineEditReplicadoEm, "replicadoEm");
  addMapping(ui->plainTextEditBaixa, "observacaoCancelamento");
  addMapping(ui->plainTextEditObs, "observacao");
  addMapping(ui->spinBoxPrazoEntrega, "prazoEntrega");
  addMapping(ui->spinBoxValidade, "validade");

  mapperItem.setModel(ui->tableProdutos->model());
  mapperItem.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);

  mapperItem.addMapping(ui->itemBoxProduto, modelItem.fieldIndex("idProduto"), "id");
  mapperItem.addMapping(ui->lineEditCodComercial, modelItem.fieldIndex("codComercial"));
  mapperItem.addMapping(ui->lineEditFormComercial, modelItem.fieldIndex("formComercial"));
  mapperItem.addMapping(ui->lineEditObs, modelItem.fieldIndex("obs"));
  mapperItem.addMapping(ui->lineEditPrecoUn, modelItem.fieldIndex("prcUnitario"), "value"); // TODO: replace this with a simple doubleSpinbox?
  mapperItem.addMapping(ui->lineEditUn, modelItem.fieldIndex("un"));
  mapperItem.addMapping(ui->doubleSpinBoxQuant, modelItem.fieldIndex("quant"));
  mapperItem.addMapping(ui->doubleSpinBoxDesconto, modelItem.fieldIndex("desconto"));
}

void Orcamento::registerMode() {
  ui->pushButtonCadastrarOrcamento->show();

  ui->itemBoxConsultor->hide();
  ui->labelBaixa->hide();
  ui->labelConsultor->hide();
  ui->labelReplicaDe->hide();
  ui->labelReplicadoEm->hide();
  ui->lineEditReplicaDe->hide();
  ui->lineEditReplicadoEm->hide();
  ui->plainTextEditBaixa->hide();
  ui->pushButtonAtualizarOrcamento->hide();
  ui->pushButtonCalcularFrete->hide();
  ui->pushButtonReplicar->hide();

  ui->itemBoxConsultor->setReadOnlyItemBox(true);

  ui->pushButtonApagarOrc->setDisabled(true);
  ui->pushButtonGerarExcel->setDisabled(true);
  ui->pushButtonGerarPdf->setDisabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);
  //  ui->itemBoxEndereco->setDisabled(true);
}

void Orcamento::updateMode() {
  ui->itemBoxEndereco->show();
  ui->pushButtonAtualizarOrcamento->show();
  ui->pushButtonReplicar->show();

  ui->pushButtonCadastrarOrcamento->hide();

  ui->pushButtonApagarOrc->setEnabled(true);
  ui->pushButtonGerarExcel->setEnabled(true);
  ui->pushButtonGerarPdf->setEnabled(true);
  ui->pushButtonGerarVenda->setEnabled(true);

  ui->spinBoxValidade->setReadOnly(true);
  ui->checkBoxRepresentacao->setDisabled(true);

  ui->lineEditReplicaDe->setReadOnly(true);
  ui->lineEditReplicadoEm->setReadOnly(true);
}

bool Orcamento::newRegister() {
  if (not RegisterDialog::newRegister()) { return false; }

  ui->lineEditOrcamento->setText("Auto gerado");
  ui->dataEmissao->setDate(qApp->serverDate());
  on_dataEmissao_dateChanged(ui->dataEmissao->date());
  ui->spinBoxValidade->setValue(7);
  novoItem();

  return true;
}

void Orcamento::removeItem() {
  if (modelItem.rowCount() == 1 and ui->lineEditOrcamento->text() != "Auto gerado") { throw RuntimeError("Não pode cadastrar um orçamento sem itens!", this); }

  unsetConnections();

  try {
    [&] {
      if (ui->lineEditOrcamento->text() != "Auto gerado") { save(true); } // save pending rows before submitAll

      if (not modelItem.removeRow(currentRowItem)) { throw RuntimeException("Erro removendo linha: " + modelItem.lastError().text()); }

      if (ui->lineEditOrcamento->text() != "Auto gerado") {
        modelItem.submitAll();
        calcPrecoGlobalTotal();
        save(true);
      } else {
        calcPrecoGlobalTotal();
      }

      if (modelItem.rowCount() == 0) {
        if (ui->lineEditOrcamento->text() == "Auto gerado") { ui->checkBoxRepresentacao->setEnabled(true); }

        ui->itemBoxProduto->setFornecedorRep("");
      }

      novoItem();
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::generateId() {
  const QString siglaLoja = UserSession::fromLoja("sigla", ui->itemBoxVendedor->text()).toString();

  if (siglaLoja.isEmpty()) { throw RuntimeException("Erro buscando sigla da loja!"); }

  QString id = siglaLoja + "-" + qApp->serverDate().toString("yy");

  const QString replica = ui->lineEditReplicaDe->text();

  if (replica.isEmpty()) {
    SqlQuery query;
    query.prepare("SELECT MAX(idOrcamento) AS idOrcamento FROM orcamento WHERE idOrcamento LIKE :id");
    query.bindValue(":id", id + "%");

    if (not query.exec()) { throw RuntimeException("Erro buscando próximo id disponível: " + query.lastError().text()); }

    const int last = query.first() ? query.value("idOrcamento").toString().remove(id).leftRef(4).toInt() : 0;

    id += QString("%1").arg(last + 1, 4, 10, QChar('0'));
    id += ui->checkBoxRepresentacao->isChecked() ? "R" : "";
    id += "O";

    if (id.size() != 12 and id.size() != 13) { throw RuntimeException("Tamanho do Id errado: " + id); }
  } else {
    SqlQuery query;
    query.prepare(
        "SELECT COALESCE(MAX(CAST(RIGHT(idOrcamento, CHAR_LENGTH(idOrcamento) - LOCATE('Rev', idOrcamento) - 2) AS UNSIGNED)) + 1, 1) AS revisao FROM orcamento WHERE CHAR_LENGTH(idOrcamento) > 16 "
        "AND idOrcamento LIKE :idOrcamento");
    query.bindValue(":idOrcamento", replica.left(11) + "%");

    if (not query.exec() or not query.first()) { throw RuntimeException("Erro buscando próxima revisão disponível: " + query.lastError().text()); }

    id = replica.left(replica.indexOf("-REV")) + "-REV" + query.value("revisao").toString();
  }

  ui->lineEditOrcamento->setText(id);
}

void Orcamento::recalcularTotais() {
  // TODO: just change this function to call 'calcPrecoGlobalTotal' and be sure all is recalculated?

  double subTotalBruto = 0.;
  double subTotalLiq = 0.;
  double total = 0.;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    subTotalBruto += modelItem.data(row, "parcial").toDouble();
    subTotalLiq += modelItem.data(row, "parcialDesc").toDouble();
    total += modelItem.data(row, "total").toDouble();
  }

  if (abs(subTotalBruto - ui->doubleSpinBoxSubTotalBruto->value()) > 1) {
    QStringList logString;

    logString << "IdOrcamento: " + ui->lineEditOrcamento->text() + "\nsubTotalBruto: " + QString::number(subTotalBruto) + "\nspinBox: " + QString::number(ui->doubleSpinBoxSubTotalBruto->value());

    for (int row = 0; row < modelItem.rowCount(); ++row) {
      logString << "Id: " + modelItem.data(row, "idOrcamentoProduto").toString() + "\nprcUnitario: " + modelItem.data(row, "prcUnitario").toString() +
                       "\ndescUnitario: " + modelItem.data(row, "descUnitario").toString() + "\nquant: " + modelItem.data(row, "quant").toString() +
                       "\ncodComercial: " + modelItem.data(row, "codComercial").toString() + "\nparcial: " + modelItem.data(row, "parcial").toString() +
                       "\ndesconto: " + modelItem.data(row, "desconto").toString() + "\nparcialDesc: " + modelItem.data(row, "parcialDesc").toString() +
                       "\ndescGlobal: " + modelItem.data(row, "descGlobal").toString() + "\ntotal: " + modelItem.data(row, "total").toString();
    }

    Log::createLog("Exceção", logString.join("\n"));

    calcPrecoGlobalTotal();

    throw RuntimeException("Subtotal dos itens não confere com SubTotalBruto! Recalculando valores!");
  }

  if (abs(subTotalLiq - ui->doubleSpinBoxSubTotalLiq->value()) > 1) {
    QStringList logString;

    logString << "IdOrcamento: " + ui->lineEditOrcamento->text() + "\nsubTotalLiq: " + QString::number(subTotalLiq) + "\nspinBox: " + QString::number(ui->doubleSpinBoxSubTotalLiq->value());

    for (int row = 0; row < modelItem.rowCount(); ++row) {
      logString << "Id: " + modelItem.data(row, "idOrcamentoProduto").toString() + "\nprcUnitario: " + modelItem.data(row, "prcUnitario").toString() +
                       "\ndescUnitario: " + modelItem.data(row, "descUnitario").toString() + "\nquant: " + modelItem.data(row, "quant").toString() +
                       "\ncodComercial: " + modelItem.data(row, "codComercial").toString() + "\nparcial: " + modelItem.data(row, "parcial").toString() +
                       "\ndesconto: " + modelItem.data(row, "desconto").toString() + "\nparcialDesc: " + modelItem.data(row, "parcialDesc").toString() +
                       "\ndescGlobal: " + modelItem.data(row, "descGlobal").toString() + "\ntotal: " + modelItem.data(row, "total").toString();
    }

    Log::createLog("Exceção", logString.join("\n"));

    calcPrecoGlobalTotal();

    throw RuntimeException("Total dos itens não confere com SubTotalLíquido! Recalculando valores!");
  }

  if (abs(total - (ui->doubleSpinBoxTotal->value() - ui->doubleSpinBoxFrete->value())) > 1) {
    QStringList logString;

    logString << "IdOrcamento: " + ui->lineEditOrcamento->text() + "\ntotal: " + QString::number(total) + "\nspinBoxTotal: " + QString::number(ui->doubleSpinBoxTotal->value()) +
                     "\nspinBoxFrete: " + QString::number(ui->doubleSpinBoxFrete->value());

    for (int row = 0; row < modelItem.rowCount(); ++row) {
      logString << "Id: " + modelItem.data(row, "idOrcamentoProduto").toString() + "\nprcUnitario: " + modelItem.data(row, "prcUnitario").toString() +
                       "\ndescUnitario: " + modelItem.data(row, "descUnitario").toString() + "\nquant: " + modelItem.data(row, "quant").toString() +
                       "\ncodComercial: " + modelItem.data(row, "codComercial").toString() + "\nparcial: " + modelItem.data(row, "parcial").toString() +
                       "\ndesconto: " + modelItem.data(row, "desconto").toString() + "\nparcialDesc: " + modelItem.data(row, "parcialDesc").toString() +
                       "\ndescGlobal: " + modelItem.data(row, "descGlobal").toString() + "\ntotal: " + modelItem.data(row, "total").toString();
    }

    Log::createLog("Exceção", logString.join("\n"));

    calcPrecoGlobalTotal();

    throw RuntimeException("Total dos itens não confere com Total! Recalculando valores!");
  }
}

void Orcamento::verifyFields() {
  verificaDisponibilidadeEstoque();

  recalcularTotais();

  if (ui->itemBoxCliente->text().isEmpty()) { throw RuntimeError("Cliente inválido!", this); }

  if (ui->itemBoxVendedor->text().isEmpty()) { throw RuntimeError("Vendedor inválido!", this); }

  if (ui->itemBoxProfissional->text().isEmpty()) { throw RuntimeError("Profissional inválido!", this); }

  if (ui->itemBoxEndereco->text().isEmpty()) { throw RuntimeError(R"(Endereço inválido! Se não possui endereço, escolha "Não há"!)", this); }

  if (modelItem.rowCount() == 0) { throw RuntimeError("Não pode cadastrar um orçamento sem itens!", this); }
}

void Orcamento::savingProcedures() {
  if (tipo == Tipo::Cadastrar) {
    const int idLoja = UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()).toInt();

    if (idLoja == 0) { throw RuntimeException("Erro buscando idLoja!"); }

    setData("idLoja", idLoja);

    setData("idOrcamento", ui->lineEditOrcamento->text());
    setData("idOrcamentoBase", ui->lineEditOrcamento->text().left(11));
    setData("replicadoDe", ui->lineEditReplicaDe->text());
    setData("representacao", ui->checkBoxRepresentacao->isChecked());
  }

  setData("idUsuario", ui->itemBoxVendedor->getId());
  setData("idCliente", ui->itemBoxCliente->getId());
  setData("data", ui->dataEmissao->isReadOnly() ? qApp->serverDateTime() : ui->dataEmissao->dateTime());
  setData("descontoPorc", ui->doubleSpinBoxDescontoGlobal->value());
  setData("descontoReais", ui->doubleSpinBoxSubTotalLiq->value() * ui->doubleSpinBoxDescontoGlobal->value() / 100.);
  setData("frete", ui->doubleSpinBoxFrete->value());
  setData("idEnderecoEntrega", ui->itemBoxEndereco->getId());
  setData("idProfissional", ui->itemBoxProfissional->getId());
  setData("observacao", ui->plainTextEditObs->toPlainText());
  setData("prazoEntrega", ui->spinBoxPrazoEntrega->value());
  setData("subTotalBru", ui->doubleSpinBoxSubTotalBruto->value());
  setData("subTotalLiq", ui->doubleSpinBoxSubTotalLiq->value());
  setData("total", ui->doubleSpinBoxTotal->value());
  setData("validade", ui->spinBoxValidade->value());
  setData("freteManual", ui->checkBoxFreteManual->isChecked());

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) {
    modelItem.setData(row, "idOrcamento", ui->lineEditOrcamento->text());
    modelItem.setData(row, "idLoja", model.data(currentRow, "idLoja"));

    const double prcUnitario = modelItem.data(row, "prcUnitario").toDouble();
    const double desconto = modelItem.data(row, "desconto").toDouble() / 100.;

    modelItem.setData(row, "descUnitario", prcUnitario - (prcUnitario * desconto));

    const bool mostrarDesconto = (modelItem.data(row, "parcialDesc").toDouble() - modelItem.data(row, "parcial").toDouble()) < -0.1;

    modelItem.setData(row, "mostrarDesconto", mostrarDesconto);
  }

  buscarCadastrarConsultor();

  atualizaReplica();
}

void Orcamento::buscarCadastrarConsultor() {
  // TODO: change this to return optional and rename to 'buscarConsultor'

  QStringList fornecedores;

  for (int row = 0, rowCount = modelItem.rowCount(); row < rowCount; ++row) { fornecedores << modelItem.data(row, "fornecedor").toString(); }

  fornecedores.removeDuplicates();

  for (auto &fornecedor : fornecedores) { fornecedor.prepend("'").append("'"); }

  SqlQuery query;

  if (not query.exec("SELECT idUsuario FROM usuario WHERE desativado = FALSE AND especialidade > 0 AND especialidade IN (SELECT especialidade FROM fornecedor WHERE razaoSocial IN (" +
                     fornecedores.join(", ") + "))")) {
    throw RuntimeException("Erro buscando consultor: " + query.lastError().text());
  }

  if (query.size() > 1) { throw RuntimeException("Mais de um consultor disponível!"); }

  if (query.size() == 1 and query.first()) { setData("idUsuarioConsultor", query.value("idUsuario")); }

  if (query.size() == 0) { model.setData(currentRow, "idUsuarioConsultor", QVariant(QVariant::UInt)); }
}

void Orcamento::atualizaReplica() {
  if (not ui->lineEditReplicaDe->text().isEmpty()) {
    SqlQuery query;
    query.prepare("UPDATE orcamento SET status = 'REPLICADO', replicadoEm = :idReplica WHERE idOrcamento = :idOrcamento");
    query.bindValue(":idReplica", ui->lineEditOrcamento->text());
    query.bindValue(":idOrcamento", ui->lineEditReplicaDe->text());

    if (not query.exec()) { throw RuntimeException("Erro salvando replicadoEm: " + query.lastError().text()); }
  }
}

void Orcamento::clearFields() {
  RegisterDialog::clearFields();

  if (UserSession::tipoUsuario == "VENDEDOR") { ui->itemBoxVendedor->setId(UserSession::idUsuario); }

  //  ui->itemBoxEndereco->setDisabled(true);
}

void Orcamento::on_pushButtonRemoverItem_clicked() { removeItem(); }

void Orcamento::on_doubleSpinBoxQuant_valueChanged(const double quant) {
  const double step = ui->doubleSpinBoxQuant->singleStep();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.;

  unsetConnections();

  try {
    [&] {
      if (currentItemIsEstoque) {
        const double caixas = quant / step;
        ui->doubleSpinBoxCaixas->setValue(caixas);

        const double itemBruto = quant * prcUn;
        ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - desc));
      } else {
        const double resto = fmod(quant, step);
        const double quant2 = not qFuzzyIsNull(resto) ? ceil(quant / step) * step : quant;
        ui->doubleSpinBoxQuant->setValue(quant2);

        const double caixas2 = quant2 / step;
        ui->doubleSpinBoxCaixas->setValue(caixas2);

        const double itemBruto2 = quant2 * prcUn;
        ui->doubleSpinBoxTotalItem->setValue(itemBruto2 * (1. - desc));
      }
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_pushButtonCadastrarOrcamento_clicked() {
  // TODO: ao fechar pedido calcular o frete com o endereco selecinado em 'end. entrega'
  // se o valor calculado for maior que o do campo frete pedir autorizacao do gerente para manter o valor atual
  // senao usa o valor calculado

  // pedir login caso o frete (manual ou automatico) seja menor que ou o valorPeso ou a porcentagem parametrizada

  save();
}

void Orcamento::on_pushButtonAtualizarOrcamento_clicked() { save(); }

void Orcamento::calcPrecoGlobalTotal() {
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

  // calcula totais considerando desconto global atual

  if (not ui->checkBoxFreteManual->isChecked()) {
    const double frete = qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete);

    ui->doubleSpinBoxFrete->setMinimum(frete);
    ui->doubleSpinBoxFrete->setValue(frete);
  }

  const double frete = ui->doubleSpinBoxFrete->value();
  const double descGlobal = ui->doubleSpinBoxDescontoGlobal->value();

  ui->doubleSpinBoxDescontoGlobalReais->setMaximum(subTotalItens);
  ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalItens * descGlobal / 100);

  ui->doubleSpinBoxTotal->setMaximum(subTotalItens + frete);
  ui->doubleSpinBoxTotal->setValue(subTotalItens * (1 - (descGlobal / 100)) + frete);
}

void Orcamento::on_pushButtonGerarPdf_clicked() {
  PDF pdf(data("idOrcamento").toString(), PDF::Tipo::Orcamento, this);
  pdf.gerarPdf();
}

void Orcamento::setupTables() {
  modelItem.setTable("orcamento_has_produto");

  modelItem.setHeaderData("produto", "Produto");
  modelItem.setHeaderData("fornecedor", "Fornecedor");
  modelItem.setHeaderData("obs", "Obs.");
  modelItem.setHeaderData("prcUnitario", "Preço/Un.");
  modelItem.setHeaderData("caixas", "Caixas");
  modelItem.setHeaderData("quant", "Quant.");
  modelItem.setHeaderData("un", "Un.");
  modelItem.setHeaderData("codComercial", "Código");
  modelItem.setHeaderData("formComercial", "Formato");
  modelItem.setHeaderData("quantCaixa", "Quant./Cx.");
  modelItem.setHeaderData("parcial", "Subtotal");
  modelItem.setHeaderData("desconto", "Desc. %");
  modelItem.setHeaderData("parcialDesc", "Total");

  modelItem.proxyModel = new SearchDialogProxyModel(&modelItem, this);

  ui->tableProdutos->setModel(&modelItem);

  ui->tableProdutos->hideColumn("idOrcamentoProduto");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("idOrcamento");
  ui->tableProdutos->hideColumn("idLoja");
  ui->tableProdutos->hideColumn("quantCaixa");
  ui->tableProdutos->hideColumn("descUnitario");
  ui->tableProdutos->hideColumn("descGlobal");
  ui->tableProdutos->hideColumn("total");
  ui->tableProdutos->hideColumn("estoque");
  ui->tableProdutos->hideColumn("promocao");
  ui->tableProdutos->hideColumn("mostrarDesconto");

  ui->tableProdutos->setItemDelegate(new DoubleDelegate(this));

  ui->tableProdutos->setItemDelegateForColumn("quant", new DoubleDelegate(4, this));
  ui->tableProdutos->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcial", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("parcialDesc", new ReaisDelegate(this));
  ui->tableProdutos->setItemDelegateForColumn("desconto", new PorcentagemDelegate(false, this));
}

void Orcamento::atualizarItem() { adicionarItem(Tipo::Atualizar); }

void Orcamento::adicionarItem(const Tipo tipoItem) {
  if (ui->itemBoxProduto->text().isEmpty()) { throw RuntimeError("Item inválido!", this); }

  if (qFuzzyIsNull(ui->doubleSpinBoxQuant->value())) { throw RuntimeError("Quantidade inválida!", this); }

  unsetConnections();

  try {
    [&] {
      if (tipoItem == Tipo::Cadastrar) { currentRowItem = modelItem.insertRowAtEnd(); }

      modelItem.setData(currentRowItem, "idProduto", ui->itemBoxProduto->getId().toInt());
      modelItem.setData(currentRowItem, "fornecedor", ui->lineEditFornecedor->text());
      modelItem.setData(currentRowItem, "produto", ui->itemBoxProduto->text());
      modelItem.setData(currentRowItem, "obs", ui->lineEditObs->text());
      modelItem.setData(currentRowItem, "prcUnitario", ui->lineEditPrecoUn->getValue());
      modelItem.setData(currentRowItem, "caixas", ui->doubleSpinBoxCaixas->value());
      modelItem.setData(currentRowItem, "quant", ui->doubleSpinBoxQuant->value());
      modelItem.setData(currentRowItem, "quantCaixa", ui->doubleSpinBoxQuant->singleStep());
      modelItem.setData(currentRowItem, "un", ui->lineEditUn->text());
      modelItem.setData(currentRowItem, "codComercial", ui->lineEditCodComercial->text());
      modelItem.setData(currentRowItem, "formComercial", ui->lineEditFormComercial->text());
      modelItem.setData(currentRowItem, "desconto", ui->doubleSpinBoxDesconto->value());
      modelItem.setData(currentRowItem, "estoque", currentItemIsEstoque);
      modelItem.setData(currentRowItem, "promocao", currentItemIsPromocao);
      modelItem.setData(currentRowItem, "parcial", modelItem.data(currentRowItem, "quant").toDouble() * modelItem.data(currentRowItem, "prcUnitario").toDouble());
      modelItem.setData(currentRowItem, "parcialDesc", ui->doubleSpinBoxTotalItem->value());
      modelItem.setData(currentRowItem, "descGlobal", ui->doubleSpinBoxDescontoGlobal->value());
      modelItem.setData(currentRowItem, "total", ui->doubleSpinBoxTotalItem->value() * (1 - (ui->doubleSpinBoxDescontoGlobal->value() / 100)));

      if (modelItem.rowCount() == 1 and ui->checkBoxRepresentacao->isChecked()) { ui->itemBoxProduto->setFornecedorRep(modelItem.data(currentRowItem, "fornecedor").toString()); }

      if (tipoItem == Tipo::Cadastrar) { backupItem.append(modelItem.record(currentRowItem)); }

      isDirty = true;
      ui->checkBoxRepresentacao->setDisabled(true);
    }();
  } catch (std::exception &e) {}

  novoItem();

  calcPrecoGlobalTotal();

  setConnections();
}

void Orcamento::on_pushButtonAdicionarItem_clicked() { adicionarItem(); }

void Orcamento::on_pushButtonAtualizarItem_clicked() { atualizarItem(); }

void Orcamento::on_pushButtonGerarVenda_clicked() {
  save(true);

  const QDateTime time = ui->dataEmissao->dateTime();

  if (not time.isValid()) { return; }

  if (time.addDays(data("validade").toInt()).date() < qApp->serverDate()) { throw RuntimeError("Orçamento vencido!"); }

  if (ui->itemBoxEndereco->text().isEmpty()) { throw RuntimeError("Deve selecionar endereço!"); }

  verificaCadastroCliente();

  auto *venda = new Venda(parentWidget());
  venda->setAttribute(Qt::WA_DeleteOnClose);
  venda->prepararVenda(ui->lineEditOrcamento->text());
  venda->show();

  close();
}

void Orcamento::on_doubleSpinBoxCaixas_valueChanged(const double caixas) {
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double desc = ui->doubleSpinBoxDesconto->value() / 100.;

  unsetConnections();

  try {
    [&] {
      if (currentItemIsEstoque) {
        const double quant = caixas * ui->spinBoxQuantCx->value();
        ui->doubleSpinBoxQuant->setValue(quant);

        const double itemBruto = quant * prcUn;
        ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - desc));
      } else {
        const double step = ui->doubleSpinBoxCaixas->singleStep();
        const double resto = fmod(caixas, step);
        const double caixas2 = not qFuzzyIsNull(resto) ? ceil(caixas) : caixas;
        ui->doubleSpinBoxCaixas->setValue(caixas2);

        const double quant2 = caixas2 * ui->spinBoxQuantCx->value();
        ui->doubleSpinBoxQuant->setValue(quant2);

        const double itemBruto2 = quant2 * prcUn;
        ui->doubleSpinBoxTotalItem->setValue(itemBruto2 * (1. - desc));
      }
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_pushButtonApagarOrc_clicked() {
  auto *baixa = new BaixaOrcamento(data("idOrcamento").toString(), this);
  baixa->show();
}

void Orcamento::on_itemBoxProduto_idChanged(const QVariant &) {
  if (ui->itemBoxProduto->text().isEmpty()) { return; }

  // -------------------------------------------------------------------------

  ui->doubleSpinBoxCaixas->clear();
  ui->doubleSpinBoxDesconto->clear();
  ui->doubleSpinBoxQuant->clear();
  ui->doubleSpinBoxTotalItem->clear();
  ui->lineEditCodComercial->clear();
  ui->lineEditEstoque->clear();
  ui->lineEditFormComercial->clear();
  ui->lineEditFornecedor->clear();
  if (ui->pushButtonAdicionarItem->isVisible()) { ui->lineEditObs->clear(); }
  ui->lineEditPrecoUn->clear();
  ui->lineEditUn->clear();
  ui->spinBoxMinimo->clear();
  ui->spinBoxQuantCx->clear();

  // -------------------------------------------------------------------------

  SqlQuery query;
  query.prepare("SELECT un, precoVenda, estoqueRestante, fornecedor, codComercial, formComercial, quantCaixa, minimo, multiplo, estoque, promocao FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", ui->itemBoxProduto->getId());

  if (not query.exec() or not query.first()) { throw RuntimeException("Erro na busca do produto: " + query.lastError().text()); }

  const QString un = query.value("un").toString().toUpper();

  ui->lineEditUn->setText(un);
  ui->lineEditPrecoUn->setValue(query.value("precoVenda").toDouble());
  ui->lineEditEstoque->setValue(query.value("estoqueRestante").toDouble());
  ui->lineEditFornecedor->setText(query.value("fornecedor").toString());
  ui->lineEditCodComercial->setText(query.value("codComercial").toString());
  ui->lineEditFormComercial->setText(query.value("formComercial").toString());

  const double minimo = query.value("minimo").toDouble();
  const double multiplo = query.value("multiplo").toDouble();
  const double quantCaixa = query.value("quantCaixa").toDouble();

  ui->spinBoxQuantCx->setValue(quantCaixa);

  ui->spinBoxMinimo->setValue(minimo);
  ui->doubleSpinBoxQuant->setMinimum(minimo);
  ui->doubleSpinBoxCaixas->setMinimum(minimo / quantCaixa);

  currentItemIsEstoque = query.value("estoque").toBool();
  currentItemIsPromocao = query.value("promocao").toInt();

  if (currentItemIsEstoque) {
    ui->doubleSpinBoxQuant->setMaximum(query.value("estoqueRestante").toDouble());
    ui->doubleSpinBoxCaixas->setMaximum(query.value("estoqueRestante").toDouble() / quantCaixa);
  } else {
    ui->doubleSpinBoxQuant->setMaximum(9999999.000000);
    ui->doubleSpinBoxCaixas->setMaximum(9999999.000000);
  }

  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxCaixas->setEnabled(true);
  ui->doubleSpinBoxDesconto->setEnabled(true);
  ui->doubleSpinBoxQuant->setEnabled(true);
  ui->doubleSpinBoxTotalItem->setEnabled(true);
  ui->lineEditPrecoUn->setEnabled(true);
  ui->lineEditUn->setEnabled(true);
  ui->spinBoxMinimo->setEnabled(true);
  ui->spinBoxQuantCx->setEnabled(true);

  ui->doubleSpinBoxCaixas->setSingleStep(1.);
  ui->doubleSpinBoxQuant->setSingleStep(quantCaixa);

  // TODO: 0verificar se preciso tratar os casos sem multiplo
  // if (minimo != 0) ...
  if (not qFuzzyIsNull(minimo) and not qFuzzyIsNull(multiplo)) {
    ui->doubleSpinBoxCaixas->setSingleStep(multiplo / quantCaixa);
    ui->doubleSpinBoxQuant->setSingleStep(multiplo);
  }

  ui->doubleSpinBoxQuant->setValue(0.);
  ui->doubleSpinBoxCaixas->setValue(0.);
  ui->doubleSpinBoxDesconto->setValue(0.);

  on_doubleSpinBoxCaixas_valueChanged(ui->doubleSpinBoxCaixas->value());
}

void Orcamento::on_itemBoxCliente_textChanged(const QString &) {
  const QString idCliente = QString::number(ui->itemBoxCliente->getId().toInt());
  ui->itemBoxEndereco->setFilter("(idCliente = " + idCliente + " OR idEndereco = 1) AND desativado = FALSE");

  SqlQuery queryCliente;
  queryCliente.prepare("SELECT idProfissionalRel FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", ui->itemBoxCliente->getId());

  if (not queryCliente.exec() or not queryCliente.first()) { throw RuntimeException("Erro ao buscar cliente: " + queryCliente.lastError().text()); }

  ui->itemBoxProfissional->setId(queryCliente.value("idProfissionalRel"));
  ui->itemBoxEndereco->setEnabled(true);
  ui->itemBoxEndereco->clear();
}

void Orcamento::on_checkBoxFreteManual_clicked(const bool checked) {
  if (not canChangeFrete) {
    qApp->enqueueInformation("Necessário autorização de um gerente ou administrador!", this);

    LoginDialog dialog(LoginDialog::Tipo::Autorizacao, this);

    if (dialog.exec() != QDialog::Accepted) {
      ui->checkBoxFreteManual->setChecked(not checked);
      return;
    }

    canChangeFrete = true;
  }

  const double frete = qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete);

  ui->doubleSpinBoxFrete->setMinimum(checked ? 0 : frete);

  if (not checked) { ui->doubleSpinBoxFrete->setValue(frete); }

  ui->doubleSpinBoxFrete->setFocus();
}

void Orcamento::on_pushButtonReplicar_clicked() {
  // passar por cada produto verificando sua validade/descontinuado
  QStringList produtos;
  QStringList estoques;
  QVector<int> skipRows;

  SqlQuery queryProduto;
  queryProduto.prepare("SELECT (descontinuado OR desativado) AS invalido FROM produto WHERE idProduto = :idProduto");

  SqlQuery queryEquivalente;
  queryEquivalente.prepare("SELECT idProduto FROM produto WHERE codComercial = :codComercial AND descontinuado = FALSE AND desativado = FALSE AND estoque = FALSE");

  SqlQuery queryEstoque;
  queryEstoque.prepare("SELECT 0 FROM produto WHERE idProduto = :idProduto AND estoqueRestante >= :quant");

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    queryProduto.bindValue(":idProduto", modelItem.data(row, "idProduto"));

    if (not queryProduto.exec() or not queryProduto.first()) { throw RuntimeException("Erro verificando validade dos produtos: " + queryProduto.lastError().text()); }

    if (queryProduto.value("invalido").toBool()) {
      queryEquivalente.bindValue(":codComercial", modelItem.data(row, "codComercial"));

      if (not queryEquivalente.exec()) { throw RuntimeException("Erro procurando produto equivalente: " + queryEquivalente.lastError().text()); }

      if (queryEquivalente.first()) {
        modelItem.setData(row, "idProduto", queryEquivalente.value("idProduto"));
      } else {
        produtos << QString::number(row + 1) + " - " + modelItem.data(row, "produto").toString();
        skipRows << row;
      }
    }

    if (modelItem.data(row, "estoque").toInt() == 1) {
      queryEstoque.bindValue(":idProduto", modelItem.data(row, "idProduto"));
      queryEstoque.bindValue(":quant", modelItem.data(row, "quant"));

      if (not queryEstoque.exec()) { throw RuntimeException("Erro verificando estoque: " + queryEstoque.lastError().text()); }

      if (not queryEstoque.first()) {
        estoques << modelItem.data(row, "produto").toString();
        skipRows << row;
      }
    }
  }

  if (not produtos.isEmpty()) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Os seguintes itens estão descontinuados e serão removidos da réplica:\n    -" + produtos.join("\n    -"), QMessageBox::Yes | QMessageBox::No,
                       this);
    msgBox.setButtonText(QMessageBox::Yes, "Continuar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) { return; }
  }

  if (not estoques.isEmpty()) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!",
                       "Os seguintes produtos de estoque não estão mais disponíveis na quantidade selecionada e serão removidos da réplica:\n    -" + estoques.join("\n    -"),
                       QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Continuar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) { return; }
  }

  auto *replica = new Orcamento(parentWidget());

  replica->ui->pushButtonReplicar->hide();

  replica->ui->itemBoxCliente->setId(data("idCliente"));
  replica->ui->itemBoxProfissional->setId(data("idProfissional"));
  replica->ui->itemBoxVendedor->setId(data("idUsuario"));
  replica->ui->itemBoxEndereco->setId(data("idEnderecoEntrega"));
  replica->ui->spinBoxValidade->setValue(data("validade").toInt());
  replica->ui->dataEmissao->setDate(qApp->serverDate());
  replica->ui->checkBoxRepresentacao->setChecked(ui->checkBoxRepresentacao->isChecked());
  replica->ui->lineEditReplicaDe->setText(data("idOrcamento").toString());
  replica->ui->plainTextEditObs->setPlainText(data("observacao").toString());

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (skipRows.contains(row)) { continue; }

    replica->ui->itemBoxProduto->setId(modelItem.data(row, "idProduto"));
    replica->ui->doubleSpinBoxQuant->setValue(modelItem.data(row, "quant").toDouble());
    replica->ui->doubleSpinBoxDesconto->setValue(modelItem.data(row, "desconto").toDouble());
    replica->ui->lineEditObs->setText(modelItem.data(row, "obs").toString());
    replica->adicionarItem();
  }

  replica->show();
}

void Orcamento::cadastrar() {
  try {
    qApp->startTransaction("Orcamento::cadastrar");

    if (tipo == Tipo::Cadastrar) {
      generateId();

      currentRow = model.insertRowAtEnd();
    }

    savingProcedures();

    model.submitAll();

    primaryId = ui->lineEditOrcamento->text();

    if (primaryId.isEmpty()) { throw RuntimeException("Id vazio!"); }

    modelItem.submitAll();

    qApp->endTransaction();

    backupItem.clear();

    model.setFilter(primaryKey + " = '" + primaryId + "'");

    modelItem.setFilter(primaryKey + " = '" + primaryId + "'");
  } catch (std::exception &e) {
    qApp->rollbackTransaction();
    model.select();
    modelItem.select();

    for (auto &record : backupItem) { modelItem.insertRecord(-1, record); }

    if (tipo == Tipo::Cadastrar) { ui->lineEditOrcamento->setText("Auto gerado"); }

    throw;
  }
}

void Orcamento::verificaCadastroCliente() {
  const int idCliente = ui->itemBoxCliente->getId().toInt();

  bool incompleto = false;

  SqlQuery queryCliente;
  queryCliente.prepare("SELECT cpf, cnpj FROM cliente WHERE idCliente = :idCliente");
  queryCliente.bindValue(":idCliente", idCliente);

  if (not queryCliente.exec() or not queryCliente.first()) { throw RuntimeException("Erro verificando se cliente possui CPF/CNPJ: " + queryCliente.lastError().text()); }

  if (queryCliente.value("cpf").toString().isEmpty() and queryCliente.value("cnpj").toString().isEmpty()) { incompleto = true; }

  SqlQuery queryCadastro;
  queryCadastro.prepare("SELECT idCliente FROM cliente_has_endereco WHERE idCliente = :idCliente");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) { throw RuntimeException("Erro verificando se cliente possui endereço: " + queryCadastro.lastError().text()); }

  if (not queryCadastro.first()) { incompleto = true; }

  queryCadastro.prepare("SELECT c.incompleto FROM orcamento o LEFT JOIN cliente c ON o.idCliente = c.idCliente WHERE c.idCliente = :idCliente AND c.incompleto = TRUE");
  queryCadastro.bindValue(":idCliente", idCliente);

  if (not queryCadastro.exec()) { throw RuntimeException("Erro verificando se cadastro do cliente está completo: " + queryCadastro.lastError().text()); }

  if (queryCadastro.first()) { incompleto = true; }

  if (incompleto) {
    auto *cadCliente = new CadastroCliente(this);
    cadCliente->viewRegisterById(idCliente);
    cadCliente->show();

    throw RuntimeError("Cadastro incompleto, deve preencher pelo menos:\n  -CPF/CNPJ\n  -Telefone Principal\n  -Email\n  -Endereço");
  }
}

void Orcamento::on_pushButtonGerarExcel_clicked() {
  Excel excel(ui->lineEditOrcamento->text(), Excel::Tipo::Orcamento, this);
  excel.gerarExcel();
}

void Orcamento::on_checkBoxRepresentacao_toggled(const bool checked) {
  ui->itemBoxProduto->setRepresentacao(checked);
  novoItem();
}

void Orcamento::on_doubleSpinBoxDesconto_valueChanged(const double desconto) {
  const double caixas = ui->doubleSpinBoxCaixas->value();
  const double caixas2 = not qFuzzyIsNull(fmod(caixas, ui->doubleSpinBoxCaixas->singleStep())) ? ceil(caixas) : caixas;
  const double quant = caixas2 * ui->spinBoxQuantCx->value();

  unsetConnections();

  try {
    [&] {
      const double prcUn = ui->lineEditPrecoUn->getValue();
      const double itemBruto = quant * prcUn;

      ui->doubleSpinBoxTotalItem->setValue(itemBruto * (1. - (desconto / 100)));
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_doubleSpinBoxDescontoGlobalReais_valueChanged(const double descontoReais) {
  unsetConnections();

  try {
    [&] {
      const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
      const double descontoPorc = descontoReais / subTotalLiq;

      for (int row = 0; row < modelItem.rowCount(); ++row) {
        modelItem.setData(row, "descGlobal", descontoPorc * 100);

        const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
        modelItem.setData(row, "total", parcialDesc * (1 - descontoPorc));
      }

      const double frete = ui->doubleSpinBoxFrete->value();

      ui->doubleSpinBoxDescontoGlobal->setValue(descontoPorc * 100);
      ui->doubleSpinBoxTotal->setValue(subTotalLiq - descontoReais + frete);
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_doubleSpinBoxFrete_valueChanged(const double frete) {
  const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
  const double desconto = ui->doubleSpinBoxDescontoGlobalReais->value();

  unsetConnections();

  try {
    [&] {
      ui->doubleSpinBoxTotal->setMinimum(frete);
      ui->doubleSpinBoxTotal->setMaximum(ui->doubleSpinBoxSubTotalLiq->value() + frete);
      ui->doubleSpinBoxTotal->setValue(subTotalLiq - desconto + frete);
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_itemBoxVendedor_textChanged(const QString &) {
  if (ui->itemBoxVendedor->text().isEmpty()) { return; }

  buscarParametrosFrete();

  if (not ui->checkBoxFreteManual->isChecked()) {
    const double frete = qMax(ui->doubleSpinBoxSubTotalBruto->value() * porcFrete / 100., minimoFrete);

    ui->doubleSpinBoxFrete->setMinimum(frete);
    ui->doubleSpinBoxFrete->setValue(frete);
  }
}

void Orcamento::buscarParametrosFrete() {
  const int idLoja = UserSession::fromLoja("usuario.idLoja", ui->itemBoxVendedor->text()).toInt();

  if (idLoja == 0) { throw RuntimeException("Erro buscando idLoja!"); }

  SqlQuery queryFrete;
  queryFrete.prepare("SELECT valorMinimoFrete, porcentagemFrete FROM loja WHERE idLoja = :idLoja");
  queryFrete.bindValue(":idLoja", idLoja);

  if (not queryFrete.exec() or not queryFrete.next()) { throw RuntimeException("Erro buscando parâmetros do frete: " + queryFrete.lastError().text()); }

  minimoFrete = queryFrete.value("valorMinimoFrete").toDouble();
  porcFrete = queryFrete.value("porcentagemFrete").toDouble();
}

void Orcamento::on_doubleSpinBoxDescontoGlobal_valueChanged(const double descontoPorc) {
  unsetConnections();

  try {
    [&] {
      const double descontoPorc2 = descontoPorc / 100;

      for (int row = 0; row < modelItem.rowCount(); ++row) {
        modelItem.setData(row, "descGlobal", descontoPorc);

        const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
        modelItem.setData(row, "total", parcialDesc * (1 - descontoPorc2));
      }

      const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
      const double frete = ui->doubleSpinBoxFrete->value();

      ui->doubleSpinBoxDescontoGlobalReais->setValue(subTotalLiq * descontoPorc2);
      ui->doubleSpinBoxTotal->setValue(subTotalLiq * (1 - descontoPorc2) + frete);
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_doubleSpinBoxTotal_valueChanged(const double total) {
  unsetConnections();

  try {
    [&] {
      const double subTotalLiq = ui->doubleSpinBoxSubTotalLiq->value();
      const double frete = ui->doubleSpinBoxFrete->value();
      const double descontoReais = subTotalLiq + frete - total;
      const double descontoPorc = descontoReais / subTotalLiq;

      for (int row = 0; row < modelItem.rowCount(); ++row) {
        modelItem.setData(row, "descGlobal", descontoPorc * 100);

        const double parcialDesc = modelItem.data(row, "parcialDesc").toDouble();
        modelItem.setData(row, "total", parcialDesc * (1 - descontoPorc));
      }

      ui->doubleSpinBoxDescontoGlobal->setValue(descontoPorc * 100);
      ui->doubleSpinBoxDescontoGlobalReais->setValue(descontoReais);
    }();
  } catch (std::exception &e) {}

  setConnections();
}

void Orcamento::on_doubleSpinBoxTotalItem_valueChanged(const double) {
  if (ui->itemBoxProduto->text().isEmpty()) { return; }

  const double quant = ui->doubleSpinBoxQuant->value();
  const double prcUn = ui->lineEditPrecoUn->getValue();
  const double itemBruto = quant * prcUn;
  const double subTotalItem = ui->doubleSpinBoxTotalItem->value();
  const double desconto = (itemBruto - subTotalItem) / itemBruto * 100.;

  if (qFuzzyIsNull(itemBruto)) { return; }

  unsetConnections();

  ui->doubleSpinBoxDesconto->setValue(desconto);

  setConnections();
}

void Orcamento::successMessage() { qApp->enqueueInformation((tipo == Tipo::Atualizar) ? "Cadastro atualizado!" : "Orçamento cadastrado com sucesso!", this); }

void Orcamento::on_pushButtonCalculadora_clicked() { QDesktopServices::openUrl(QUrl::fromLocalFile(R"(C:\Windows\System32\calc.exe)")); }

void Orcamento::on_pushButtonCalcularFrete_clicked() {
  LoginDialog dialog(LoginDialog::Tipo::Autorizacao, this);

  if (dialog.exec() == QDialog::Rejected) { return; }

  auto *frete = new CalculoFrete(this);
  frete->setCliente(ui->itemBoxCliente->getId());
  frete->exec();

  const double dist = frete->getDistancia();

  if (qFuzzyIsNull(dist)) { throw RuntimeException("Não foi possível determinar a distância!"); }

  int peso = 0;

  SqlQuery query;
  query.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    query.bindValue(":idProduto", modelItem.data(row, "idProduto"));

    if (not query.exec() or not query.first()) { throw RuntimeException("Erro buscando peso do produto: " + query.lastError().text()); }

    peso += modelItem.data(row, "caixas").toInt() * query.value("kgcx").toInt();
  }

  if (not query.exec("SELECT custoTransporteTon, custoTransporte1, custoTransporte2, custoFuncionario FROM loja WHERE nomeFantasia = 'Geral'") or not query.first()) {
    throw RuntimeException("Erro buscando parâmetros: " + query.lastError().text());
  }

  const double custoTon = query.value("custoTransporteTon").toDouble();
  const double custo1 = query.value("custoTransporte1").toDouble();
  const double custo2 = query.value("custoTransporte2").toDouble();
  const double custoFuncionario = query.value("custoFuncionario").toDouble();

  qDebug() << "peso: " << peso;

  int cargas = peso / 4500;
  int restante = peso % 4500;

  qDebug() << "inteiro: " << cargas;
  qDebug() << "resto: " << restante;

  // TODO: se endereco for 'nao há/retira' calcular apenas o valorPeso
  const double valorPeso = (peso / 1000.0 * custoTon);

  const double valorDistCargaCheia = (cargas * custo2 * dist) + (cargas * 3 * custoFuncionario);
  const double valorDistMeiaCarga =
      cargas > 0 and restante < 200 ? 0 : (restante < 2000 ? dist * custo1 + (2 * custoFuncionario / 2000.0 * restante) : dist * custo2 + (3 * custoFuncionario / 4500.0 * restante));
  const double valorFrete = valorPeso + valorDistCargaCheia + valorDistMeiaCarga;

  qDebug() << "valorPeso: " << valorPeso;
  qDebug() << "valorDistCargaCheia: " << valorDistCargaCheia;
  qDebug() << "valorDistMeiaCarga: " << valorDistMeiaCarga;
  qDebug() << "frete: " << valorFrete;

  // frete = (pesoProduto(ton.) * 180) + (pesoProduto < 2ton. ? dist. * 1.5 : pesoProduto < 4.5 ? dist. * 2 : fracionar cargas)
}

void Orcamento::on_dataEmissao_dateChanged(const QDate &date) { ui->spinBoxValidade->setMaximum(date.daysInMonth() - date.day()); }

void Orcamento::verificaDisponibilidadeEstoque() {
  SqlQuery query;

  QStringList produtos;

  for (int row = 0; row < modelItem.rowCount(); ++row) {
    if (modelItem.data(row, "estoque").toInt() != 1) { continue; }

    const QString idProduto = modelItem.data(row, "idProduto").toString();
    const QString quant = modelItem.data(row, "quant").toString();

    if (not query.exec("SELECT 0 FROM produto WHERE idProduto = " + idProduto + " AND estoqueRestante >= " + quant)) {
      throw RuntimeException("Erro verificando a disponibilidade do estoque: " + query.lastError().text());
    }

    if (not query.first()) { produtos << modelItem.data(row, "produto").toString(); }
  }

  if (not produtos.isEmpty()) {
    throw RuntimeError("Os seguintes produtos de estoque não estão mais disponíveis na quantidade selecionada:\n    -" + produtos.join("\n    -") + "\n\nRemova ou diminua a quant. para prosseguir!");
  }
}

// TODO: esse código está repetido em venda e searchDialog, refatorar
void Orcamento::on_pushButtonModelo3d_clicked() {
  const auto selection = ui->tableProdutos->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!"); }

  const int row = selection.first().row();

  const QString ip = qApp->getWebDavIp();
  const QString fornecedor = modelItem.data(row, "fornecedor").toString();
  const QString codComercial = modelItem.data(row, "codComercial").toString();

  const QString url = "http://" + ip + "/webdav/METAIS_VIVIANE/MODELOS 3D/" + fornecedor + "/" + codComercial + ".skp";

  auto *manager = new QNetworkAccessManager(this);
  auto request = QNetworkRequest(QUrl(url));
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  auto reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, this, [=] {
    if (reply->error() != QNetworkReply::NoError) {
      if (reply->error() == QNetworkReply::ContentNotFoundError) { throw RuntimeError("Produto não possui modelo 3D!"); }

      throw RuntimeException("Erro ao baixar arquivo: " + reply->errorString(), this);
    }

    const QString filename = QDir::currentPath() + "/arquivos/" + url.split("/").last();

    File file(filename);

    if (not file.open(QFile::WriteOnly)) { throw RuntimeException("Erro abrindo arquivo para escrita: " + file.errorString(), this); }

    file.write(reply->readAll());

    file.close();

    if (not QDesktopServices::openUrl(QUrl::fromLocalFile(filename))) { throw RuntimeException("Não foi possível abrir o arquivo 3D!"); }
  });
}

// NOTE: model.submitAll faz mapper voltar para -1, select tambem (talvez porque submitAll chama select)
// TODO: 0se produto for estoque permitir vender por peça (setar minimo/multiplo)
// TODO: 2orcamento de reposicao nao pode ter profissional associado (bloquear)
// TODO: 4quando cadastrar cliente no itemBox mudar para o id dele
// TODO: ?permitir que o usuario digite um valor e o sistema faça o calculo na linha?
// TODO: limitar o total ao frete? se o desconto é 100% e o frete não é zero, o minimo é o frete
// TODO: implementar mover linha para baixo/cima
//           1. colocar um botao com seta para cima e outro para baixo
//           2. para permitir reordenar os produtos colocar um campo oculto 'item' numerado sequencialmente, ai quando ler a tabela ordenar por essa coluna
// TODO: após gerar id permitir mudar vendedor apenas para os da mesma loja
// FIXME: orçamento permite adicionar o mesmo estoque duas vezes (e provavelmente faz o consumo duas vezes)
// TODO: antes de gerar excel/pdf salvar o arquivo para não ficar dados divergentes
