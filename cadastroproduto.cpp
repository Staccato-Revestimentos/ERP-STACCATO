#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "cadastrocliente.h"
#include "cadastroproduto.h"
#include "ui_cadastroproduto.h"
#include "usersession.h"

CadastroProduto::CadastroProduto(QWidget *parent)
  : RegisterDialog("Produto", "idProduto", parent), ui(new Ui::CadastroProduto) {
  ui->setupUi(this);
  // InputMasks
  //  ui->lineEditCPOF->setInputMask("9999;_");
  ui->lineEditCodBarras->setInputMask("9999999999999;_");
  ui->lineEditNCM->setInputMask("99999999;_");

  ui->lineEditPcCx->setValidator(new QIntValidator(0, 9999, this));
  //  ui->lineEditComissao->setValidator(new QDoubleValidator(0.0,100.0,2,this));
  ui->lineEditEstoque->setValidator(new QIntValidator(0, 9999, this));

  ui->comboBoxOrigem->addItem("0 - Nacional", 0);
  ui->comboBoxOrigem->addItem("1 - Imp. Direta", 1);
  ui->comboBoxOrigem->addItem("2 - Merc. Interno", 2);

  setupMapper();
  newRegister();

  if (UserSession::getTipo() == "VENDEDOR") {
    ui->pushButtonRemover->setDisabled(true);
  }

//  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  SearchDialog *sdFornecedor = SearchDialog::fornecedor(this);
  ui->itemBoxFornecedor->setSearchDialog(sdFornecedor);

//  CadastroCliente *cadFornecedor = new CadastroCliente(this);
  CadastroCliente *cadFornecedor = new CadastroCliente(this);
  cadFornecedor->setTipo("PJ");
  ui->itemBoxFornecedor->setRegisterDialog(cadFornecedor);
}

CadastroProduto::~CadastroProduto() {
  delete ui;
}

// void CadastroProduto::updateComboboxFornecedor() {
//  ui->comboBoxFornecedor->clear();
//  ui->comboBoxFornecedor->addItem("Selecione um fornecedor!");
//  QSqlQuery query("SELECT idCadastro, razaoSocial FROM Cadastro WHERE tipo ='FORNECEDOR' or tipo =
//  'AMBOS'");
//  if (query.exec()) {
//    while (query.next()) {
//      QString str = query.value(0).toString() + " - " + query.value(1).toString();
//      ui->comboBoxFornecedor->addItem(str, query.value(0));
//    }
//  }
//}

void CadastroProduto::clearFields() {
  //  foreach (QComboBox *box, this->findChildren<QComboBox *>()) {
  //    box->setCurrentIndex(0);
  //  }
  foreach (QLineEdit *line, this->findChildren<QLineEdit *>()) {
    line->clear();
  }
  ui->radioButtonDesc->setChecked(false);
  ui->radioButtonLote->setChecked(false);
}

void CadastroProduto::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroProduto::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

bool CadastroProduto::verifyFields() {
  // TODO : VerifyFields Produto
  if(!RegisterDialog::verifyFields({ui->lineEditDescricao,ui->lineEditUn, ui->lineEditNCM, ui->lineEditCusto, ui->lineEditVenda}))
    return false;

  if (ui->itemBoxFornecedor->getValue().isNull()) {
    ui->itemBoxFornecedor->setFocus();
    QMessageBox::warning(this, "Atenção!", "Você não preencheu um item obrigatório!", QMessageBox::Ok,
                         QMessageBox::NoButton);
    return false;
  }

//  if (ui->comboBoxOrigem->currentData().isNull()) {
//    ui->comboBoxOrigem->setFocus();
//    QMessageBox::warning(this, "Atenção!", "Você não preencheu um item obrigatório!", QMessageBox::Ok,
//                         QMessageBox::NoButton);
//    return false;
//  }

  return true;
}

