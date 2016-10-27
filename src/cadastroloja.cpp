#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "cadastroloja.h"
#include "cepcompleter.h"
#include "porcentagemdelegate.h"
#include "searchdialog.h"
#include "ui_cadastroloja.h"
#include "usersession.h"

CadastroLoja::CadastroLoja(QWidget *parent)
    : RegisterAddressDialog("loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  //  for (auto const *line : findChildren<QLineEdit *>()) {
  //    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  //  }

  setupUi();
  setupTables();
  setupMapper();
  newRegister();

  ui->pushButtonAtualizarPagamento->hide();

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") {
    ui->pushButtonRemover->setDisabled(true);
    ui->pushButtonRemoverEnd->setDisabled(true);
  }
}

CadastroLoja::~CadastroLoja() { delete ui; }

void CadastroLoja::setupUi() {
  ui->lineEditCNPJ->setInputMask("99.999.999/9999-99;_");
  ui->lineEditSIGLA->setInputMask(">AANN;_");
  ui->lineEditCEP->setInputMask("99999-999;_");
  ui->lineEditUF->setInputMask(">AA;_");
}

void CadastroLoja::setupTables() {

  modelAssocia1.setTable("forma_pagamento");
  modelAssocia1.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelAssocia1.setHeaderData("pagamento", "Pagamento");
  modelAssocia1.setHeaderData("parcelas", "Parcelas");

  if (not modelAssocia1.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela forma_pagamento: " + modelAssocia1.lastError().text());
  }

  ui->tableAssocia1->setModel(&modelAssocia1);
  ui->tableAssocia1->hideColumn("idPagamento");

  //

  modelAssocia2.setTable("view_pagamento_loja");
  modelAssocia2.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelAssocia2.setHeaderData("pagamento", "Pagamento");

  if (not modelAssocia2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_pagamento_loja: " + modelAssocia2.lastError().text());
  }

  ui->tableAssocia2->setModel(&modelAssocia2);
  ui->tableAssocia2->hideColumn("idLoja");
  ui->tableAssocia2->hideColumn("idPagamento");
  //

  //  modelAlcadas.setTable("alcadas");
  modelAlcadas.setTable("usuario_has_tipo");
  modelAlcadas.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelAlcadas.setHeaderData("tipo", "Tipo");

  //  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::idLoja()));

  if (not modelAlcadas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de alçadas: " + modelAlcadas.lastError().text());
    return;
  }

  ui->tableAlcadas->setModel(&modelAlcadas);
  ui->tableAlcadas->hideColumn("idTipo");
  //  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idAlcada"));
  //  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idLoja"));
  ui->tableAlcadas->resizeColumnsToContents();

  //

  modelPermissoes.setTable("usuario_has_permissao");
  modelPermissoes.setEditStrategy(QSqlTableModel::OnManualSubmit);

  if (not modelPermissoes.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de permissões: " + modelPermissoes.lastError().text());
    return;
  }

  //

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn("idEndereco");
  ui->tableEndereco->hideColumn("desativado");
  ui->tableEndereco->hideColumn("idLoja");
  ui->tableEndereco->hideColumn("codUF");

  //

  modelConta.setTable("loja_has_conta");
  modelConta.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelConta.setFilter("idLoja = " + QString::number(UserSession::idLoja()));

  modelConta.setHeaderData("banco", "Banco");
  modelConta.setHeaderData("agencia", "Agência");
  modelConta.setHeaderData("conta", "Conta");

  //  if (not modelConta.select()) {
  //    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta: " + modelConta.lastError().text());
  //    return;
  //  }

  ui->tableConta->setModel(&modelConta);
  ui->tableConta->hideColumn("idConta");
  ui->tableConta->hideColumn("idLoja");
  ui->tableConta->hideColumn("desativado");

  //
  modelPagamentos.setTable("forma_pagamento");
  modelPagamentos.setEditStrategy(SqlTableModel::OnManualSubmit);

  modelPagamentos.setHeaderData("pagamento", "Pagamento");
  modelPagamentos.setHeaderData("parcelas", "Parcelas");

  //  modelPagamentos.select();

  ui->tablePagamentos->setModel(&modelPagamentos);
  ui->tablePagamentos->hideColumn("idPagamento");

  modelTaxas.setTable("forma_pagamento_has_taxa");
  modelTaxas.setEditStrategy(SqlTableModel::OnManualSubmit);

  modelTaxas.setHeaderData("parcela", "Quant. Parcelas");
  modelTaxas.setHeaderData("taxa", "Taxa");

  //  modelTaxas.setFilter("0");
  //  modelTaxas.select();

  ui->tableTaxas->setModel(&modelTaxas);
  ui->tableTaxas->hideColumn("idTaxa");
  ui->tableTaxas->hideColumn("idPagamento");
  ui->tableTaxas->setItemDelegateForColumn("taxa", new PorcentagemDelegate(this));
  //
}

