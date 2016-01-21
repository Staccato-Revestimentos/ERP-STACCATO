#include <QSqlError>

#include "doubledelegate.h"
#include "entregascliente.h"
#include "ui_widgetlogisticaentrega.h"
#include "usersession.h"
#include "widgetlogisticaentrega.h"

WidgetLogisticaEntrega::WidgetLogisticaEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaEntrega) {
  ui->setupUi(this);

  if (UserSession::tipoUsuario() == "VENDEDOR") {
    ui->table->hide();
    ui->labelEntregasCliente->hide();
  }
}

WidgetLogisticaEntrega::~WidgetLogisticaEntrega() { delete ui; }

void WidgetLogisticaEntrega::setupTables() {
  model.setTable("view_venda");

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

QString WidgetLogisticaEntrega::updateTables() {
  if (model.tableName().isEmpty()) setupTables();

  if (not model.select()) return "Erro lendo tabela vendas: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaLimpar_clicked() {
  model.setFilter("tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaEnviado_clicked() {
  model.setFilter("status = 'ENVIADO' AND tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_radioButtonEntregaPendente_clicked() {
  model.setFilter("status = 'PENDENTE' AND tipo = 'cliente'");
  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_lineEditBuscaEntregas_textChanged(const QString &text) {
  model.setFilter(text.isEmpty() ? "" : "(idPedido LIKE '%" + text + "%') OR (status LIKE '%" + text + "%')");

  ui->table->resizeColumnsToContents();
}

void WidgetLogisticaEntrega::on_table_activated(const QModelIndex &index) {
  EntregasCliente *entregas = new EntregasCliente(this);
  entregas->viewEntrega(model.data(index.row(), "Código").toString());
}
