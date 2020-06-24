#include "estoque.h"
#include "ui_estoque.h"

#include "application.h"
#include "doubledelegate.h"
#include "estoqueproxymodel.h"
#include "usersession.h"
#include "xml_viewer.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

Estoque::Estoque(const QString &idEstoque, const bool showWindow, QWidget *parent) : QDialog(parent), idEstoque(idEstoque), ui(new Ui::Estoque) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);

  setupTables();

  connect(ui->pushButtonExibirNfe, &QPushButton::clicked, this, &Estoque::on_pushButtonExibirNfe_clicked);

  viewRegisterById(showWindow);

  const QString tipoUsuario = UserSession::tipoUsuario();

  if (tipoUsuario == "VENDEDOR" or tipoUsuario == "VENDEDOR ESPECIAL") { ui->pushButtonExibirNfe->hide(); }
}

Estoque::~Estoque() { delete ui; }

void Estoque::setupTables() {
  modelEstoque.setTable("estoque");

  modelEstoque.setHeaderData("idEstoque", "Estoque");
  modelEstoque.setHeaderData("recebidoPor", "Recebido Por");
  modelEstoque.setHeaderData("status", "Status");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("descricao", "Produto");
  modelEstoque.setHeaderData("observacao", "Obs.");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Caixas");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("lote", "Lote");
  modelEstoque.setHeaderData("bloco", "Bloco");

  modelEstoque.proxyModel = new EstoqueProxyModel(&modelEstoque, this);

  ui->tableEstoque->setModel(&modelEstoque);

  ui->tableEstoque->setItemDelegateForColumn("quant", new DoubleDelegate(4, this));

  ui->tableEstoque->hideColumn("restante");
  ui->tableEstoque->hideColumn("ajuste");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("codBarras");
  ui->tableEstoque->hideColumn("ncm");
  ui->tableEstoque->hideColumn("nve");
  ui->tableEstoque->hideColumn("extipi");
  ui->tableEstoque->hideColumn("cest");
  ui->tableEstoque->hideColumn("cfop");
  ui->tableEstoque->hideColumn("valorUnid");
  ui->tableEstoque->hideColumn("valor");
  ui->tableEstoque->hideColumn("codBarrasTrib");
  ui->tableEstoque->hideColumn("unTrib");
  ui->tableEstoque->hideColumn("quantTrib");
  ui->tableEstoque->hideColumn("valorUnidTrib");
  ui->tableEstoque->hideColumn("frete");
  ui->tableEstoque->hideColumn("seguro");
  ui->tableEstoque->hideColumn("desconto");
  ui->tableEstoque->hideColumn("outros");
  ui->tableEstoque->hideColumn("compoeTotal");
  ui->tableEstoque->hideColumn("numeroPedido");
  ui->tableEstoque->hideColumn("itemPedido");
  ui->tableEstoque->hideColumn("tipoICMS");
  ui->tableEstoque->hideColumn("orig");
  ui->tableEstoque->hideColumn("cstICMS");
  ui->tableEstoque->hideColumn("modBC");
  ui->tableEstoque->hideColumn("vBC");
  ui->tableEstoque->hideColumn("pICMS");
  ui->tableEstoque->hideColumn("vICMS");
  ui->tableEstoque->hideColumn("modBCST");
  ui->tableEstoque->hideColumn("pMVAST");
  ui->tableEstoque->hideColumn("vBCST");
  ui->tableEstoque->hideColumn("pICMSST");
  ui->tableEstoque->hideColumn("vICMSST");
  ui->tableEstoque->hideColumn("cEnq");
  ui->tableEstoque->hideColumn("cstIPI");
  ui->tableEstoque->hideColumn("vBCIPI");
  ui->tableEstoque->hideColumn("pIPI");
  ui->tableEstoque->hideColumn("vIPI");
  ui->tableEstoque->hideColumn("cstPIS");
  ui->tableEstoque->hideColumn("vBCPIS");
  ui->tableEstoque->hideColumn("pPIS");
  ui->tableEstoque->hideColumn("vPIS");
  ui->tableEstoque->hideColumn("cstCOFINS");
  ui->tableEstoque->hideColumn("vBCCOFINS");
  ui->tableEstoque->hideColumn("pCOFINS");
  ui->tableEstoque->hideColumn("vCOFINS");
  ui->tableEstoque->hideColumn("valorGare");

  //--------------------------------------------------------------------

  modelConsumo.setTable("estoque_has_consumo");

  //--------------------------------------------------------------------

  modelViewConsumo.setTable("view_estoque_consumo");

  modelViewConsumo.setHeaderData("statusProduto", "Status Pedido");
  modelViewConsumo.setHeaderData("status", "Status Consumo");
  modelViewConsumo.setHeaderData("bloco", "Bloco");
  modelViewConsumo.setHeaderData("fornecedor", "Fornecedor");
  modelViewConsumo.setHeaderData("descricao", "Produto");
  modelViewConsumo.setHeaderData("quant", "Quant.");
  modelViewConsumo.setHeaderData("un", "Un.");
  modelViewConsumo.setHeaderData("caixas", "Caixas");
  modelViewConsumo.setHeaderData("codComercial", "Cód. Com.");
  modelViewConsumo.setHeaderData("dataRealEnt", "Entrega");
  modelViewConsumo.setHeaderData("created", "Criado");

  modelViewConsumo.proxyModel = new EstoqueProxyModel(&modelViewConsumo, this);

  ui->tableConsumo->setModel(&modelViewConsumo);

  ui->tableConsumo->setItemDelegateForColumn("quant", new DoubleDelegate(4, this));

  ui->tableConsumo->showColumn("created");
  ui->tableConsumo->hideColumn("idEstoque");
  ui->tableConsumo->hideColumn("quantUpd");
}