void CadastroLoja::clearFields() {
  for (auto const &line : findChildren<QLineEdit *>()) line->clear();
}

bool CadastroLoja::verifyFields() {
  for (auto const &line : ui->groupBoxCadastro->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  return true;
}

bool CadastroLoja::savingProcedures() {
  if (not setData("descricao", ui->lineEditDescricao->text())) return false;
  if (not setData("razaoSocial", ui->lineEditRazaoSocial->text())) return false;
  if (not setData("sigla", ui->lineEditSIGLA->text())) return false;
  if (not setData("nomeFantasia", ui->lineEditNomeFantasia->text())) return false;
  if (not setData("cnpj", ui->lineEditCNPJ->text())) return false;
  if (not setData("inscEstadual", ui->lineEditInscEstadual->text())) return false;
  if (not setData("tel", ui->lineEditTel->text())) return false;
  if (not setData("tel2", ui->lineEditTel2->text())) return false;
  if (not setData("valorMinimoFrete", ui->doubleSpinBoxValorMinimoFrete->value())) return false;
  if (not setData("porcentagemFrete", ui->doubleSpinBoxPorcFrete->value())) return false;

  return true;
}

void CadastroLoja::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();

  ui->tabWidget->setTabEnabled(1, false);
  ui->tabWidget->setTabEnabled(2, false);
  ui->tabWidget->setTabEnabled(3, false);
}

void CadastroLoja::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroLoja::setupMapper() {
  addMapping(ui->doubleSpinBoxPorcFrete, "porcentagemFrete");
  addMapping(ui->doubleSpinBoxValorMinimoFrete, "valorMinimoFrete");
  addMapping(ui->lineEditCNPJ, "cnpj");
  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->lineEditInscEstadual, "inscEstadual");
  addMapping(ui->lineEditNomeFantasia, "nomeFantasia");
  addMapping(ui->lineEditRazaoSocial, "razaoSocial");
  addMapping(ui->lineEditSIGLA, "sigla");
  addMapping(ui->lineEditTel, "tel");
  addMapping(ui->lineEditTel2, "tel2");

  mapperEnd.addMapping(ui->comboBoxTipoEnd, modelEnd.fieldIndex("descricao"));
  mapperEnd.addMapping(ui->lineEditBairro, modelEnd.fieldIndex("bairro"));
  mapperEnd.addMapping(ui->lineEditCEP, modelEnd.fieldIndex("CEP"));
  mapperEnd.addMapping(ui->lineEditCidade, modelEnd.fieldIndex("cidade"));
  mapperEnd.addMapping(ui->lineEditComp, modelEnd.fieldIndex("complemento"));
  mapperEnd.addMapping(ui->lineEditLogradouro, modelEnd.fieldIndex("logradouro"));
  mapperEnd.addMapping(ui->lineEditNro, modelEnd.fieldIndex("numero"));
  mapperEnd.addMapping(ui->lineEditUF, modelEnd.fieldIndex("uf"));

  mapperConta.setModel(&modelConta);
  mapperConta.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  mapperConta.addMapping(ui->lineEditBanco, modelConta.fieldIndex("banco"));
  mapperConta.addMapping(ui->lineEditAgencia, modelConta.fieldIndex("agencia"));
  mapperConta.addMapping(ui->lineEditConta, modelConta.fieldIndex("conta"));

  mapperPagamento.setModel(&modelPagamentos);
  mapperPagamento.setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
  mapperPagamento.addMapping(ui->lineEditPagamento, modelPagamentos.fieldIndex("pagamento"));
  mapperPagamento.addMapping(ui->spinBoxParcelas, modelPagamentos.fieldIndex("parcelas"));
}

