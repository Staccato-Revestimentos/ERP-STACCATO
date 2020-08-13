#include "inputdialogproduto.h"
#include "ui_inputdialogproduto.h"

#include "application.h"
#include "editdelegate.h"
#include "noeditdelegate.h"
#include "porcentagemdelegate.h"
#include "reaisdelegate.h"
#include "sortfilterproxymodel.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

InputDialogProduto::InputDialogProduto(const Tipo &tipo, QWidget *parent) : QDialog(parent), tipo(tipo), ui(new Ui::InputDialogProduto) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  ui->labelAliquota->hide();
  ui->doubleSpinBoxAliquota->hide();
  ui->labelST->hide();
  ui->doubleSpinBoxST->hide();
  ui->lineEditCodRep->hide();

  ui->dateEditEvento->setDate(qApp->serverDate());
  ui->dateEditProximo->setDate(qApp->serverDate());

  if (tipo == Tipo::GerarCompra) {
    ui->labelEvento->setText("Data compra:");
    ui->labelProximoEvento->setText("Data prevista confirmação:");

    connect(ui->table->model(), &QAbstractItemModel::dataChanged, this, &InputDialogProduto::updateTableData);
  }

  if (tipo == Tipo::Faturamento) {
    ui->labelProximoEvento->hide();
    ui->dateEditProximo->hide();
    ui->comboBoxST->hide();
    ui->labelAliquota->hide();
    ui->doubleSpinBoxAliquota->hide();
    ui->labelST->hide();
    ui->doubleSpinBoxST->hide();

    ui->labelEvento->setText("Data faturamento:");
  }

  setConnections();

  adjustSize();

  showMaximized();
}

InputDialogProduto::~InputDialogProduto() { delete ui; }

void InputDialogProduto::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->comboBoxST, &QComboBox::currentTextChanged, this, &InputDialogProduto::on_comboBoxST_currentTextChanged, connectionType);
  connect(ui->dateEditEvento, &QDateEdit::dateChanged, this, &InputDialogProduto::on_dateEditEvento_dateChanged, connectionType);
  connect(ui->doubleSpinBoxAliquota, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InputDialogProduto::on_doubleSpinBoxAliquota_valueChanged, connectionType);
  connect(ui->doubleSpinBoxST, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InputDialogProduto::on_doubleSpinBoxST_valueChanged, connectionType);
  connect(ui->pushButtonSalvar, &QPushButton::clicked, this, &InputDialogProduto::on_pushButtonSalvar_clicked, connectionType);
  connect(ui->lineEditCodRep, &QLineEdit::textEdited, this, &InputDialogProduto::on_lineEditCodRep_textEdited, connectionType);
}

void InputDialogProduto::unsetConnections() {
  disconnect(ui->comboBoxST, &QComboBox::currentTextChanged, this, &InputDialogProduto::on_comboBoxST_currentTextChanged);
  disconnect(ui->dateEditEvento, &QDateEdit::dateChanged, this, &InputDialogProduto::on_dateEditEvento_dateChanged);
  disconnect(ui->doubleSpinBoxAliquota, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InputDialogProduto::on_doubleSpinBoxAliquota_valueChanged);
  disconnect(ui->doubleSpinBoxST, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &InputDialogProduto::on_doubleSpinBoxST_valueChanged);
  disconnect(ui->pushButtonSalvar, &QPushButton::clicked, this, &InputDialogProduto::on_pushButtonSalvar_clicked);
  disconnect(ui->lineEditCodRep, &QLineEdit::textEdited, this, &InputDialogProduto::on_lineEditCodRep_textEdited);
}

