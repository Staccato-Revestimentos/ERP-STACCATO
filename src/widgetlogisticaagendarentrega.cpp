#include "widgetlogisticaagendarentrega.h"
#include "ui_widgetlogisticaagendarentrega.h"

#include "application.h"
#include "cadastrarnfe.h"
#include "doubledelegate.h"
#include "file.h"
#include "financeiroproxymodel.h"
#include "followup.h"
#include "inputdialog.h"
#include "sql.h"
#include "usersession.h"
#include "xml.h"

#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>

WidgetLogisticaAgendarEntrega::WidgetLogisticaAgendarEntrega(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaAgendarEntrega) {
  ui->setupUi(this);
  timer.setSingleShot(true);
}

WidgetLogisticaAgendarEntrega::~WidgetLogisticaAgendarEntrega() { delete ui; }

void WidgetLogisticaAgendarEntrega::setupTables() {
  modelVendas.setQuery(Sql::view_entrega_pendente());

  modelVendas.select();

  modelVendas.setHeaderData("prazoEntrega", "Prazo Limite");
  modelVendas.setHeaderData("novoPrazoEntrega", "Novo Prazo");
  modelVendas.setHeaderData("dataRealReceb", "Receb.");
  modelVendas.setHeaderData("statusFinanceiro", "Financeiro");
  modelVendas.setHeaderData("idVenda", "Venda");
  modelVendas.setHeaderData("kg", "Kg.");

  modelVendas.proxyModel = new FinanceiroProxyModel(&modelVendas, this);

  ui->tableVendas->setModel(&modelVendas);

  ui->tableVendas->setItemDelegate(new DoubleDelegate(this));

  ui->tableVendas->hideColumn("data");

  // -----------------------------------------------------------------

  modelProdutos.setQuery(Sql::view_agendar_entrega() + " LIMIT 0");

  modelProdutos.select();

  modelProdutos.setHeaderData("dataPrevEnt", "Prev. Ent.");
  modelProdutos.setHeaderData("status", "Status");
  modelProdutos.setHeaderData("fornecedor", "Fornecedor");
  modelProdutos.setHeaderData("idVenda", "Venda");
  modelProdutos.setHeaderData("idNFeSaida", "NFe");
  modelProdutos.setHeaderData("idNFeFutura", "NFe Futura");
  modelProdutos.setHeaderData("produto", "Produto");
  modelProdutos.setHeaderData("idEstoque", "Estoque");
  modelProdutos.setHeaderData("lote", "Lote");
  modelProdutos.setHeaderData("local", "Local");
  modelProdutos.setHeaderData("bloco", "Bloco");
  modelProdutos.setHeaderData("caixas", "Caixas");
  modelProdutos.setHeaderData("quant", "Quant.");
  modelProdutos.setHeaderData("un", "Un.");
  modelProdutos.setHeaderData("quantCaixa", "Quant./Cx.");
  modelProdutos.setHeaderData("codComercial", "Cód. Com.");
  modelProdutos.setHeaderData("formComercial", "Form. Com.");

  ui->tableProdutos->setModel(&modelProdutos);

  ui->tableProdutos->hideColumn("idVendaProduto2");
  ui->tableProdutos->hideColumn("idProduto");
  ui->tableProdutos->hideColumn("dataRealEnt");
  ui->tableProdutos->hideColumn("idConsumo");

  connect(ui->tableProdutos->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WidgetLogisticaAgendarEntrega::calcularPeso);

  // -----------------------------------------------------------------

  modelTranspAtual.setTable("veiculo_has_produto");

  modelTranspAtual.setHeaderData("idVenda", "Venda");
  modelTranspAtual.setHeaderData("status", "Status");
  modelTranspAtual.setHeaderData("fornecedor", "Fornecedor");
  modelTranspAtual.setHeaderData("produto", "Produto");
  modelTranspAtual.setHeaderData("caixas", "Cx.");
  modelTranspAtual.setHeaderData("kg", "Kg.");
  modelTranspAtual.setHeaderData("quant", "Quant.");
  modelTranspAtual.setHeaderData("un", "Un.");
  modelTranspAtual.setHeaderData("quantCaixa", "Quant./Cx.");
  modelTranspAtual.setHeaderData("codComercial", "Cód. Com.");
  modelTranspAtual.setHeaderData("formComercial", "Form. Com.");

  ui->tableTranspAtual->setModel(&modelTranspAtual);

  ui->tableTranspAtual->hideColumn("fotoEntrega");
  ui->tableTranspAtual->hideColumn("idVendaProduto1");
  ui->tableTranspAtual->hideColumn("idVendaProduto2");
  ui->tableTranspAtual->hideColumn("id");
  ui->tableTranspAtual->hideColumn("idEstoque");
  ui->tableTranspAtual->hideColumn("idEvento");
  ui->tableTranspAtual->hideColumn("idVeiculo");
  ui->tableTranspAtual->hideColumn("idCompra");
  ui->tableTranspAtual->hideColumn("idNFeSaida");
  ui->tableTranspAtual->hideColumn("idLoja");
  ui->tableTranspAtual->hideColumn("idProduto");
  ui->tableTranspAtual->hideColumn("obs");
  ui->tableTranspAtual->hideColumn("data");

  // -----------------------------------------------------------------

  modelTranspAgend.setTable("veiculo_has_produto");

  modelTranspAgend.setHeaderData("data", "Agendado");
  modelTranspAgend.setHeaderData("idEstoque", "Estoque");
  modelTranspAgend.setHeaderData("idVenda", "Venda");
  modelTranspAgend.setHeaderData("status", "Status");
  modelTranspAgend.setHeaderData("fornecedor", "Fornecedor");
  modelTranspAgend.setHeaderData("produto", "Produto");
  modelTranspAgend.setHeaderData("caixas", "Cx.");
  modelTranspAgend.setHeaderData("kg", "Kg.");
  modelTranspAgend.setHeaderData("quant", "Quant.");
  modelTranspAgend.setHeaderData("un", "Un.");
  modelTranspAgend.setHeaderData("quantCaixa", "Quant./Cx.");
  modelTranspAgend.setHeaderData("codComercial", "Cód. Com.");
  modelTranspAgend.setHeaderData("formComercial", "Form. Com.");

  ui->tableTranspAgend->setModel(&modelTranspAgend);

  ui->tableTranspAgend->hideColumn("fotoEntrega");
  ui->tableTranspAgend->hideColumn("idVendaProduto1");
  ui->tableTranspAgend->hideColumn("idVendaProduto2");
  ui->tableTranspAgend->hideColumn("id");
  ui->tableTranspAgend->hideColumn("idEvento");
  ui->tableTranspAgend->hideColumn("idVeiculo");
  ui->tableTranspAgend->hideColumn("idCompra");
  ui->tableTranspAgend->hideColumn("idNFeSaida");
  ui->tableTranspAgend->hideColumn("idLoja");
  ui->tableTranspAgend->hideColumn("idProduto");
  ui->tableTranspAgend->hideColumn("obs");
}

