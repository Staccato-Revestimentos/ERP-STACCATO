#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "importarxml.h"
#include "inputdialog.h"
#include "ui_widgetcomprafaturar.h"
#include "widgetcomprafaturar.h"

WidgetCompraFaturar::WidgetCompraFaturar(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraFaturar) {
  ui->setupUi(this);

  setupTables();
}

WidgetCompraFaturar::~WidgetCompraFaturar() { delete ui; }

void WidgetCompraFaturar::tableFornCompras_activated(const QString &fornecedor) {
  model.setFilter("fornecedor = '" + fornecedor + "' AND status = 'EM FATURAMENTO'");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + model.lastError().text());
    return;
  }
}

void WidgetCompraFaturar::setupTables() {
  model.setTable("view_faturamento");

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("COUNT(idProduto)", "Itens");
  model.setHeaderData("SUM(preco)", "Preço");
  model.setHeaderData("status", "Status");

  ui->table->setModel(&model);
}

QString WidgetCompraFaturar::updateTables() {
  if (not model.select()) {
    return "Erro lendo tabela faturamento: " + model.lastError().text();
  }

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetCompraFaturar::on_pushButtonMarcarFaturado_clicked() {
  if (ui->table->selectionModel()->selectedRows().size() == 0) {
    QMessageBox::critical(this, "Erro!", "Não selecionou nenhuma compra!");
    return;
  }

  int row = ui->table->selectionModel()->selectedRows().first().row();

  ImportarXML *import = new ImportarXML(this);
  import->filtrar(model.data(row, "Fornecedor").toString());
  import->showMaximized();

  if (import->exec() != QDialog::Accepted) return;

  QString const idCompra = import->getIdCompra();
  //----------------------------------------------------------//

  InputDialog *inputDlg = new InputDialog(InputDialog::Faturamento, this);
  //  inputDlg->setFilter(idCompra);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  const QDate dataFat = inputDlg->getDate();
  const QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;

  // TODO: juntar os updates em uma unica query

  if (not query.exec("UPDATE pedido_fornecedor_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
    return;
  }

  // salvar status na venda
  if (not query.exec("UPDATE venda_has_produto SET dataRealFat = '" + dataFat.toString("yyyy-MM-dd") +
                     "', dataPrevColeta = '" + dataPrevista.toString("yyyy-MM-dd") +
                     "', status = 'EM COLETA' WHERE idCompra = " + idCompra + "")) {
    QMessageBox::critical(this, "Erro!", "Erro salvando status da venda: " + query.lastError().text());
    return;
  }

  if (not query.exec("CALL update_venda_status()")) {
    QMessageBox::critical(this, "Erro!", "Erro atualizando status das vendas: " + query.lastError().text());
    return;
  }

  updateTables();

  QMessageBox::information(this, "Aviso!", "Confirmado faturamento.");
}