void InputDialogProduto::setupTables() {
  if (tipo == Tipo::GerarCompra) { modelPedidoFornecedor.setTable("pedido_fornecedor_has_produto"); }
  if (tipo == Tipo::Faturamento) { modelPedidoFornecedor.setTable("pedido_fornecedor_has_produto2"); }

  modelPedidoFornecedor.setHeaderData("ordemRepresentacao", "Cód. Rep.");
  modelPedidoFornecedor.setHeaderData("idVenda", "Código");
  modelPedidoFornecedor.setHeaderData("fornecedor", "Fornecedor");
  modelPedidoFornecedor.setHeaderData("descricao", "Produto");
  modelPedidoFornecedor.setHeaderData("colecao", "Coleção");
  modelPedidoFornecedor.setHeaderData("caixas", "Caixas");
  modelPedidoFornecedor.setHeaderData("prcUnitario", "$ Unit.");
  modelPedidoFornecedor.setHeaderData("quant", "Quant.");
  modelPedidoFornecedor.setHeaderData("preco", "Total");
  modelPedidoFornecedor.setHeaderData("un", "Un.");
  modelPedidoFornecedor.setHeaderData("un2", "Un.2");
  modelPedidoFornecedor.setHeaderData("kgcx", "Kg./Cx.");
  modelPedidoFornecedor.setHeaderData("formComercial", "Formato");
  modelPedidoFornecedor.setHeaderData("codComercial", "Cód. Com.");
  modelPedidoFornecedor.setHeaderData("obs", "Obs.");
  modelPedidoFornecedor.setHeaderData("aliquotaSt", "Alíquota ST");
  modelPedidoFornecedor.setHeaderData("st", "ST");

  modelPedidoFornecedor.proxyModel = new SortFilterProxyModel(&modelPedidoFornecedor, this);

  ui->table->setModel(&modelPedidoFornecedor);

  ui->table->hideColumn("idRelacionado");
  ui->table->hideColumn("idVendaProduto1");
  ui->table->hideColumn("idVendaProduto2");
  ui->table->hideColumn("statusFinanceiro");
  ui->table->hideColumn("ordemCompra");
  ui->table->hideColumn("quantUpd");
  ui->table->hideColumn("selecionado");
  if (tipo == Tipo::GerarCompra) { ui->table->hideColumn("idPedido1"); }
  if (tipo == Tipo::Faturamento) {
    ui->table->hideColumn("idPedido2");
    ui->table->hideColumn("idPedidoFK");
  }
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("codBarras");
  ui->table->hideColumn("idCompra");
  ui->table->hideColumn("status");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealFat");
  ui->table->hideColumn("dataPrevColeta");
  ui->table->hideColumn("dataRealColeta");
  ui->table->hideColumn("dataPrevReceb");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");

  ui->table->setItemDelegate(new NoEditDelegate(this));

  if (tipo == Tipo::GerarCompra) {
    ui->table->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
    ui->table->setItemDelegateForColumn("preco", new ReaisDelegate(this));
    ui->table->setItemDelegateForColumn("aliquotaSt", new PorcentagemDelegate(false, this));
    ui->table->setItemDelegateForColumn("quant", new EditDelegate(this));
    ui->table->setItemDelegateForColumn("caixas", new EditDelegate(this));
  }

  if (tipo == Tipo::Faturamento) { ui->table->setItemDelegateForColumn("ordemRepresentacao", new EditDelegate(this)); }
}

bool InputDialogProduto::setFilter(const QStringList &ids) {
  if (ids.isEmpty()) { return qApp->enqueueException(false, "Ids vazio!", this); }

  QString filter;

  if (tipo == Tipo::GerarCompra) { filter = "`idPedido1` IN (" + ids.join(", ") + ") AND status = 'PENDENTE'"; }
  if (tipo == Tipo::Faturamento) { filter = "idCompra IN (" + ids.join(", ") + ") AND status = 'EM FATURAMENTO'"; }

  if (filter.isEmpty()) { return qApp->enqueueException(false, "Filtro vazio!", this); }

  modelPedidoFornecedor.setFilter(filter);

  if (not modelPedidoFornecedor.select()) { return false; }

  calcularTotal();

  QSqlQuery query;
  query.prepare("SELECT aliquotaSt, st, representacao FROM fornecedor WHERE razaoSocial = :razaoSocial");
  query.bindValue(":razaoSocial", modelPedidoFornecedor.data(0, "fornecedor"));

  if (not query.exec() or not query.first()) { return qApp->enqueueException(false, "Erro buscando substituicao tributaria do fornecedor: " + query.lastError().text(), this); }

  ui->comboBoxST->setCurrentText(query.value("st").toString());
  ui->doubleSpinBoxAliquota->setValue(query.value("aliquotaSt").toDouble());

  if (query.value("representacao").toBool() and tipo == Tipo::Faturamento) { ui->lineEditCodRep->show(); }

  if (tipo == Tipo::GerarCompra) { qApp->enqueueInformation("Ajustar preço e quantidade se necessário!", this); }

  return true;
}

QDate InputDialogProduto::getDate() const { return ui->dateEditEvento->date(); }

QDate InputDialogProduto::getNextDate() const { return ui->dateEditProximo->date(); }

void InputDialogProduto::updateTableData(const QModelIndex &topLeft) {
  disconnect(ui->table->model(), &QAbstractItemModel::dataChanged, this, &InputDialogProduto::updateTableData);

  [&] {
    const QString header = modelPedidoFornecedor.headerData(topLeft.column(), Qt::Horizontal).toString();
    const int row = topLeft.row();

    // TODO: se alterar quant. tem que alterar caixas

    if (header == "Quant." or header == "$ Unit.") {
      const double preco = modelPedidoFornecedor.data(row, "quant").toDouble() * modelPedidoFornecedor.data(row, "prcUnitario").toDouble();
      if (not modelPedidoFornecedor.setData(row, "preco", preco)) { return; }
    }

    if (header == "Total") {
      const double preco = modelPedidoFornecedor.data(row, "preco").toDouble() / modelPedidoFornecedor.data(row, "quant").toDouble();
      if (not modelPedidoFornecedor.setData(row, "prcUnitario", preco)) { return; }
    }
  }();

  connect(ui->table->model(), &QAbstractItemModel::dataChanged, this, &InputDialogProduto::updateTableData);

  calcularTotal();
}