void CadastroLoja::on_pushButtonCadastrar_clicked() { save(); }

void CadastroLoja::on_pushButtonAtualizar_clicked() { update(); }

void CadastroLoja::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroLoja::on_pushButtonRemover_clicked() { remove(); }

void CadastroLoja::on_pushButtonBuscar_clicked() {
  SearchDialog *sdLoja = SearchDialog::loja(this);
  connect(sdLoja, &SearchDialog::itemSelected, this, &CadastroLoja::viewRegisterById);
  sdLoja->show();
}

void CadastroLoja::on_lineEditCNPJ_textEdited(const QString &text) {
  ui->lineEditCNPJ->setStyleSheet(validaCNPJ(QString(text).remove(".").remove("/").remove("-"))
                                      ? "background-color: rgb(255, 255, 127)"
                                      : "background-color: rgb(255, 255, 127);color: rgb(255, 0, 0)");
}

void CadastroLoja::on_pushButtonAdicionarEnd_clicked() {
  if (not cadastrarEndereco(false)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar este endereço.");
    return;
  }

  novoEndereco();
}

void CadastroLoja::on_pushButtonAtualizarEnd_clicked() {
  if (not cadastrarEndereco(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível atualizar este endereço.");
    return;
  }

  novoEndereco();
}

void CadastroLoja::on_pushButtonEndLimpar_clicked() { novoEndereco(); }

void CadastroLoja::on_pushButtonRemoverEnd_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Remover");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not setDataEnd("desativado", true)) {
      QMessageBox::critical(this, "Erro!", "Erro marcando desativado!");
      return;
    }

    if (not modelEnd.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelEnd.lastError().text());
      return;
    }

    novoEndereco();
  }
}

void CadastroLoja::on_checkBoxMostrarInativos_clicked(const bool &checked) {
  modelEnd.setFilter("idLoja = " + data("idLoja").toString() + (checked ? "" : " AND desativado = FALSE"));

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço: " + modelEnd.lastError().text());
  }

  ui->tableEndereco->resizeColumnsToContents();
}