void WidgetLogisticaAgendarEntrega::calcularPeso() {
  double peso = 0;

  SqlQuery query;
  query.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");

  const auto selectedRows = ui->tableProdutos->selectionModel()->selectedRows();

  for (const auto &index : selectedRows) {
    query.bindValue(":idProduto", modelProdutos.data(index.row(), "idProduto"));

    if (not query.exec() or not query.first()) { throw RuntimeException("Erro buscando peso do produto: " + query.lastError().text(), this); }

    const double kg = query.value("kgcx").toDouble();
    const double caixas = modelProdutos.data(index.row(), "caixas").toDouble();
    peso += kg * caixas;
  }

  ui->doubleSpinBoxPeso->setValue(peso);

  ui->doubleSpinBoxPeso->setStyleSheet((ui->doubleSpinBoxPeso->value() > ui->doubleSpinBoxDisponivel->value()) ? "color: rgb(255, 0, 0);" : "");
}

void WidgetLogisticaAgendarEntrega::delayFiltro() { timer.start(500); }

void WidgetLogisticaAgendarEntrega::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(&timer, &QTimer::timeout, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxCancelado, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxDevolvido, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEmColeta, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEmCompra, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEmEntrega, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEmFaturamento, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEmRecebimento, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEntregaAgend, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEntregue, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxEstoque, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxIniciado, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxPendente, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxRepoEntrega, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->checkBoxRepoReceb, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->dateTimeEdit, &QDateTimeEdit::dateChanged, this, &WidgetLogisticaAgendarEntrega::on_dateTimeEdit_dateChanged, connectionType);
  connect(ui->groupBoxStatus, &QGroupBox::toggled, this, &WidgetLogisticaAgendarEntrega::on_groupBoxStatus_toggled, connectionType);
  connect(ui->itemBoxVeiculo, &ItemBox::textChanged, this, &WidgetLogisticaAgendarEntrega::on_itemBoxVeiculo_textChanged, connectionType);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaAgendarEntrega::delayFiltro, connectionType);
  connect(ui->pushButtonAdicionarParcial, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarParcial_clicked, connectionType);
  connect(ui->pushButtonAdicionarProduto, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarProduto_clicked, connectionType);
  connect(ui->pushButtonAgendarCarga, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAgendarCarga_clicked, connectionType);
  connect(ui->pushButtonGerarNFeFutura, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonGerarNFeFutura_clicked, connectionType);
  connect(ui->pushButtonImportarNFe, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonImportarNFe_clicked, connectionType);
  connect(ui->pushButtonReagendarPedido, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonReagendarPedido_clicked, connectionType);
  connect(ui->pushButtonRemoverProduto, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonRemoverProduto_clicked, connectionType);
  connect(ui->radioButtonEntregaLimpar, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->radioButtonParcialEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->radioButtonSemEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->radioButtonTotalEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro, connectionType);
  connect(ui->tableVendas, &TableView::clicked, this, &WidgetLogisticaAgendarEntrega::on_tableVendas_clicked, connectionType);
  connect(ui->tableVendas, &TableView::doubleClicked, this, &WidgetLogisticaAgendarEntrega::on_tableVendas_doubleClicked, connectionType);
}

void WidgetLogisticaAgendarEntrega::unsetConnections() {
  disconnect(&timer, &QTimer::timeout, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxCancelado, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxDevolvido, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEmColeta, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEmCompra, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEmEntrega, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEmFaturamento, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEmRecebimento, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEntregaAgend, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEntregue, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxEstoque, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxIniciado, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxPendente, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxRepoEntrega, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->checkBoxRepoReceb, &QCheckBox::toggled, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->dateTimeEdit, &QDateTimeEdit::dateChanged, this, &WidgetLogisticaAgendarEntrega::on_dateTimeEdit_dateChanged);
  disconnect(ui->groupBoxStatus, &QGroupBox::toggled, this, &WidgetLogisticaAgendarEntrega::on_groupBoxStatus_toggled);
  disconnect(ui->itemBoxVeiculo, &ItemBox::textChanged, this, &WidgetLogisticaAgendarEntrega::on_itemBoxVeiculo_textChanged);
  disconnect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->pushButtonAdicionarParcial, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarParcial_clicked);
  disconnect(ui->pushButtonAdicionarProduto, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarProduto_clicked);
  disconnect(ui->pushButtonAgendarCarga, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonAgendarCarga_clicked);
  disconnect(ui->pushButtonGerarNFeFutura, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonGerarNFeFutura_clicked);
  disconnect(ui->pushButtonImportarNFe, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonImportarNFe_clicked);
  disconnect(ui->pushButtonReagendarPedido, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonReagendarPedido_clicked);
  disconnect(ui->pushButtonRemoverProduto, &QPushButton::clicked, this, &WidgetLogisticaAgendarEntrega::on_pushButtonRemoverProduto_clicked);
  disconnect(ui->radioButtonEntregaLimpar, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->radioButtonParcialEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->radioButtonSemEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->radioButtonTotalEstoque, &QRadioButton::clicked, this, &WidgetLogisticaAgendarEntrega::montaFiltro);
  disconnect(ui->tableVendas, &TableView::clicked, this, &WidgetLogisticaAgendarEntrega::on_tableVendas_clicked);
  disconnect(ui->tableVendas, &TableView::doubleClicked, this, &WidgetLogisticaAgendarEntrega::on_tableVendas_doubleClicked);
}

