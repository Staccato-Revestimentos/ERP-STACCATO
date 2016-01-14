#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "checkboxdelegate.h"
#include "comboboxdelegate.h"
#include "inputdialog.h"
#include "ui_widgetlogisticacoleta.h"
#include "widgetlogisticacoleta.h"

WidgetLogisticaColeta::WidgetLogisticaColeta(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaColeta) {
  ui->setupUi(this);

  setupTables();
}

WidgetLogisticaColeta::~WidgetLogisticaColeta() { delete ui; }

QString WidgetLogisticaColeta::updateTables() {
  model.setFilter("0");

  if (not model.select()) {
    return "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text();
  }

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaColeta::TableFornLogistica_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM COLETA'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  for (int row = 0; row < model.rowCount(); ++row) {
    ui->table->openPersistentEditor(row, "selecionado");
  }

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaColeta::setupTables() {
  model.setTable("pedido_fornecedor_has_produto");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  model.setHeaderData("selecionado", "");
  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("descricao", "Descrição");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("quant", "Quant.");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("preco", "Preço");
  model.setHeaderData("formComercial", "Form. Com.");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("codBarras", "Cód. Bar.");
  model.setHeaderData("idCompra", "Compra");
  model.setHeaderData("dataRealFat", "Data Fat.");
  model.setHeaderData("dataPrevColeta", "Prev. Coleta");
  model.setHeaderData("status", "Status");

  model.setFilter("status = 'EM COLETA'");

  ui->table->setModel(&model);
  ui->table->setItemDelegateForColumn("status", new ComboBoxDelegate(this));
  ui->table->setItemDelegateForColumn("selecionado", new CheckBoxDelegate(this));
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("idLoja");
  ui->table->hideColumn("item");
  ui->table->hideColumn("idProduto");
  ui->table->hideColumn("prcUnitario");
  ui->table->hideColumn("parcial");
  ui->table->hideColumn("desconto");
  ui->table->hideColumn("parcialDesc");
  ui->table->hideColumn("descGlobal");
  ui->table->hideColumn("dataPrevCompra");
  ui->table->hideColumn("dataRealCompra");
  ui->table->hideColumn("dataPrevConf");
  ui->table->hideColumn("dataRealConf");
  ui->table->hideColumn("dataPrevFat");
  ui->table->hideColumn("dataRealColeta");
  ui->table->hideColumn("dataPrevEnt");
  ui->table->hideColumn("dataRealEnt");
  ui->table->hideColumn("dataPrevReceb");
  ui->table->hideColumn("dataRealReceb");
  ui->table->hideColumn("quantUpd");
}

void WidgetLogisticaColeta::on_pushButtonMarcarColetado_clicked() {
  QList<int> lista;

  for (const auto index : model.match(model.index(0, model.fieldIndex("selecionado")), Qt::DisplayRole, true, -1,
                                      Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap))) {
    lista.append(index.row());
  }

  if (lista.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhum item selecionado!");
    return;
  }

  QString filtro = model.filter();

  InputDialog *inputDlg = new InputDialog(InputDialog::Coleta, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QDate dataColeta = inputDlg->getDate();
  QDate dataPrevista = inputDlg->getNextDate();

  model.setFilter(filtro);
  model.select();

  for (const auto row : lista) {
    if (model.data(row, "status").toString() != "EM COLETA") {
      model.select();
      QMessageBox::critical(this, "Erro!", "Produto não estava em coleta!");
      return;
    }

    if (not model.setData(row, "status", "EM RECEBIMENTO")) {
      QMessageBox::critical(this, "Erro!", "Erro marcando status EM RECEBIMENTO: " + model.lastError().text());
      return;
    }

    // salvar status na venda
    QSqlQuery query;

    query.prepare("UPDATE venda_has_produto SET dataRealColeta = :dataRealColeta, dataPrevReceb = :dataPrevReceb, "
                  "status = 'EM RECEBIMENTO' WHERE idCompra = :idCompra");
    query.bindValue(":dataRealColeta", dataColeta);
    query.bindValue(":dataPrevReceb", dataPrevista);
    query.bindValue(":idCompra", model.data(row, "idCompra"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da venda: " + query.lastError().text());
      return;
    }

    if (not query.exec("CALL update_venda_status()")) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
      return;
    }
    //

    if (not model.setData(row, "dataRealColeta", dataColeta.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data da coleta: " + model.lastError().text());
      return;
    }

    if (not model.setData(row, "dataPrevReceb", dataPrevista.toString("yyyy-MM-dd"))) {
      QMessageBox::critical(this, "Erro!", "Erro guardando data prevista: " + model.lastError().text());
      return;
    }
  }

  if (not model.submitAll()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro salvando dados da tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado coleta.");
}

void WidgetLogisticaColeta::on_checkBoxMarcarTodos_clicked(const bool &checked) {
  for (int row = 0; row < model.rowCount(); ++row) {
    model.setData(row, "selecionado", checked);
  }
}
