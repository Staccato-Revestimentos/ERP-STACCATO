#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "cadastroloja.h"
#include "cepcompleter.h"
#include "searchdialog.h"
#include "ui_cadastroloja.h"
#include "usersession.h"

CadastroLoja::CadastroLoja(QWidget *parent)
  : RegisterAddressDialog("loja", "idLoja", parent), ui(new Ui::CadastroLoja) {
  ui->setupUi(this);

  for (const auto *line : findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }

  setupUi();
  setupTables();
  setupMapper();
  newRegister();

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
  modelAlcadas.setTable("alcadas");
  modelAlcadas.setEditStrategy(QSqlTableModel::OnManualSubmit);
  modelAlcadas.setFilter("idLoja = " + QString::number(UserSession::idLoja()) + "");

  if (not modelAlcadas.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela de alçadas: " + modelAlcadas.lastError().text());
    return;
  }

  ui->tableAlcadas->setModel(&modelAlcadas);
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idAlcada"));
  ui->tableAlcadas->hideColumn(modelAlcadas.fieldIndex("idLoja"));
  ui->tableAlcadas->resizeColumnsToContents();

  ui->tableEndereco->setModel(&modelEnd);
  ui->tableEndereco->hideColumn("idEndereco");
  ui->tableEndereco->hideColumn("desativado");
  ui->tableEndereco->hideColumn("idLoja");
  ui->tableEndereco->hideColumn("codUF");
}

void CadastroLoja::clearFields() {
  for (auto const &line : findChildren<QLineEdit *>()) {
    line->clear();
  }
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
  ui->lineEditCNPJ->setStyleSheet(
        validaCNPJ(QString(text).remove(".").remove("/").remove("-")) ? "" : "color: rgb(255, 0, 0)");
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

bool CadastroLoja::viewRegister(const QModelIndex &index) {
  if (not RegisterDialog::viewRegister(index)) return false;

  modelEnd.setFilter("idLoja = " + data("idLoja").toString() + " AND desativado = FALSE");

  if (not modelEnd.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela endereço da loja: " + modelEnd.lastError().text());
    return false;
  }

  ui->tableEndereco->resizeColumnsToContents();

  return true;
}

void CadastroLoja::successMessage() { QMessageBox::information(this, "Atenção!", "Loja cadastrada com sucesso!"); }

void CadastroLoja::on_tableEndereco_entered(const QModelIndex &) { ui->tableEndereco->resizeColumnsToContents(); }