void WidgetLogisticaAgendarEntrega::on_groupBoxStatus_toggled(const bool enabled) {
  unsetConnections();

  try {
    const auto children = ui->groupBoxStatus->findChildren<QCheckBox *>(QRegularExpression("checkBox"));

    for (const auto &child : children) {
      child->setEnabled(true);
      child->setChecked(enabled);
    }
  } catch (std::exception &) {}

  setConnections();

  montaFiltro();
}

void WidgetLogisticaAgendarEntrega::updateTables() {
  if (not isSet) {
    if (UserSession::tipoUsuario == "VENDEDOR") {
      ui->tableVendas->hide();
      ui->labelEntregasCliente->hide();
    }

    ui->itemBoxVeiculo->setSearchDialog(SearchDialog::veiculo(this));
    ui->dateTimeEdit->setDate(qApp->serverDate());

    setConnections();
    isSet = true;
  }

  if (not modelIsSet) {
    setupTables();
    montaFiltro();
    modelIsSet = true;
  }

  // -------------------------------------------------------------------------

  modelVendas.select();

  // -------------------------------------------------------------------------

  modelProdutos.select();

  // -------------------------------------------------------------------------

  modelTranspAgend.select();

  // -------------------------------------------------------------------------

  calcularDisponivel();
}

void WidgetLogisticaAgendarEntrega::resetTables() { modelIsSet = false; }

void WidgetLogisticaAgendarEntrega::on_tableVendas_clicked(const QModelIndex &index) {
  if (not index.isValid()) { return; }

  selectedIdVenda = modelVendas.data(index.row(), "idVenda").toString();

  filtroProdutos();

  ui->lineEditAviso->setText((modelVendas.data(index.row(), "statusFinanceiro").toString() != "LIBERADO") ? "Financeiro não liberou!" : "");
}

void WidgetLogisticaAgendarEntrega::montaFiltro() {
  QString filtroCheck;

  if (ui->radioButtonEntregaLimpar->isChecked()) { filtroCheck = "HAVING (Estoque > 0 OR Outros > 0)"; }
  if (ui->radioButtonTotalEstoque->isChecked()) { filtroCheck = "HAVING Estoque > 0 AND Outros = 0"; }
  if (ui->radioButtonParcialEstoque->isChecked()) { filtroCheck = "HAVING Estoque > 0 AND Outros > 0"; }
  if (ui->radioButtonSemEstoque->isChecked()) { filtroCheck = "HAVING Estoque = 0 AND Outros > 0"; }

  //-------------------------------------

  const QString textoBusca = qApp->sanitizeSQL(ui->lineEditBusca->text());

  const QString filtroBusca = textoBusca.isEmpty() ? ""
                                                   : " AND (vp2.idVenda LIKE '%" + textoBusca + "%' OR che.bairro LIKE '%" + textoBusca + "%' OR che.logradouro LIKE '%" + textoBusca +
                                                         "%' OR che.cidade LIKE '%" + textoBusca + "%')";

  //-------------------------------------

  QStringList status;

  for (const auto &child : ui->groupBoxStatus->findChildren<QCheckBox *>(QRegularExpression("checkBox"))) {
    if (child->isChecked()) { status << "'" + child->text().toUpper() + "'"; }
  }

  const QString filtroStatus = status.isEmpty() ? "" : " AND vp2.status IN (" + status.join(", ") + ")";

  //-------------------------------------

  modelVendas.setQuery(Sql::view_entrega_pendente(filtroBusca, filtroCheck, filtroStatus));

  modelVendas.select();

  //-------------------------------------

  filtroProdutos();
}

void WidgetLogisticaAgendarEntrega::filtroProdutos() {
  if (selectedIdVenda.isEmpty()) { return; }

  QStringList filtros;
  QStringList filtroCheck;

  const auto children = ui->groupBoxStatus->findChildren<QCheckBox *>(QRegularExpression("checkBox"));

  for (const auto &child : children) {
    if (child->isChecked()) { filtroCheck << "'" + child->text().toUpper() + "'"; }
  }

  if (not filtroCheck.isEmpty()) { filtros << "vp2.status IN (" + filtroCheck.join(", ") + ")"; }

  modelProdutos.setQuery(Sql::view_agendar_entrega(selectedIdVenda, filtros.join(" AND ")));

  modelProdutos.select();
}

void WidgetLogisticaAgendarEntrega::on_pushButtonAgendarCarga_clicked() {
  // TODO: não deixar agendar carga com mistura de produtos com e sem nfe futura

  if (modelTranspAtual.rowCount() == 0) { throw RuntimeError("Carga vazia!", this); }

  // -------------------------------------------------------------------------

  QStringList idVendas;

  for (int row = 0; row < modelTranspAtual.rowCount(); ++row) { idVendas << modelTranspAtual.data(row, "idVenda").toString(); }

  // -------------------------------------------------------------------------

  qApp->startTransaction("WidgetLogisticaAgendarEntrega::on_pushButtonAgendarCarga");

  processRows();

  Sql::updateVendaStatus(idVendas);

  qApp->endTransaction();

  // -------------------------------------------------------------------------

  updateTables();

  qApp->enqueueInformation("Confirmado agendamento!", this);
}