bool CadastroLoja::cadastrarEndereco(const bool &isUpdate) {
  for (auto const &line : ui->groupBoxEndereco->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  if (not ui->lineEditCEP->isValid()) {
    ui->lineEditCEP->setFocus();
    QMessageBox::critical(this, "Erro!", "CEP inválido!");
    return false;
  }

  rowEnd = isUpdate ? mapperEnd.currentIndex() : modelEnd.rowCount();

  if (not isUpdate) modelEnd.insertRow(rowEnd);

  if (not setDataEnd("descricao", ui->comboBoxTipoEnd->currentText())) return false;
  if (not setDataEnd("cep", ui->lineEditCEP->text())) return false;
  if (not setDataEnd("logradouro", ui->lineEditLogradouro->text())) return false;
  if (not setDataEnd("numero", ui->lineEditNro->text())) return false;
  if (not setDataEnd("complemento", ui->lineEditComp->text())) return false;
  if (not setDataEnd("bairro", ui->lineEditBairro->text())) return false;
  if (not setDataEnd("cidade", ui->lineEditCidade->text())) return false;
  if (not setDataEnd("uf", ui->lineEditUF->text())) return false;
  if (not setDataEnd("codUF", getCodigoUF(ui->lineEditUF->text()))) return false;
  if (not setDataEnd("desativado", false)) return false;

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroLoja::novoEndereco() {
  ui->pushButtonAtualizarEnd->hide();
  ui->pushButtonAdicionarEnd->show();
  ui->tableEndereco->clearSelection();
  clearEndereco();
}

void CadastroLoja::clearEndereco() {
  ui->lineEditBairro->clear();
  ui->lineEditCEP->clear();
  ui->lineEditCidade->clear();
  ui->lineEditComp->clear();
  ui->lineEditLogradouro->clear();
  ui->lineEditNro->clear();
  ui->lineEditUF->clear();
}

void CadastroLoja::on_lineEditCEP_textChanged(const QString &cep) {
  if (not ui->lineEditCEP->isValid()) return;

  ui->lineEditNro->clear();
  ui->lineEditComp->clear();

  CepCompleter cc;

  if (not cc.buscaCEP(cep)) {
    QMessageBox::warning(this, "Aviso!", "CEP não encontrado!");
    return;
  }

  ui->lineEditUF->setText(cc.getUf());
  ui->lineEditCidade->setText(cc.getCidade());
  ui->lineEditLogradouro->setText(cc.getEndereco());
  ui->lineEditBairro->setText(cc.getBairro());
}

void CadastroLoja::on_tableEndereco_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarEnd->show();
  ui->pushButtonAdicionarEnd->hide();
  mapperEnd.setCurrentModelIndex(index);
}

bool CadastroLoja::viewRegister() {
  if (not RegisterDialog::viewRegister()) return false;

  modelEnd.setFilter("idLoja = " + primaryId + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço da loja: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  //

  modelConta.setFilter("idLoja = " + primaryId + " AND desativado = FALSE");

  if (not modelConta.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta: " + modelConta.lastError().text());
    return false;
  }

  ui->tableConta->resizeColumnsToContents();

  //  modelPagamentos.setFilter("idLoja = " + primaryId);

  if (not modelPagamentos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pagamento: " + modelPagamentos.lastError().text());
    return false;
  }

  modelTaxas.setFilter("0");

  ui->tabWidget->setTabEnabled(1, true);
  ui->tabWidget->setTabEnabled(2, true);
  ui->tabWidget->setTabEnabled(3, true);

  //

  modelAssocia2.setFilter("idLoja = " + data("idLoja").toString());

  if (not modelAssocia2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_pagamento_loja: " + modelAssocia2.lastError().text());
  }

  //

  return true;
}

void CadastroLoja::successMessage() { QMessageBox::information(this, "Atenção!", "Loja cadastrada com sucesso!"); }

void CadastroLoja::on_tableEndereco_entered(const QModelIndex &) { ui->tableEndereco->resizeColumnsToContents(); }

bool CadastroLoja::cadastrar() {
  if (not verifyFields()) return false;

  row = isUpdate ? mapper.currentIndex() : model.rowCount();

  if (row == -1) {
    QMessageBox::critical(this, "Erro!", "Erro linha -1");
    return false;
  }

  if (not isUpdate and not model.insertRow(row)) return false;

  if (not savingProcedures()) return false;

  for (int column = 0; column < model.rowCount(); ++column) {
    const QVariant dado = model.data(row, column);
    if (dado.type() == QVariant::String) {
      if (not model.setData(row, column, dado.toString().toUpper())) return false;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + model.lastError().text());
    return false;
  }

  primaryId =
      data(row, primaryKey).isValid() ? data(row, primaryKey).toString() : model.query().lastInsertId().toString();

  //

  for (int row = 0, rowCount = modelEnd.rowCount(); row < rowCount; ++row) {
    if (not modelEnd.setData(row, primaryKey, primaryId)) return false;
  }

  for (int column = 0; column < modelEnd.rowCount(); ++column) {
    const QVariant dado = modelEnd.data(row, column);
    if (dado.type() == QVariant::String) {
      if (not modelEnd.setData(row, column, dado.toString().toUpper())) return false;
    }
  }

  if (not modelEnd.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + modelEnd.lastError().text());
    return false;
  }

  //

  for (int row = 0; row < modelConta.rowCount(); ++row) {
    if (not modelConta.setData(row, primaryKey, primaryId)) return false;
  }

  if (not modelConta.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro: " + modelConta.lastError().text());
    return false;
  }

  //

  return true;
}

bool CadastroLoja::save() {
  QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec();
  QSqlQuery("START TRANSACTION").exec();

  if (not cadastrar()) {
    QSqlQuery("ROLLBACK").exec();
    return false;
  }

  QSqlQuery("COMMIT").exec();

  isDirty = false;

  viewRegisterById(primaryId);

  if (not silent) successMessage();

  return true;
}

void CadastroLoja::on_tableConta_clicked(const QModelIndex &index) {
  ui->pushButtonAtualizarConta->show();
  ui->pushButtonAdicionarConta->hide();
  mapperConta.setCurrentModelIndex(index);
}

bool CadastroLoja::newRegister() {
  if (not confirmationMessage()) return false;

  isUpdate = false;

  clearFields();
  registerMode();

  modelEnd.setFilter("idEndereco = 0");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço: " + modelEnd.lastError().text());
    return false;
  }

  modelConta.setFilter("idLoja = 0");

  if (not modelConta.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela conta: " + modelConta.lastError().text());
    return false;
  }

  return true;
}