void InputDialogProduto::calcularTotal() {
  double total = 0;

  for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) { total += modelPedidoFornecedor.data(row, "preco").toDouble(); }

  ui->doubleSpinBoxTotal->setValue(total + ui->doubleSpinBoxST->value());

  ui->doubleSpinBoxST->setMaximum(total * 0.2);
}

void InputDialogProduto::on_pushButtonSalvar_clicked() {
  if (ui->lineEditCodRep->isVisible() and ui->lineEditCodRep->text().isEmpty()) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Não preencheu 'Cód. Rep.'. Continuar?", QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Salvar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) { return; }
  }

  if (not cadastrar()) { return; }

  QDialog::accept();
  close();
}

bool InputDialogProduto::cadastrar() {
  if (not qApp->startTransaction("InputDialog::cadastrar")) { return false; }

  const bool success = [&] {
    if (tipo == Tipo::GerarCompra) {
      QSqlQuery queryUpdate;
      queryUpdate.prepare(
          "UPDATE pedido_fornecedor_has_produto2 SET aliquotaSt = :aliquotaSt, st = :st, quant = :quant, caixas = :caixas, prcUnitario = :prcUnitario, preco = :preco WHERE idPedidoFK = :idPedido1");

      for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
        queryUpdate.bindValue(":aliquotaSt", modelPedidoFornecedor.data(row, "aliquotaSt"));
        queryUpdate.bindValue(":st", modelPedidoFornecedor.data(row, "st"));
        queryUpdate.bindValue(":quant", modelPedidoFornecedor.data(row, "quant"));
        queryUpdate.bindValue(":caixas", modelPedidoFornecedor.data(row, "caixas"));
        queryUpdate.bindValue(":prcUnitario", modelPedidoFornecedor.data(row, "prcUnitario"));
        queryUpdate.bindValue(":preco", modelPedidoFornecedor.data(row, "preco"));
        queryUpdate.bindValue(":idPedido1", modelPedidoFornecedor.data(row, "idPedido1"));

        if (not queryUpdate.exec()) { return qApp->enqueueException(false, "Erro copiando dados para tabela 2: " + queryUpdate.lastError().text(), this); }
      }
    }

    if (tipo == Tipo::Faturamento) {
      for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
        if (modelPedidoFornecedor.data(row, "fornecedor").toString() == "ATELIER STACCATO") {
          if (not modelPedidoFornecedor.setData(row, "status", "ENTREGUE")) { return false; }
        }
      }
    }

    return modelPedidoFornecedor.submitAll();
  }();

  if (success) {
    if (not qApp->endTransaction()) { return false; }
  } else {
    qApp->rollbackTransaction();
  }

  return true;
}

void InputDialogProduto::on_dateEditEvento_dateChanged(const QDate &date) {
  if (ui->dateEditProximo->date() < date) { ui->dateEditProximo->setDate(date); }
}

void InputDialogProduto::on_doubleSpinBoxAliquota_valueChanged(double aliquota) {
  unsetConnections();

  [&] {
    double total = 0;

    for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) { total += modelPedidoFornecedor.data(row, "preco").toDouble(); }

    const double valueSt = total * aliquota / 100;

    ui->doubleSpinBoxST->setValue(valueSt);

    for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
      if (not modelPedidoFornecedor.setData(row, "aliquotaSt", aliquota)) { return; }
    }

    ui->doubleSpinBoxTotal->setValue(total + valueSt);
  }();

  setConnections();
}

void InputDialogProduto::on_doubleSpinBoxST_valueChanged(double valueSt) {
  unsetConnections();

  [&] {
    double total = 0;

    for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) { total += modelPedidoFornecedor.data(row, "preco").toDouble(); }

    const double aliquota = valueSt * 100 / total;

    ui->doubleSpinBoxAliquota->setValue(aliquota);

    for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
      if (not modelPedidoFornecedor.setData(row, "aliquotaSt", aliquota)) { return; }
    }

    ui->doubleSpinBoxTotal->setValue(total + valueSt);
  }();

  setConnections();
}

void InputDialogProduto::on_comboBoxST_currentTextChanged(const QString &text) {
  if (text == "Sem ST") {
    ui->doubleSpinBoxST->setValue(0);

    ui->labelAliquota->hide();
    ui->doubleSpinBoxAliquota->hide();
    ui->labelST->hide();
    ui->doubleSpinBoxST->hide();
  }

  if (text == "ST Fornecedor" or text == "ST Loja") {
    ui->labelAliquota->show();
    ui->doubleSpinBoxAliquota->show();
    ui->labelST->show();
    ui->doubleSpinBoxST->show();

    ui->doubleSpinBoxAliquota->setValue(4.68);
  }

  for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
    if (not modelPedidoFornecedor.setData(row, "st", text)) { return; }
  }
}

void InputDialogProduto::on_lineEditCodRep_textEdited(const QString &text) {
  for (int row = 0; row < modelPedidoFornecedor.rowCount(); ++row) {
    if (not modelPedidoFornecedor.setData(row, "ordemRepresentacao", text)) { return; }
  }
}