void WidgetLogisticaAgendarEntrega::processRows() {
  const QDateTime dataPrevEnt = ui->dateTimeEdit->dateTime();

  SqlQuery queryEvento;

  if (not queryEvento.exec("SELECT COALESCE(MAX(idEvento), 0) + 1 FROM veiculo_has_produto") or not queryEvento.first()) {
    throw RuntimeException("Erro comunicando com o banco de dados: " + queryEvento.lastError().text());
  }

  const int idEvento = queryEvento.value(0).toInt();

  SqlQuery querySelecao;
  querySelecao.prepare("SELECT idVenda, codComercial FROM venda_has_produto2 WHERE idVendaProduto2 = :idVendaProduto2");

  SqlQuery queryCompra;
  queryCompra.prepare("UPDATE pedido_fornecedor_has_produto2 SET status = 'ENTREGA AGEND.', dataPrevEnt = :dataPrevEnt WHERE status = 'ESTOQUE' AND idVendaProduto2 = :idVendaProduto2");

  SqlQuery queryVenda;
  queryVenda.prepare(
      "UPDATE venda_has_produto2 SET status = 'ENTREGA AGEND.', dataPrevEnt = :dataPrevEnt WHERE status IN ('PENDENTE', 'REPO. ENTREGA', 'ESTOQUE') AND idVendaProduto2 = :idVendaProduto2");

  for (int row = 0; row < modelTranspAtual.rowCount(); ++row) {
    modelTranspAtual.setData(row, "data", dataPrevEnt);
    modelTranspAtual.setData(row, "idEvento", idEvento);

    const int idVendaProduto2 = modelTranspAtual.data(row, "idVendaProduto2").toInt();

    querySelecao.bindValue(":idVendaProduto2", idVendaProduto2);

    if (not querySelecao.exec() or not querySelecao.first()) { throw RuntimeException("Erro buscando dados do produto: " + querySelecao.lastError().text()); }

    queryCompra.bindValue(":dataPrevEnt", dataPrevEnt);
    queryCompra.bindValue(":idVendaProduto2", idVendaProduto2);

    if (not queryCompra.exec()) { throw RuntimeException("Erro atualizando status da compra: " + queryCompra.lastError().text()); }

    queryVenda.bindValue(":dataPrevEnt", dataPrevEnt);
    queryVenda.bindValue(":idVendaProduto2", idVendaProduto2);

    if (not queryVenda.exec()) { throw RuntimeException("Erro atualizando produtos venda: " + queryVenda.lastError().text()); }
  }

  modelTranspAtual.submitAll();
}

void WidgetLogisticaAgendarEntrega::adicionarProduto(const QModelIndexList &list) {
  for (const auto &index : list) { adicionaProdutoNoModel(index.row(), modelProdutos.data(index.row(), "caixas").toDouble()); }
}

void WidgetLogisticaAgendarEntrega::adicionaProdutoNoModel(const int row, const double caixas) {
  SqlQuery query;
  query.prepare("SELECT kgcx FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", modelProdutos.data(row, "idProduto"));

  if (not query.exec() or not query.first()) { throw RuntimeException("Erro buscando peso do produto: " + query.lastError().text()); }

  const double quantCaixa = modelProdutos.data(row, "quantCaixa").toDouble();
  const double kg = query.value("kgcx").toDouble();

  const int newRow = modelTranspAtual.insertRowAtEnd();

  modelTranspAtual.setData(newRow, "fornecedor", modelProdutos.data(row, "fornecedor"));
  modelTranspAtual.setData(newRow, "quantCaixa", modelProdutos.data(row, "quantCaixa"));
  modelTranspAtual.setData(newRow, "formComercial", modelProdutos.data(row, "formComercial"));
  modelTranspAtual.setData(newRow, "idVeiculo", ui->itemBoxVeiculo->getId());
  modelTranspAtual.setData(newRow, "idVenda", modelProdutos.data(row, "idVenda"));
  modelTranspAtual.setData(newRow, "idVendaProduto2", modelProdutos.data(row, "idVendaProduto2"));
  modelTranspAtual.setData(newRow, "idProduto", modelProdutos.data(row, "idProduto"));
  modelTranspAtual.setData(newRow, "produto", modelProdutos.data(row, "produto"));
  modelTranspAtual.setData(newRow, "codComercial", modelProdutos.data(row, "codComercial"));
  modelTranspAtual.setData(newRow, "un", modelProdutos.data(row, "un"));
  modelTranspAtual.setData(newRow, "caixas", caixas);
  modelTranspAtual.setData(newRow, "kg", kg * caixas);
  modelTranspAtual.setData(newRow, "quant", caixas * quantCaixa);
  modelTranspAtual.setData(newRow, "status", "ENTREGA AGEND.");
}

void WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarProduto_clicked() {
  if (ui->itemBoxVeiculo->getId().isNull()) { throw RuntimeError("Deve escolher uma transportadora antes!", this); }

  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.isEmpty()) { throw RuntimeError("Nenhum item selecionado!", this); }

  if (ui->doubleSpinBoxPeso->value() > ui->doubleSpinBoxDisponivel->value()) { qApp->enqueueWarning("Peso maior que capacidade do veículo!", this); }

  bool semEstoque = false;

  for (const auto &index : list) {
    const int row = index.row();

    const auto listMatch = modelTranspAtual.match("idVendaProduto2", modelProdutos.data(row, "idVendaProduto2"), 1, Qt::MatchExactly);

    if (not listMatch.isEmpty()) { throw RuntimeError("Item '" + modelProdutos.data(row, "produto").toString() + "' já inserido!", this); }

    if (not modelProdutos.data(row, "dataPrevEnt").isNull()) { throw RuntimeError("Produto já agendado!", this); }

    const QString status = modelProdutos.data(row, "status").toString();

    if (status != "PENDENTE" and status != "REPO. ENTREGA" and status != "ESTOQUE") { throw RuntimeError("Produto não está PENDENTE/ESTOQUE/REPO. ENTREGA!"); }

    if (status != "ESTOQUE") { semEstoque = true; }
  }

  // -------------------------------------------------------------------------

  if (semEstoque) {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Um ou mais produtos não estão em estoque! Tem certeza que deseja agendar?", QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Agendar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) { return; }
  }

  // -------------------------------------------------------------------------

  adicionarProduto(list);

  ui->tableProdutos->clearSelection();
}

