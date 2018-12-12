#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "application.h"
#include "estoqueprazoproxymodel.h"
#include "inputdialogconfirmacao.h"
#include "ui_widgetlogisticarepresentacao.h"
#include "widgetlogisticarepresentacao.h"

WidgetLogisticaRepresentacao::WidgetLogisticaRepresentacao(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaRepresentacao) { ui->setupUi(this); }

WidgetLogisticaRepresentacao::~WidgetLogisticaRepresentacao() { delete ui; }

void WidgetLogisticaRepresentacao::setConnections() {
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged);
  connect(ui->pushButtonMarcarEntregue, &QPushButton::clicked, this, &WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked);
}

void WidgetLogisticaRepresentacao::updateTables() {
  if (not isSet) {
    setConnections();
    isSet = true;
  }

  if (not modelIsSet) {
    setupTables();
    modelIsSet = true;
  }

  if (not modelViewLogisticaRepresentacao.select()) { return; }

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaRepresentacao::tableFornLogistica_activated(const QString &fornecedor) {
  ui->lineEditBusca->clear();

  modelViewLogisticaRepresentacao.setFilter("fornecedor = '" + fornecedor + "'");

  if (not modelViewLogisticaRepresentacao.select()) { return; }

  ui->table->resizeColumnsToContents();

  ui->table->sortByColumn("prazoEntrega", Qt::AscendingOrder);
}

void WidgetLogisticaRepresentacao::resetTables() { modelIsSet = false; }

void WidgetLogisticaRepresentacao::setupTables() {
  modelViewLogisticaRepresentacao.setTable("view_logistica_representacao");
  modelViewLogisticaRepresentacao.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelViewLogisticaRepresentacao.setHeaderData("idVenda", "Venda");
  modelViewLogisticaRepresentacao.setHeaderData("cliente", "Cliente");
  modelViewLogisticaRepresentacao.setHeaderData("descricao", "Produto");
  modelViewLogisticaRepresentacao.setHeaderData("codComercial", "Cód. Com.");
  modelViewLogisticaRepresentacao.setHeaderData("quant", "Quant.");
  modelViewLogisticaRepresentacao.setHeaderData("un", "Un.");
  modelViewLogisticaRepresentacao.setHeaderData("caixas", "Cx.");
  modelViewLogisticaRepresentacao.setHeaderData("kgcx", "Kg./Cx.");
  modelViewLogisticaRepresentacao.setHeaderData("ordemCompra", "OC");
  modelViewLogisticaRepresentacao.setHeaderData("prazoEntrega", "Prazo Limite");

  modelViewLogisticaRepresentacao.setFilter("0");

  if (not modelViewLogisticaRepresentacao.select()) { return; }

  ui->table->setModel(new EstoquePrazoProxyModel(&modelViewLogisticaRepresentacao, this));
  ui->table->hideColumn("idPedido");
  ui->table->hideColumn("fornecedor");
  ui->table->hideColumn("idVendaProduto");
}

void WidgetLogisticaRepresentacao::on_pushButtonMarcarEntregue_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) { return qApp->enqueueError("Nenhum item selecionado!"); }

  InputDialogConfirmacao input(InputDialogConfirmacao::Tipo::Representacao);

  if (input.exec() != InputDialogConfirmacao::Accepted) { return; }

  if (not qApp->startTransaction()) { return; }

  if (not processRows(list, input.getDateTime(), input.getRecebeu())) { return qApp->rollbackTransaction(); }

  if (not qApp->endTransaction()) { return; }

  updateTables();
  qApp->enqueueInformation("Atualizado!");
}

bool WidgetLogisticaRepresentacao::processRows(const QModelIndexList &list, const QDateTime &dataEntrega, const QString &recebeu) {
  QSqlQuery query1;
  query1.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt WHERE idVendaProduto = :idVendaProduto AND status NOT IN ('CANCELADO', 'DEVOLVIDO')");

  QSqlQuery query2;
  query2.prepare(
      "UPDATE venda_has_produto SET status = 'ENTREGUE', dataRealEnt = :dataRealEnt, recebeu = :recebeu WHERE idVendaProduto = :idVendaProduto AND status NOT IN ('CANCELADO', 'DEVOLVIDO')");

  for (const auto &item : list) {
    query1.bindValue(":dataRealEnt", dataEntrega);
    query1.bindValue(":idVendaProduto", modelViewLogisticaRepresentacao.data(item.row(), "idVendaProduto"));

    if (not query1.exec()) { return qApp->enqueueError(false, "Erro salvando status no pedido_fornecedor: " + query1.lastError().text()); }

    query2.bindValue(":dataRealEnt", dataEntrega);
    query2.bindValue(":idVendaProduto", modelViewLogisticaRepresentacao.data(item.row(), "idVendaProduto"));
    query2.bindValue(":recebeu", recebeu);

    if (not query2.exec()) { return qApp->enqueueError(false, "Erro salvando status na venda_produto: " + query2.lastError().text()); }
  }

  return true;
}

void WidgetLogisticaRepresentacao::on_lineEditBusca_textChanged(const QString &text) {
  modelViewLogisticaRepresentacao.setFilter("(idVenda LIKE '%" + text + "%' OR cliente LIKE '%" + text + "%')");

  if (not modelViewLogisticaRepresentacao.select()) { return; }

  ui->table->resizeColumnsToContents();
}

// TODO: 2palimanan precisa de coleta/recebimento (colocar flag no cadastro dizendo que entra no fluxo de logistica)