void Estoque::buscarRestante() {
  const double quantRestante = modelEstoque.data(0, "restante").toDouble();
  const QString un = modelEstoque.data(0, "un").toString();

  ui->doubleSpinBoxRestante->setValue(quantRestante);
  ui->doubleSpinBoxRestante->setSuffix(" " + un);
}

bool Estoque::viewRegisterById(const bool showWindow) {
  if (idEstoque.isEmpty()) { return qApp->enqueueError(false, "Estoque não encontrado!", this); }

  modelEstoque.setFilter("idEstoque = " + idEstoque);

  if (not modelEstoque.select()) { return false; }

  //--------------------------------------

  modelViewConsumo.setFilter("idEstoque = " + idEstoque);

  if (not modelViewConsumo.select()) { return false; }

  //--------------------------------------

  buscarRestante();

  if (showWindow) { show(); }

  return true;
}

void Estoque::on_pushButtonExibirNfe_clicked() { exibirNota(); }

void Estoque::exibirNota() {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", modelEstoque.data(0, "idNFe"));

  if (not query.exec()) { return qApp->enqueueError("Erro buscando nfe: " + query.lastError().text(), this); }

  if (query.size() == 0) { return qApp->enqueueWarning("Não encontrou NFe associada!", this); }

  while (query.next()) {
    auto *viewer = new XML_Viewer(query.value("xml").toByteArray(), this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
  }
}

bool Estoque::criarConsumo(const int idVendaProduto2, const double quant) {
  // TODO: verificar se as divisões de linha batem com a outra função criarConsumo

  if (modelEstoque.filter().isEmpty()) { return qApp->enqueueError(false, "Não setou idEstoque!", this); }

  if (quant > ui->doubleSpinBoxRestante->value()) { return qApp->enqueueError(false, "Quantidade insuficiente!", this); }

  // -------------------------------------------------------------------------

  if (not dividirCompra(idVendaProduto2, quant)) { return false; }

  // -------------------------------------------------------------------------

  const int rowEstoque = 0;
  const int rowConsumo = modelConsumo.insertRowAtEnd();

  for (int column = 0, columnCount = modelEstoque.columnCount(); column < columnCount; ++column) {
    const QString field = modelEstoque.record().fieldName(column);
    const int index = modelConsumo.fieldIndex(field, true);

    if (index == -1) { continue; }
    if (column == modelEstoque.fieldIndex("valor")) { break; }

    const QVariant value = modelEstoque.data(rowEstoque, column);

    if (value.isNull()) { continue; }

    if (not modelConsumo.setData(rowConsumo, index, value)) { return false; }
  }

  QSqlQuery query;
  query.prepare("SELECT quantCaixa FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", modelEstoque.data(rowEstoque, "idProduto"));

  if (not query.exec() or not query.first()) { return qApp->enqueueError(false, "Erro buscando dados do produto: " + query.lastError().text(), this); }

  const double quantCaixa = query.value("quantCaixa").toDouble();
  const double caixas = quant / quantCaixa;

  if (not modelConsumo.setData(rowConsumo, "idEstoque", modelEstoque.data(rowEstoque, "idEstoque"))) { return false; }
  if (not modelConsumo.setData(rowConsumo, "idVendaProduto2", idVendaProduto2)) { return false; }
  if (not modelConsumo.setData(rowConsumo, "status", "CONSUMO")) { return false; }
  if (not modelConsumo.setData(rowConsumo, "quant", quant * -1)) { return false; }
  if (not modelConsumo.setData(rowConsumo, "quantUpd", static_cast<int>(FieldColors::DarkGreen))) { return false; }
  if (not modelConsumo.setData(rowConsumo, "caixas", caixas)) { return false; }
  const double valorUnid = modelConsumo.data(rowConsumo, "valorUnid").toDouble();
  if (not modelConsumo.setData(rowConsumo, "valor", quant * valorUnid)) { return false; }

  if (not modelConsumo.submitAll()) { return false; }

  // -------------------------------------------------------------------------
  // copy lote to venda_has_produto

  const QString lote = modelEstoque.data(rowEstoque, "lote").toString();

  if (not lote.isEmpty() and lote != "N/D") {
    QSqlQuery queryProduto;
    queryProduto.prepare("UPDATE venda_has_produto2 SET lote = :lote WHERE idVendaProduto2 = :idVendaProduto2");
    queryProduto.bindValue(":lote", modelEstoque.data(rowEstoque, "lote"));
    queryProduto.bindValue(":idVendaProduto2", idVendaProduto2);

    if (not queryProduto.exec()) { return qApp->enqueueError(false, "Erro salvando lote: " + queryProduto.lastError().text(), this); }
  }

  // -------------------------------------------------------------------------

  return true;
}

bool Estoque::dividirCompra(const int idVendaProduto2, const double quant) {
  // se quant a consumir for igual a quant da compra apenas alterar idVenda/produto
  // senao fazer a quebra

  SqlTableModel modelCompra;
  modelCompra.setTable("pedido_fornecedor_has_produto2");

  const QString subQuery = "SELECT idPedido2 FROM estoque_has_compra WHERE idEstoque = " + idEstoque;
  modelCompra.setFilter("(idPedido2 IN (" + subQuery + ") OR idRelacionado IN (" + subQuery + ")) AND idVenda IS NULL AND idVendaProduto2 IS NULL AND quant >= " + QString::number(quant));

  if (not modelCompra.select()) { return false; }

  if (modelCompra.rowCount() == 0) { return true; }

  //--------------------------------------------------------------------

  QSqlQuery query;
  query.prepare("SELECT idVenda FROM venda_has_produto2 WHERE idVendaProduto2 = :idVendaProduto2");
  query.bindValue(":idVendaProduto2", idVendaProduto2);

  if (not query.exec() or not query.first()) { return qApp->enqueueError(false, "Erro buscando idVenda: " + query.lastError().text(), this); }

  const int row = 0;
  const double quantCompra = modelCompra.data(row, "quant").toDouble();

  if (quant > quantCompra) { return qApp->enqueueError(false, "Erro quant > quantCompra", this); }

  if (qFuzzyCompare(quant, quantCompra)) {
    if (not modelCompra.setData(row, "idVenda", query.value("idVenda"))) { return false; }
    if (not modelCompra.setData(row, "idVendaProduto2", idVendaProduto2)) { return false; }
  }

  const bool dividir = quant < quantCompra;

  if (dividir) {
    // NOTE: *quebralinha pedido_fornecedor2

    const double caixas = modelCompra.data(row, "caixas").toDouble();
    const double prcUnitario = modelCompra.data(row, "prcUnitario").toDouble();
    const double quantOriginal = modelCompra.data(row, "quant").toDouble();
    const double proporcaoNovo = quant / quantOriginal;
    const double proporcaoAntigo = (quantOriginal - quant) / quantOriginal;

    if (not modelCompra.setData(row, "quant", quantOriginal - quant)) { return false; }
    if (not modelCompra.setData(row, "caixas", caixas * proporcaoAntigo)) { return false; }
    if (not modelCompra.setData(row, "preco", prcUnitario * (quantOriginal - quant))) { return false; }

    // -------------------------------------------------------------------------

    const int newRow = modelCompra.insertRowAtEnd();

    for (int column = 0, columnCount = modelCompra.columnCount(); column < columnCount; ++column) {
      if (column == modelCompra.fieldIndex("idPedido2")) { continue; }
      if (column == modelCompra.fieldIndex("created")) { continue; }
      if (column == modelCompra.fieldIndex("lastUpdated")) { continue; }

      const QVariant value = modelCompra.data(row, column);

      if (value.isNull()) { continue; }

      if (not modelCompra.setData(newRow, column, value)) { return false; }
    }

    // -------------------------------------------------------------------------

    if (not modelCompra.setData(newRow, "idRelacionado", modelCompra.data(row, "idPedido2"))) { return false; }
    if (not modelCompra.setData(newRow, "idVenda", query.value("idVenda"))) { return false; }
    if (not modelCompra.setData(newRow, "idVendaProduto2", idVendaProduto2)) { return false; }
    if (not modelCompra.setData(newRow, "quant", quant)) { return false; }
    if (not modelCompra.setData(newRow, "caixas", caixas * proporcaoNovo)) { return false; }
    if (not modelCompra.setData(newRow, "preco", prcUnitario * quant)) { return false; }
  }

  if (not modelCompra.submitAll()) { return false; }

  return true;
}

bool Estoque::desfazerConsumo(const int idVendaProduto2) {
  // there is one implementation in InputDialogConfirmacao
  // TODO: juntar as lógicas
  // TODO: se houver agendamento de estoque remover

  // NOTE: estoque_has_consumo may have the same idVendaProduto2 in more than one row (only until the field is made UNIQUE)
  QSqlQuery queryDelete;
  queryDelete.prepare("DELETE FROM estoque_has_consumo WHERE idVendaProduto2 = :idVendaProduto2");
  queryDelete.bindValue(":idVendaProduto2", idVendaProduto2);

  if (not queryDelete.exec()) { return qApp->enqueueError(false, "Erro removendo consumo estoque: " + queryDelete.lastError().text()); }

  // TODO: juntar linhas sem consumo do mesmo tipo? (usar idRelacionado)
  QSqlQuery queryCompra;
  queryCompra.prepare("UPDATE pedido_fornecedor_has_produto2 SET idVenda = NULL, idVendaProduto2 = NULL WHERE idVendaProduto2 = :idVendaProduto2 AND status NOT IN ('CANCELADO', 'DEVOLVIDO')");
  queryCompra.bindValue(":idVendaProduto2", idVendaProduto2);

  if (not queryCompra.exec()) { return qApp->enqueueError(false, "Erro atualizando pedido compra: " + queryCompra.lastError().text()); }

  QSqlQuery queryVenda;
  queryVenda.prepare(
      "UPDATE venda_has_produto2 SET status = CASE WHEN reposicaoEntrega THEN 'REPO. ENTREGA' WHEN reposicaoReceb THEN 'REPO. RECEB.' ELSE 'PENDENTE' END, idCompra = NULL, lote = NULL, "
      "dataPrevCompra = NULL, dataRealCompra = NULL, dataPrevConf = NULL, dataRealConf = NULL, dataPrevFat = NULL, dataRealFat = NULL, dataPrevColeta = NULL, dataRealColeta = NULL, dataPrevReceb = "
      "NULL, dataRealReceb = NULL, dataPrevEnt = NULL, dataRealEnt = NULL WHERE `idVendaProduto2` = :idVendaProduto2 AND status NOT IN ('CANCELADO', 'DEVOLVIDO', 'QUEBRADO')");
  queryVenda.bindValue(":idVendaProduto2", idVendaProduto2);

  if (not queryVenda.exec()) { return qApp->enqueueError(false, "Erro atualizando pedido venda: " + queryVenda.lastError().text()); }

  return true;
}

// TODO: 1colocar o botao de desvincular consumo nesta tela
// TODO: no view_widget_estoque deixar apenas o status do consumo