void WidgetLogisticaAgendarEntrega::on_pushButtonRemoverProduto_clicked() {
  const auto selection = ui->tableTranspAtual->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhum item selecionado!", this); }

  modelTranspAtual.removeSelection(selection);
}

void WidgetLogisticaAgendarEntrega::on_itemBoxVeiculo_textChanged(const QString &) {
  SqlQuery query;
  query.prepare("SELECT capacidade FROM transportadora_has_veiculo WHERE idVeiculo = :idVeiculo");
  query.bindValue(":idVeiculo", ui->itemBoxVeiculo->getId());

  if (not query.exec() or not query.first()) { throw RuntimeException("Erro buscando dados veiculo: " + query.lastError().text(), this); }

  ui->doubleSpinBoxCapacidade->setValue(query.value("capacidade").toDouble());

  // ------------------------------------------

  modelTranspAtual.select();

  modelTranspAgend.setFilter("idVeiculo = " + ui->itemBoxVeiculo->getId().toString() + " AND status != 'FINALIZADO' AND DATE(data) = '" + ui->dateTimeEdit->date().toString("yyyy-MM-dd") + "'");

  // ------------------------------------------

  calcularDisponivel();
}

void WidgetLogisticaAgendarEntrega::calcularDisponivel() {
  double peso = 0;

  for (int row = 0; row < modelTranspAgend.rowCount(); ++row) {
    const int hour = modelTranspAgend.data(row, "data").toDateTime().time().hour();

    if (hour != ui->dateTimeEdit->time().hour()) { continue; }

    peso += modelTranspAgend.data(row, "kg").toDouble();
  }

  const double capacidade = ui->doubleSpinBoxCapacidade->value();

  ui->doubleSpinBoxDisponivel->setValue(capacidade - peso);
}

void WidgetLogisticaAgendarEntrega::on_dateTimeEdit_dateChanged(const QDate &date) {
  if (ui->itemBoxVeiculo->text().isEmpty()) { return; }

  modelTranspAgend.setFilter("idVeiculo = " + ui->itemBoxVeiculo->getId().toString() + " AND status != 'FINALIZADO' AND DATE(data) = '" + date.toString("yyyy-MM-dd") + "'");

  calcularDisponivel();
}

void WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarParcial_clicked() {
  if (ui->itemBoxVeiculo->getId().isNull()) { throw RuntimeError("Deve escolher uma transportadora antes!", this); }

  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.isEmpty()) { throw RuntimeError("Nenhum item selecionado!", this); }

  if (list.size() > 1) { throw RuntimeError("Deve selecionar apenas um item para agendamento parcial!", this); }

  const int row = list.first().row();

  const auto list2 = modelTranspAtual.match("idVendaProduto2", modelProdutos.data(row, "idVendaProduto2"), 1, Qt::MatchExactly);

  if (not list2.isEmpty()) { throw RuntimeError("Item '" + modelProdutos.data(row, "produto").toString() + "' já inserido!", this); }

  // TODO: calcular o peso parcial e não o total para comparar
  if (ui->doubleSpinBoxPeso->value() > ui->doubleSpinBoxDisponivel->value()) { qApp->enqueueWarning("Peso maior que capacidade do veículo!", this); }

  if (modelProdutos.data(row, "caixas").toInt() == 1) { throw RuntimeError("Produto tem apenas uma caixa!", this); }

  if (not modelProdutos.data(row, "dataPrevEnt").isNull()) { throw RuntimeError("Produto já agendado!", this); }

  // -------------------------------------------------------------------------

  const QString status = modelProdutos.data(row, "status").toString();

  if (status != "PENDENTE" and status != "REPO. ENTREGA" and status != "ESTOQUE") { throw RuntimeError("Produto não está PENDENTE/ESTOQUE/REPO. ENTREGA!"); }

  if (status != "ESTOQUE") {
    QMessageBox msgBox(QMessageBox::Question, "Atenção!", "O produto não está em estoque! Tem certeza que deseja agendar?", QMessageBox::Yes | QMessageBox::No, this);
    msgBox.setButtonText(QMessageBox::Yes, "Agendar");
    msgBox.setButtonText(QMessageBox::No, "Voltar");

    if (msgBox.exec() == QMessageBox::No) { return; }
  }

  // -------------------------------------------------------------------------

  // perguntar quantidade
  const double caixasTotal = modelProdutos.data(row, "caixas").toDouble();

  bool ok;

  const double caixasAgendar = QInputDialog::getDouble(this, "Agendar", "Quantidade de caixas: ", caixasTotal, 0, caixasTotal, 1, &ok);

  if (qFuzzyIsNull(caixasAgendar) or not ok) { return; }

  const int novoIdVendaProduto2 = qApp->reservarIdVendaProduto2();

  // -------------------------------------------------------------------------

  qApp->startTransaction("WidgetLogisticaAgendarEntrega::on_pushButtonAdicionarParcial");

  adicionarProdutoParcial(row, caixasAgendar, caixasTotal, novoIdVendaProduto2);

  qApp->endTransaction();

  // -------------------------------------------------------------------------

  updateTables();
}

void WidgetLogisticaAgendarEntrega::adicionarProdutoParcial(const int row, const double caixasAgendar, const double caixasTotal, const int novoIdVendaProduto2) {
  if (caixasAgendar < caixasTotal) { dividirVenda(row, caixasAgendar, caixasTotal, novoIdVendaProduto2); }

  adicionaProdutoNoModel(row, caixasAgendar);
}