bool CadastroLoja::cadastrarConta(const bool &isUpdate) {
  for (auto const &line : ui->groupBoxConta->findChildren<QLineEdit *>()) {
    if (not verifyRequiredField(line)) return false;
  }

  const int rowConta = isUpdate ? mapperConta.currentIndex() : modelConta.rowCount();

  if (not isUpdate) modelConta.insertRow(rowConta);

  if (not modelConta.setData(rowConta, "banco", ui->lineEditBanco->text())) return false;
  if (not modelConta.setData(rowConta, "agencia", ui->lineEditAgencia->text())) return false;
  if (not modelConta.setData(rowConta, "conta", ui->lineEditConta->text())) return false;

  ui->tableConta->resizeColumnsToContents();

  return true;
}

void CadastroLoja::novaConta() {
  ui->pushButtonAtualizarConta->hide();
  ui->pushButtonAdicionarConta->show();
  ui->tableConta->clearSelection();
  clearConta();
}

void CadastroLoja::clearConta() {
  ui->lineEditBanco->clear();
  ui->lineEditAgencia->clear();
  ui->lineEditConta->clear();
}

void CadastroLoja::on_pushButtonAdicionarConta_clicked() {
  if (not cadastrarConta(false)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar esta conta.");
    return;
  }

  novaConta();
}

void CadastroLoja::on_pushButtonAtualizarConta_clicked() {
  if (not cadastrarConta(true)) {
    QMessageBox::critical(this, "Erro!", "Não foi possível cadastrar esta conta.");
    return;
  }

  novaConta();
}

void CadastroLoja::on_pushButtonContaLimpar_clicked() { novaConta(); }

void CadastroLoja::on_pushButtonRemoverConta_clicked() {
  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja remover?",
                     QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Remover");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::Yes) {
    if (not modelConta.setData(mapperConta.currentIndex(), "desativado", true)) {
      QMessageBox::critical(this, "Erro!", "Erro marcando desativado!");
      return;
    }

    if (not modelConta.submitAll()) {
      QMessageBox::critical(this, "Erro!", "Não foi possível remover este item: " + modelConta.lastError().text());
      return;
    }

    novaConta();
  }
}

void CadastroLoja::on_checkBoxMostrarInativosConta_clicked(bool checked) {
  modelConta.setFilter("idLoja = " + data("idLoja").toString() + (checked ? "" : " AND desativado = FALSE"));

  if (not modelConta.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela contas: " + modelConta.lastError().text());
  }

  ui->tableEndereco->resizeColumnsToContents();
}

void CadastroLoja::on_pushButtonAdicionarPagamento_clicked() { // TODO: wrap in a transaction and test setdatas?
  const int row = modelPagamentos.rowCount();
  modelPagamentos.insertRow(row);

  //  modelPagamentos.setData(row, "idLoja", data("idLoja"));
  modelPagamentos.setData(row, "pagamento", ui->lineEditPagamento->text());
  modelPagamentos.setData(row, "parcelas", ui->spinBoxParcelas->value());

  modelPagamentos.submitAll();

  const int id = modelPagamentos.query().lastInsertId().toInt();

  for (int i = 0; i < ui->spinBoxParcelas->value(); ++i) {
    const int row = modelTaxas.rowCount();
    modelTaxas.insertRow(row);

    modelTaxas.setData(row, "idPagamento", id);
    modelTaxas.setData(row, "parcela", i + 1);
    modelTaxas.setData(row, "taxa", 0);
  }

  modelTaxas.submitAll();

  modelTaxas.setFilter("idPagamento = " + QString::number(id));

  if (not modelTaxas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela taxas: " + modelTaxas.lastError().text());
  }
}

void CadastroLoja::on_tablePagamentos_clicked(const QModelIndex &index) {
  const int id = modelPagamentos.data(index.row(), "idPagamento").toInt();

  modelTaxas.setFilter("idPagamento = " + QString::number(id));

  if (not modelTaxas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela taxas: " + modelTaxas.lastError().text());
  }

  mapperPagamento.setCurrentModelIndex(index);

  ui->pushButtonAdicionarPagamento->hide();
  ui->pushButtonAtualizarPagamento->show();
}