void CadastroProduto::setupMapper() {
  // Não funciona do jeito que eu quero!!
  mapper.setModel(&model);
  addMapping(ui->lineEditDescricao, "descricao");
  addMapping(ui->lineEditUn, "un");
  // addMapping(ui->lineEditRetBold,"retBold");
  addMapping(ui->lineEditColecao, "colecao");
  // addMapping(ui->lineEditTipo,"tipo");
  addMapping(ui->lineEditFormComer, "formComercial");
  addMapping(ui->lineEditCodComer, "codComercial");
  addMapping(ui->lineEditCodInd, "codIndustrial");
  addMapping(ui->lineEditCodBarras, "codBarras");
  addMapping(ui->lineEditNCM, "ncm");
  // addMapping(ui->lineEditCPOF,"cpof");
  addMapping(ui->lineEditICMS, "icms");
  addMapping(ui->lineEditPcCx, "pccx");
  // Double
  addMapping(ui->lineEditM2Cx, "m2cx", "value");
  addMapping(ui->lineEditQtePallet, "qtdPallet", "value");
  addMapping(ui->lineEditCusto, "custo", "value");
  addMapping(ui->lineEditIPI, "ipi", "value");
  addMapping(ui->lineEditST, "st", "value");
  addMapping(ui->lineEditMarkup, "markup", "value");
  addMapping(ui->lineEditVenda, "precoVenda", "value");
  addMapping(ui->lineEditComissao, "comissao", "value");

  // textEdit
  addMapping(ui->textEditObserv, "observacoes", "plainText");
  addMapping(ui->comboBoxSitTrib, "situacaoTributaria");
  addMapping(ui->itemBoxFornecedor, "idFornecedor", "value");
  addMapping(ui->comboBoxOrigem, "origem", "currentData");
  addMapping(ui->radioButtonDesc, "descontinuado");
  addMapping(ui->radioButtonLote, "temLote");
  addMapping(ui->lineEditEstoque, "estoque");
}

bool CadastroProduto::savingProcedures(int row) {
  setData(row, "Fornecedor", ui->itemBoxFornecedor->text());
  setData(row, "idFornecedor", ui->itemBoxFornecedor->getValue());

  setData(row, "temLote", ui->radioButtonLote->isChecked());
  setData(row, "descontinuado", ui->radioButtonDesc->isChecked());
  setData(row, "origem", ui->comboBoxOrigem->currentData());
  setData(row, "sitTrib", ui->comboBoxSitTrib->currentText());

  setData(row, "descricao", ui->lineEditDescricao->text() );
  setData(row, "un", ui->lineEditUn->text() );
  setData(row, "colecao", ui->lineEditColecao->text() );
  setData(row, "formComercial", ui->lineEditFormComer->text() );
  setData(row, "codComercial", ui->lineEditCodComer->text() );
  setData(row, "codIndustrial", ui->lineEditCodInd->text() );
  setData(row, "codBarras", ui->lineEditCodBarras->text() );
  setData(row, "ncm", ui->lineEditNCM->text() );
  setData(row, "icms", ui->lineEditICMS->text() );
  setData(row, "pccx", ui->lineEditPcCx->text() );
  setData(row, "m2cx", ui->lineEditM2Cx->getValue() );
  setData(row, "qtdPallet", ui->lineEditQtePallet->getValue() );
  setData(row, "custo", ui->lineEditCusto->getValue() );
  setData(row, "ipi", ui->lineEditIPI->getValue() );
  setData(row, "st", ui->lineEditST->getValue() );
  setData(row, "markup", ui->lineEditMarkup->getValue() );
  setData(row, "precoVenda", ui->lineEditVenda->getValue() );
  setData(row, "comissao", ui->lineEditComissao->getValue() );

  setData(row, "obsercacoes", ui->textEditObserv->toPlainText());
  setData(row, "situacaoTributaria", ui->comboBoxSitTrib->currentText());
  setData(row, "origem", ui->comboBoxOrigem->currentData());
  setData(row, "descontinuado", ui->radioButtonDesc->isChecked());
  setData(row, "temLote", ui->radioButtonLote->isChecked());
  setData(row, "estoque", ui->lineEditEstoque->text());

  return true;
}

// void CadastroProduto::on_pushButtonCadForn_clicked() {
////  CadastroCliente *cad = new CadastroCliente(this);
////  connect(cad, &QDialog::finished, this, &CadastroProduto::updateComboboxFornecedor);
//  updateComboboxFornecedor();
//}

void CadastroProduto::on_pushButtonCadastrar_clicked() {
  save();
}

void CadastroProduto::on_pushButtonAtualizar_clicked() {
  save();
}

void CadastroProduto::on_pushButtonNovoCad_clicked() {
  newRegister();
}

void CadastroProduto::on_pushButtonRemover_clicked() {
  remove();
}

void CadastroProduto::on_pushButtonCancelar_clicked() {
  close();
}

void CadastroProduto::on_pushButtonBuscar_clicked() {
  SearchDialog *sdProd = SearchDialog::produto(this);
  sdProd->show();
  connect(sdProd, &SearchDialog::itemSelected, this, &CadastroProduto::changeItem);
}

void CadastroProduto::changeItem(QVariant value, QString text) {
  Q_UNUSED(text)
  viewRegisterById(value);
}