void WidgetLogisticaAgendarEntrega::dividirVenda(const int row, const double caixasAgendar, const double caixasTotal, const int novoIdVendaProduto2) {
  SqlTableModel modelProdutosTemp;
  modelProdutosTemp.setTable("venda_has_produto2");

  modelProdutosTemp.setFilter("idVendaProduto2 = " + modelProdutos.data(row, "idVendaProduto2").toString());

  modelProdutosTemp.select();

  if (modelProdutosTemp.rowCount() == 0) { return; }

  // -------------------------------------------------------------------------

  const double quantCaixa = modelProdutosTemp.data(0, "quantCaixa").toDouble();
  const double proporcao = caixasAgendar / caixasTotal;
  const double parcial = modelProdutosTemp.data(0, "parcial").toDouble();
  const double parcialDesc = modelProdutosTemp.data(0, "parcialDesc").toDouble();
  const double total = modelProdutosTemp.data(0, "total").toDouble();

  modelProdutosTemp.setData(0, "quant", (caixasAgendar * quantCaixa));
  modelProdutosTemp.setData(0, "caixas", caixasAgendar);
  modelProdutosTemp.setData(0, "parcial", parcial * proporcao);
  modelProdutosTemp.setData(0, "parcialDesc", parcialDesc * proporcao);
  modelProdutosTemp.setData(0, "total", total * proporcao);

  // -------------------------------------------------------------------------

  const int newRow = modelProdutosTemp.insertRowAtEnd();
  // NOTE: *quebralinha venda_produto2

  // copiar colunas
  for (int column = 0, columnCount = modelProdutosTemp.columnCount(); column < columnCount; ++column) {
    if (column == modelProdutosTemp.fieldIndex("idVendaProduto2")) { continue; }
    if (column == modelProdutosTemp.fieldIndex("created")) { continue; }
    if (column == modelProdutosTemp.fieldIndex("lastUpdated")) { continue; }

    const QVariant value = modelProdutosTemp.data(0, column);

    if (value.isNull()) { continue; }

    modelProdutosTemp.setData(newRow, column, value);
  }

  // alterar quant, precos, etc da linha nova
  const double proporcaoNovo = (caixasTotal - caixasAgendar) / caixasTotal;

  modelProdutosTemp.setData(newRow, "idVendaProduto2", novoIdVendaProduto2);
  modelProdutosTemp.setData(newRow, "idRelacionado", modelProdutosTemp.data(0, "idVendaProduto2"));
  modelProdutosTemp.setData(newRow, "quant", ((caixasTotal - caixasAgendar) * quantCaixa));
  modelProdutosTemp.setData(newRow, "caixas", (caixasTotal - caixasAgendar));
  modelProdutosTemp.setData(newRow, "parcial", parcial * proporcaoNovo);
  modelProdutosTemp.setData(newRow, "parcialDesc", parcialDesc * proporcaoNovo);
  modelProdutosTemp.setData(newRow, "total", total * proporcaoNovo);

  modelProdutosTemp.submitAll();

  // -------------------------------------------------------------------------

  // TODO: em vez de quebrar consumo, mandar para a funcao Estoque::criarConsumo?
  dividirConsumo(row, proporcao, proporcaoNovo, modelProdutosTemp.query().lastInsertId().toInt());

  // -------------------------------------------------------------------------

  dividirCompra(row, caixasAgendar, caixasTotal, novoIdVendaProduto2);
}