void CadastroLoja::on_pushButtonRemoverPagamento_clicked() {
  const auto list = ui->tablePagamentos->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  const int id = modelPagamentos.data(list.first().row(), "idPagamento").toInt();

  QSqlQuery query;
  query.prepare("DELETE FROM forma_pagamento WHERE idPagamento = :idPagamento");
  query.bindValue(":idPagamento", id);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro apagando pagamentos: " + query.lastError().text());
    return;
  }

  if (not modelPagamentos.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela pagamentos: " + modelPagamentos.lastError().text());
  }

  modelTaxas.setFilter("0");

  ui->lineEditPagamento->clear();
  ui->spinBoxParcelas->clear();

  ui->pushButtonAtualizarPagamento->hide();
  ui->pushButtonAdicionarPagamento->show();
}

void CadastroLoja::on_pushButtonAtualizarPagamento_clicked() {
  modelPagamentos.setData(mapperPagamento.currentIndex(), "pagamento", ui->lineEditPagamento->text());
  modelPagamentos.setData(mapperPagamento.currentIndex(), "parcelas", ui->spinBoxParcelas->value());

  modelTaxas.removeRows(0, modelTaxas.rowCount());

  for (int i = 0; i < ui->spinBoxParcelas->value(); ++i) {
    const int row = modelTaxas.rowCount();
    modelTaxas.insertRow(row);

    modelTaxas.setData(row, "idPagamento", modelPagamentos.data(mapperPagamento.currentIndex(), "idPagamento"));
    modelTaxas.setData(row, "parcela", i + 1);
    modelTaxas.setData(row, "taxa", 0);
  }

  if (not modelPagamentos.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando dados: " + modelPagamentos.lastError().text());
  }

  if (not modelTaxas.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando taxas: " + modelTaxas.lastError().text());
  }

  modelTaxas.setFilter("0");

  ui->lineEditPagamento->clear();
  ui->spinBoxParcelas->clear();

  ui->pushButtonAtualizarPagamento->hide();
  ui->pushButtonAdicionarPagamento->show();
}

void CadastroLoja::on_pushButtonAtualizarTaxas_clicked() {
  if (not modelTaxas.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando taxas: " + modelTaxas.lastError().text());
  }

  QMessageBox::information(this, "Aviso!", "Taxas atualizadas!");
}

void CadastroLoja::on_pushButtonCadastrarTipo_clicked() {
  const int row = modelAlcadas.rowCount();
  modelAlcadas.insertRow(row);

  modelAlcadas.setData(row, "tipo", ui->lineEditTipo->text());

  if (not modelAlcadas.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro cadastrando tipo: " + modelAlcadas.lastError().text());
  }

  if (not modelAlcadas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela alçadas: " + modelAlcadas.lastError().text());
  }

  ui->tableAlcadas->resizeColumnsToContents();
}

void CadastroLoja::on_pushButtonAdicionaAssociacao_clicked() {
  const auto list = ui->tableAssocia1->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &index : list) {
    QSqlQuery query;
    query.prepare("INSERT INTO forma_pagamento_has_loja (idPagamento, idLoja) VALUES (:idPagamento, :idLoja)");
    query.bindValue(":idPagamento", modelAssocia1.data(index.row(), "idPagamento"));
    query.bindValue(":idLoja", data("idLoja").toInt());

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro cadastrando associacao: " + query.lastError().text());
    }
  }

  if (not modelAssocia2.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela view_pagamento_loja: " + modelAssocia2.lastError().text());
  }
}

void CadastroLoja::on_pushButtonRemoveAssociacao_clicked() {
  const auto list = ui->tableAssocia2->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  for (auto const &index : list) {
    QSqlQuery query;
    query.prepare("DELETE FROM forma_pagamento_has_loja WHERE idPagamento = :idPagamento AND idLoja = :idLoja");
    query.bindValue(":idPagamento", modelAssocia2.data(index.row(), "idPagamento"));
    query.bindValue(":idLoja", modelAssocia2.data(index.row(), "idLoja"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro removendo associacao: " + query.lastError().text());
    }
  }

  modelAssocia2.select();
}

// TODO: remover aba 'alcadas' e passar funcionalidade para cadastroUsuario
// TODO: montar aba alcadas
// TODO: montar aba pagamentos
// TODO: add 'created' and 'lastUpdated' to tables in bd