void WidgetLogisticaAgendarEntrega::dividirConsumo(const int row, const double proporcao, const double proporcaoNovo, const int idVendaProduto2) {
  SqlTableModel modelConsumoTemp;
  modelConsumoTemp.setTable("estoque_has_consumo");

  modelConsumoTemp.setFilter("idVendaProduto2 = " + modelProdutos.data(row, "idVendaProduto2").toString());

  modelConsumoTemp.select();

  if (modelConsumoTemp.rowCount() == 0) { return; }

  // -------------------------------------------------------------------------
  // NOTE: *quebralinha estoque_consumo

  const double quantConsumo = modelConsumoTemp.data(0, "quant").toDouble();
  const double caixasConsumo = modelConsumoTemp.data(0, "caixas").toDouble();
  const double valorConsumo = modelConsumoTemp.data(0, "valor").toDouble();

  // dados da NFe
  const double quantTrib = modelConsumoTemp.data(0, "quantTrib").toDouble();
  const double desconto = modelConsumoTemp.data(0, "desconto").toDouble();
  const double vBC = modelConsumoTemp.data(0, "vBC").toDouble();
  const double vICMS = modelConsumoTemp.data(0, "vICMS").toDouble();
  const double vBCST = modelConsumoTemp.data(0, "vBCST").toDouble();
  const double vICMSST = modelConsumoTemp.data(0, "vICMSST").toDouble();
  const double vBCPIS = modelConsumoTemp.data(0, "vBCPIS").toDouble();
  const double vPIS = modelConsumoTemp.data(0, "vPIS").toDouble();
  const double vBCCOFINS = modelConsumoTemp.data(0, "vBCCOFINS").toDouble();
  const double vCOFINS = modelConsumoTemp.data(0, "vCOFINS").toDouble();

  modelConsumoTemp.setData(0, "quant", quantConsumo * proporcao);
  modelConsumoTemp.setData(0, "caixas", caixasConsumo * proporcao);
  modelConsumoTemp.setData(0, "valor", valorConsumo * proporcao);

  // impostos
  modelConsumoTemp.setData(0, "quantTrib", quantTrib * proporcao);
  modelConsumoTemp.setData(0, "desconto", desconto * proporcao);
  modelConsumoTemp.setData(0, "vBC", vBC * proporcao);
  modelConsumoTemp.setData(0, "vICMS", vICMS * proporcao);
  modelConsumoTemp.setData(0, "vBCST", vBCST * proporcao);
  modelConsumoTemp.setData(0, "vICMSST", vICMSST * proporcao);
  modelConsumoTemp.setData(0, "vBCPIS", vBCPIS * proporcao);
  modelConsumoTemp.setData(0, "vPIS", vPIS * proporcao);
  modelConsumoTemp.setData(0, "vBCCOFINS", vBCCOFINS * proporcao);
  modelConsumoTemp.setData(0, "vCOFINS", vCOFINS * proporcao);

  // -------------------------------------------------------------------------
  // alterar linha nova
  const int newRow = modelConsumoTemp.insertRowAtEnd();

  for (int column = 0, columnCount = modelConsumoTemp.columnCount(); column < columnCount; ++column) {
    if (column == modelConsumoTemp.fieldIndex("idConsumo")) { continue; }
    if (column == modelConsumoTemp.fieldIndex("idVendaProduto2")) { continue; }
    if (column == modelConsumoTemp.fieldIndex("created")) { continue; }
    if (column == modelConsumoTemp.fieldIndex("lastUpdated")) { continue; }

    const QVariant value = modelConsumoTemp.data(0, column);

    if (value.isNull()) { continue; }

    modelConsumoTemp.setData(newRow, column, value);
  }

  modelConsumoTemp.setData(newRow, "idVendaProduto2", idVendaProduto2);
  modelConsumoTemp.setData(newRow, "quant", quantConsumo * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "caixas", caixasConsumo * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "valor", valorConsumo * proporcaoNovo);

  // impostos
  modelConsumoTemp.setData(newRow, "quantTrib", quantTrib * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "desconto", desconto * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vBC", vBC * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vICMS", vICMS * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vBCST", vBCST * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vICMSST", vICMSST * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vBCPIS", vBCPIS * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vPIS", vPIS * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vBCCOFINS", vBCCOFINS * proporcaoNovo);
  modelConsumoTemp.setData(newRow, "vCOFINS", vCOFINS * proporcaoNovo);

  modelConsumoTemp.submitAll();
}

void WidgetLogisticaAgendarEntrega::dividirCompra(const int row, const double caixasAgendar, const double caixasTotal, const int novoIdVendaProduto2) {
  SqlTableModel modelCompra;
  modelCompra.setTable("pedido_fornecedor_has_produto2");

  modelCompra.setFilter("idVendaProduto2 = " + modelProdutos.data(row, "idVendaProduto2").toString());

  modelCompra.select();

  if (modelCompra.rowCount() == 0) { return; }

  // -------------------------------------------------------------------------
  // NOTE: *quebralinha pedido_fornecedor2

  const double prcUnitario = modelCompra.data(0, "prcUnitario").toDouble();
  const double quantCaixa = modelProdutos.data(row, "quantCaixa").toDouble();
  const double quantAgendar = caixasAgendar * quantCaixa;

  modelCompra.setData(0, "quant", quantAgendar);
  modelCompra.setData(0, "caixas", caixasAgendar);
  modelCompra.setData(0, "preco", quantAgendar * prcUnitario);

  // -------------------------------------------------------------------------
  // copiar linha
  const int newRow = modelCompra.insertRowAtEnd();

  for (int column = 0, columnCount = modelCompra.columnCount(); column < columnCount; ++column) {
    if (column == modelCompra.fieldIndex("idPedido2")) { continue; }
    if (column == modelCompra.fieldIndex("created")) { continue; }
    if (column == modelCompra.fieldIndex("lastUpdated")) { continue; }

    const QVariant value = modelCompra.data(0, column);

    if (value.isNull()) { continue; }

    modelCompra.setData(newRow, column, value);
  }

  const double caixasRestante = caixasTotal - caixasAgendar;

  modelCompra.setData(newRow, "idRelacionado", modelCompra.data(0, "idPedido2"));
  modelCompra.setData(newRow, "idVendaProduto2", novoIdVendaProduto2);
  modelCompra.setData(newRow, "quant", caixasRestante * quantCaixa);
  modelCompra.setData(newRow, "caixas", caixasRestante);
  modelCompra.setData(newRow, "preco", caixasRestante * quantCaixa * prcUnitario);

  modelCompra.submitAll();
}

void WidgetLogisticaAgendarEntrega::on_pushButtonReagendarPedido_clicked() {
  // get idVenda from view_entrega_pendente/modelVendas and set 'novoPrazo'

  const auto list = ui->tableVendas->selectionModel()->selectedRows();

  if (list.isEmpty()) { throw RuntimeError("Nenhum item selecionado!", this); }

  InputDialog input(InputDialog::Tipo::ReagendarPedido, this);

  if (input.exec() != InputDialog::Accepted) { return; }

  qApp->startTransaction("WidgetLogisticaAgendarEntrega::on_pushButtonReagendarPedido");

  reagendar(list, input.getNextDate(), input.getObservacao());

  qApp->endTransaction();

  updateTables();
  qApp->enqueueInformation("Reagendado com sucesso!", this);
}

void WidgetLogisticaAgendarEntrega::reagendar(const QModelIndexList &list, const QDate &dataPrev, const QString &observacao) {
  SqlQuery query1;
  query1.prepare("UPDATE venda SET novoPrazoEntrega = :novoPrazoEntrega WHERE idVenda = :idVenda");

  SqlQuery query2;
  query2.prepare("INSERT INTO venda_has_followup (idVenda, idVendaBase, idLoja, idUsuario, tipoOperacao, observacao, dataFollowup) VALUES (:idVenda, :idVendaBase, :idLoja, :idUsuario, :tipoOperacao, "
                 ":observacao, :dataFollowup)");

  for (const auto &index : list) {
    const int row = index.row();

    query1.bindValue(":novoPrazoEntrega", modelVendas.data(row, "data").toDate().daysTo(dataPrev));
    query1.bindValue(":idVenda", modelVendas.data(row, "idVenda"));

    if (not query1.exec()) { throw RuntimeException("Erro atualizando novo prazo: " + query1.lastError().text()); }

    query2.bindValue(":idVenda", modelVendas.data(row, "idVenda"));
    query2.bindValue(":idVendaBase", modelVendas.data(row, "idVenda").toString().left(11));
    query2.bindValue(":idLoja", UserSession::idLoja);
    query2.bindValue(":idUsuario", UserSession::idUsuario);
    query2.bindValue(":tipoOperacao", "Alteração do prazo de entrega");
    query2.bindValue(":observacao", observacao);
    query2.bindValue(":dataFollowup", qApp->serverDate());

    if (not query2.exec()) { throw RuntimeException("Erro salvando followup: " + query2.lastError().text()); }
  }
}

void WidgetLogisticaAgendarEntrega::on_tableVendas_doubleClicked(const QModelIndex &index) {
  if (not index.isValid()) { return; }

  if (index.column() == modelVendas.record().indexOf("novoPrazoEntrega")) {
    const auto list = ui->tableVendas->selectionModel()->selectedRows();

    if (list.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!", this); }

    FollowUp *followup = new FollowUp(modelVendas.data(list.first().row(), "idVenda").toString(), FollowUp::Tipo::Venda, this);
    followup->setAttribute(Qt::WA_DeleteOnClose);
    followup->show();
  }
}

void WidgetLogisticaAgendarEntrega::on_pushButtonGerarNFeFutura_clicked() {
  // TODO: bloquear se já houver nfe emitida

  if (not ui->tableProdutos->model()) { return; }

  const auto list = ui->tableProdutos->selectionModel()->selectedRows();

  if (list.isEmpty()) { throw RuntimeError("Nenhum item selecionado!", this); }

  for (const auto &index : list) {
    if (not modelProdutos.data(index.row(), "idNFeFutura").isNull()) { throw RuntimeError("Produto já possui nota futura!", this); }
  }

  const QString idVenda = modelProdutos.data(list.first().row(), "idVenda").toString();

  if (idVenda.isEmpty()) { throw RuntimeException("Erro buscando 'Venda'!", this); }

  QStringList lista;

  for (const auto &index : list) { lista << modelProdutos.data(index.row(), "idVendaProduto2").toString(); }

  lista.removeDuplicates();

  auto *nfe = new CadastrarNFe(idVenda, lista, CadastrarNFe::Tipo::Futura, this);
  nfe->setAttribute(Qt::WA_DeleteOnClose);

  nfe->show();
}

void WidgetLogisticaAgendarEntrega::on_pushButtonImportarNFe_clicked() {
  const auto selection = ui->tableProdutos->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Não selecionou nenhuma linha!", this); }

  for (const auto &index : selection) {
    if (modelProdutos.data(index.row(), "idNFeSaida").toInt() > 0) { throw RuntimeError("Pelo menos uma linha selecionada já possui nota!", this); }
  }

  const QString filePath = QFileDialog::getOpenFileName(this, "Arquivo XML", "", "XML (*.xml)");

  if (filePath.isEmpty()) { return; }

  File file(filePath);

  if (not file.open(QFile::ReadOnly)) { throw RuntimeException("Erro lendo arquivo: " + file.errorString(), this); }

  XML xml(file.readAll(), XML::Tipo::Saida, this);

  xml.validar();

  SqlQuery query;
  query.prepare("SELECT 0 FROM nfe WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", xml.chaveAcesso);

  if (not query.exec()) { throw RuntimeException("Erro verificando se nota já cadastrada: " + query.lastError().text(), this); }

  if (query.first()) { throw RuntimeError("Nota já cadastrada!", this); }

  SqlQuery queryNFe;
  queryNFe.prepare("INSERT INTO nfe (idVenda, numeroNFe, tipo, xml, status, chaveAcesso, cnpjOrig, cnpjDest, valor) "
                   "VALUES (:idVenda, :numeroNFe, 'SAÍDA', :xml, 'AUTORIZADO', :chaveAcesso, :cnpjOrig, :cnpjDest, :valor)");
  queryNFe.bindValue(":idVenda", modelProdutos.data(0, "idVenda"));
  queryNFe.bindValue(":numeroNFe", xml.nNF);
  queryNFe.bindValue(":xml", xml.fileContent);
  queryNFe.bindValue(":chaveAcesso", xml.chaveAcesso);
  queryNFe.bindValue(":cnpjOrig", xml.cnpjOrig);
  queryNFe.bindValue(":cnpjDest", xml.cnpjDest);
  queryNFe.bindValue(":valor", xml.vNF_Total);

  if (not queryNFe.exec()) { throw RuntimeException("Erro importando NFe: " + queryNFe.lastError().text(), this); }

  const QVariant id = queryNFe.lastInsertId();

  // update other tables

  SqlQuery queryVenda;
  queryVenda.prepare("UPDATE venda_has_produto2 SET idNFeSaida = :idNFeSaida WHERE idVendaProduto2 = :idVendaProduto2");

  for (const auto &index : selection) {
    queryVenda.bindValue(":idNFeSaida", id);
    queryVenda.bindValue(":idVendaProduto2", modelProdutos.data(index.row(), "idVendaProduto2"));

    if (not queryVenda.exec()) { throw RuntimeException("Erro salvando NFe nos produtos: " + queryVenda.lastError().text(), this); }
  }

  updateTables();

  qApp->enqueueInformation("Nota importada com sucesso!", this);
}

// TODO: 1'em entrega' deve entrar na categoria 100% estoque?
// TODO: 5adicionar botao para cancelar agendamento (verificar com Anderson)
// TODO: 5refazer filtros do estoque (casos 'devolvido', 'cancelado', 'em entrega')
// TODO: permitir agendar sem estoque apenas produtos 'PENDENTE' para evitar problemas
