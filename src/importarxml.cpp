#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include "estoqueproxymodel.h"
#include "importarxml.h"
#include "noeditdelegate.h"
#include "reaisdelegate.h"
#include "singleeditdelegate.h"
#include "ui_importarxml.h"
#include "xml.h"

ImportarXML::ImportarXML(const QStringList &idsCompra, const QDateTime &dataReal, QWidget *parent)
    : QDialog(parent), dataReal(dataReal), idsCompra(idsCompra), ui(new Ui::ImportarXML) {
  ui->setupUi(this);

  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(Qt::Window);

  setupTables(idsCompra);
}

ImportarXML::~ImportarXML() { delete ui; }

void ImportarXML::setupTables(const QStringList &idsCompra) {
  modelEstoque.setTable("estoque");
  modelEstoque.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelEstoque.setHeaderData("status", "Status");
  modelEstoque.setHeaderData("numeroNFe", "NFe");
  modelEstoque.setHeaderData("local", "Local");
  modelEstoque.setHeaderData("fornecedor", "Fornecedor");
  modelEstoque.setHeaderData("descricao", "Produto");
  modelEstoque.setHeaderData("quant", "Quant.");
  modelEstoque.setHeaderData("un", "Un.");
  modelEstoque.setHeaderData("caixas", "Cx.");
  modelEstoque.setHeaderData("codBarras", "Cód. Bar.");
  modelEstoque.setHeaderData("codComercial", "Cód. Com.");
  modelEstoque.setHeaderData("valorUnid", "R$ Unid.");
  modelEstoque.setHeaderData("valor", "R$");

  modelEstoque.setFilter("status = 'TEMP'");

  if (not modelEstoque.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque: " + modelEstoque.lastError().text());
  }

  ui->tableEstoque->setModel(new EstoqueProxyModel(&modelEstoque, this));
  ui->tableEstoque->setItemDelegate(new NoEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("quant", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("un", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("valorUnid", new ReaisDelegate(this));
  ui->tableEstoque->setItemDelegateForColumn("valor", new ReaisDelegate(this));
  ui->tableEstoque->hideColumn("ordemCompra_");
  ui->tableEstoque->hideColumn("recebidoPor");
  ui->tableEstoque->hideColumn("quantUpd");
  ui->tableEstoque->hideColumn("idNFe");
  ui->tableEstoque->hideColumn("idEstoque");
  ui->tableEstoque->hideColumn("idCompra");
  ui->tableEstoque->hideColumn("idProduto");
  ui->tableEstoque->hideColumn("ncm");
  ui->tableEstoque->hideColumn("cfop");
  ui->tableEstoque->hideColumn("codBarrasTrib");
  ui->tableEstoque->hideColumn("unTrib");
  ui->tableEstoque->hideColumn("quantTrib");
  ui->tableEstoque->hideColumn("valorTrib");
  ui->tableEstoque->hideColumn("desconto");
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
  ui->tableEstoque->hideColumn("cstPIS");
  ui->tableEstoque->hideColumn("vBCPIS");
  ui->tableEstoque->hideColumn("pPIS");
  ui->tableEstoque->hideColumn("vPIS");
  ui->tableEstoque->hideColumn("cstCOFINS");
  ui->tableEstoque->hideColumn("vBCCOFINS");
  ui->tableEstoque->hideColumn("pCOFINS");
  ui->tableEstoque->hideColumn("vCOFINS");

  //

  modelConsumo.setTable("estoque_has_consumo");
  modelConsumo.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelConsumo.setHeaderData("status", "Status");
  //  modelConsumo.setHeaderData("ordemCompra", "OC");
  modelConsumo.setHeaderData("numeroNFe", "NFe");
  modelConsumo.setHeaderData("local", "Local");
  modelConsumo.setHeaderData("fornecedor", "Fornecedor");
  modelConsumo.setHeaderData("descricao", "Produto");
  modelConsumo.setHeaderData("quant", "Quant.");
  modelConsumo.setHeaderData("un", "Un.");
  modelConsumo.setHeaderData("caixas", "Cx.");
  modelConsumo.setHeaderData("codBarras", "Cód. Bar.");
  modelConsumo.setHeaderData("codComercial", "Cód. Com.");
  modelConsumo.setHeaderData("valorUnid", "R$ Unid.");
  modelConsumo.setHeaderData("valor", "R$");

  modelConsumo.setFilter("status = 'TEMP'");

  if (not modelConsumo.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela estoque_has_consumo: " + modelConsumo.lastError().text());
  }

  ui->tableConsumo->setModel(new EstoqueProxyModel(&modelConsumo, this));
  ui->tableConsumo->setItemDelegateForColumn("valorUnid", new ReaisDelegate(this));
  ui->tableConsumo->setItemDelegateForColumn("valor", new ReaisDelegate(this));
  ui->tableConsumo->hideColumn("ordemCompra_");
  ui->tableConsumo->hideColumn("idCompra_");
  ui->tableConsumo->hideColumn("idConsumo");
  ui->tableConsumo->hideColumn("quantUpd");
  ui->tableConsumo->hideColumn("idNFe");
  ui->tableConsumo->hideColumn("idEstoque");
  ui->tableConsumo->hideColumn("idCompra");
  ui->tableConsumo->hideColumn("idVendaProduto");
  ui->tableConsumo->hideColumn("idProduto");
  ui->tableConsumo->hideColumn("ncm");
  ui->tableConsumo->hideColumn("cfop");
  ui->tableConsumo->hideColumn("codBarrasTrib");
  ui->tableConsumo->hideColumn("unTrib");
  ui->tableConsumo->hideColumn("quantTrib");
  ui->tableConsumo->hideColumn("valorTrib");
  ui->tableConsumo->hideColumn("desconto");
  ui->tableConsumo->hideColumn("compoeTotal");
  ui->tableConsumo->hideColumn("numeroPedido");
  ui->tableConsumo->hideColumn("itemPedido");
  ui->tableConsumo->hideColumn("tipoICMS");
  ui->tableConsumo->hideColumn("orig");
  ui->tableConsumo->hideColumn("cstICMS");
  ui->tableConsumo->hideColumn("modBC");
  ui->tableConsumo->hideColumn("vBC");
  ui->tableConsumo->hideColumn("pICMS");
  ui->tableConsumo->hideColumn("vICMS");
  ui->tableConsumo->hideColumn("modBCST");
  ui->tableConsumo->hideColumn("pMVAST");
  ui->tableConsumo->hideColumn("vBCST");
  ui->tableConsumo->hideColumn("pICMSST");
  ui->tableConsumo->hideColumn("vICMSST");
  ui->tableConsumo->hideColumn("cEnq");
  ui->tableConsumo->hideColumn("cstIPI");
  ui->tableConsumo->hideColumn("cstPIS");
  ui->tableConsumo->hideColumn("vBCPIS");
  ui->tableConsumo->hideColumn("pPIS");
  ui->tableConsumo->hideColumn("vPIS");
  ui->tableConsumo->hideColumn("cstCOFINS");
  ui->tableConsumo->hideColumn("vBCCOFINS");
  ui->tableConsumo->hideColumn("pCOFINS");
  ui->tableConsumo->hideColumn("vCOFINS");

  //

  modelCompra.setTable("pedido_fornecedor_has_produto");
  modelCompra.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelCompra.setHeaderData("status", "Status");
  modelCompra.setHeaderData("ordemCompra", "OC");
  modelCompra.setHeaderData("idVenda", "Venda");
  modelCompra.setHeaderData("fornecedor", "Fornecedor");
  modelCompra.setHeaderData("descricao", "Produto");
  modelCompra.setHeaderData("colecao", "Coleção");
  modelCompra.setHeaderData("quant", "Quant.");
  modelCompra.setHeaderData("quantConsumida", "Consumido");
  modelCompra.setHeaderData("un", "Un.");
  modelCompra.setHeaderData("un2", "Un. 2");
  modelCompra.setHeaderData("caixas", "Cx.");
  modelCompra.setHeaderData("prcUnitario", "R$ Unid.");
  modelCompra.setHeaderData("preco", "R$");
  modelCompra.setHeaderData("kgcx", "Kg./Cx.");
  modelCompra.setHeaderData("formComercial", "Form. Com.");
  modelCompra.setHeaderData("codComercial", "Cód. Com.");
  modelCompra.setHeaderData("codBarras", "Cód. Bar.");
  modelCompra.setHeaderData("obs", "Obs.");

  modelCompra.setFilter("idCompra = " + idsCompra.join(" OR idCompra = "));

  if (not modelCompra.select()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro lendo tabela pedido_fornecedor_has_produto: " + modelCompra.lastError().text());
  }

  ui->tableCompra->setModel(new EstoqueProxyModel(&modelCompra, this));
  ui->tableCompra->setItemDelegate(new NoEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("codComercial", new SingleEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("descricao", new SingleEditDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("prcUnitario", new ReaisDelegate(this));
  ui->tableCompra->setItemDelegateForColumn("preco", new ReaisDelegate(this));
  ui->tableCompra->hideColumn("idVendaProduto");
  ui->tableCompra->hideColumn("selecionado");
  ui->tableCompra->hideColumn("statusFinanceiro");
  ui->tableCompra->hideColumn("quantUpd");
  ui->tableCompra->hideColumn("idCompra");
  ui->tableCompra->hideColumn("idNFe");
  ui->tableCompra->hideColumn("idEstoque");
  ui->tableCompra->hideColumn("idPedido");
  ui->tableCompra->hideColumn("idProduto");
  ui->tableCompra->hideColumn("dataPrevCompra");
  ui->tableCompra->hideColumn("dataRealCompra");
  ui->tableCompra->hideColumn("dataPrevConf");
  ui->tableCompra->hideColumn("dataRealConf");
  ui->tableCompra->hideColumn("dataPrevFat");
  ui->tableCompra->hideColumn("dataRealFat");
  ui->tableCompra->hideColumn("dataPrevColeta");
  ui->tableCompra->hideColumn("dataRealColeta");
  ui->tableCompra->hideColumn("dataPrevReceb");
  ui->tableCompra->hideColumn("dataRealReceb");
  ui->tableCompra->hideColumn("dataPrevEnt");
  ui->tableCompra->hideColumn("dataRealEnt");
  ui->tableCompra->hideColumn("aliquotaSt");
  ui->tableCompra->hideColumn("st");

  ui->tableCompra->resizeColumnsToContents();
}

bool ImportarXML::importar() {
  //--------------
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (not modelEstoque.setData(row, "status", "EM COLETA")) return false;
  }

  for (int row = 0; row < modelConsumo.rowCount(); ++row) {
    if (not modelConsumo.setData(row, "status", "PRÉ-CONSUMO")) return false;
  }

  for (int row = 0; row < modelCompra.rowCount(); ++row)
    if (not modelCompra.setData(row, "selecionado", false)) return false;

  //--------------

  bool ok = false;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") == Green) {
      ok = true;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nenhuma compra pareada!");
    return false;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    int color = modelEstoque.data(row, "quantUpd").toInt();

    if (color == Red) {
      ok = false;
      break;
    }
  }

  if (not ok) {
    QMessageBox::critical(this, "Erro!", "Nem todos os estoques estão ok!");
    return false;
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    return false;
  }

  if (not modelCompra.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela compra: " + modelCompra.lastError().text());
    return false;
  }

  if (not modelConsumo.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados do consumo: " + modelConsumo.lastError().text());
    return false;
  }

  // NOTE: todos as linhas em cima devem ser pareadas mas nem todas em baixo precisam ser (como a nota nao pode ser
  // cadastrada duas vezes seus produtos devem ser todos pareados)

  //------------------------------
  QSqlQuery query;
  for (auto const &idCompra : idsCompra) {
    query.prepare("UPDATE pedido_fornecedor_has_produto SET status = 'EM COLETA', dataRealFat = :dataRealFat WHERE "
                  "idCompra = :idCompra AND quantUpd = 1");
    query.bindValue(":dataRealFat", dataReal);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro atualizando status da compra: " + query.lastError().text());
      return false;
    }
  }
  //------------------------------

  return true;
}

void ImportarXML::on_pushButtonImportar_clicked() {
  QSqlQuery("SAVEPOINT importar").exec();

  if (not importar()) {
    QMessageBox::critical(this, "Erro!", "Erro importando nota!");
    QSqlQuery("ROLLBACK TO importar").exec();
    return;
  }

  QDialog::accept();
  close();
}

bool ImportarXML::limparAssociacoes() {
  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd").toInt() == 1) continue;
    if (modelCompra.data(row, "status").toString() != "EM FATURAMENTO") continue;

    if (not modelCompra.setData(row, "quantUpd", 0)) return false;
    if (not modelCompra.setData(row, "quantConsumida", 0)) return false;
  }

  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    if (not modelEstoque.setData(row, "quantUpd", 0)) return false;
  }

  for (int row = 0; row < modelConsumo.rowCount(); ++row) {
    QSqlQuery query;
    query.prepare("UPDATE venda_has_produto SET status = 'EM FATURAMENTO' WHERE idVendaProduto = :idVendaProduto");
    query.bindValue(":idVendaProduto", modelConsumo.data(row, "idVendaProduto"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro limpando status do produto da venda: " + query.lastError().text());
      return false;
    }

    if (not modelConsumo.removeRow(row)) return false;
  }

  return true;
}

bool ImportarXML::procurar() {
  const QString filePath = QFileDialog::getOpenFileName(this, "Arquivo XML", QDir::currentPath(), "XML (*.xml)");

  if (filePath.isEmpty()) {
    ui->lineEdit->setText(QString());
    return false;
  }

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return false;
  }

  if (not lerXML(file)) return false;

  file.close();

  if (not parear()) return false;

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela estoque: " + modelEstoque.lastError().text());
    return false;
  }

  if (not modelCompra.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados da tabela compra: " + modelCompra.lastError().text());
    return false;
  }

  if (not modelConsumo.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando dados do consumo: " + modelConsumo.lastError().text());
    return false;
  }

  bool ok = true;

  for (int row = 0; row < modelCompra.rowCount(); ++row) {
    if (modelCompra.data(row, "quantUpd") != Green) {
      ok = false;
      break;
    }
  }

  if (ok) ui->pushButtonProcurar->setDisabled(true);

  ui->tableEstoque->resizeColumnsToContents();
  ui->tableConsumo->resizeColumnsToContents();
  ui->tableCompra->resizeColumnsToContents();

  return true;
}

void ImportarXML::on_pushButtonProcurar_clicked() {
  QSqlQuery("SAVEPOINT procurar").exec();

  if (not procurar()) {
    QMessageBox::critical(this, "Erro!", "Erro carregando nota!");
    QSqlQuery("ROLLBACK TO procurar").exec();
    return;
  }
}

bool ImportarXML::associarItens(const int rowCompra, const int rowEstoque, double &estoqueConsumido) {
  if (modelEstoque.data(rowEstoque, "quantUpd") == Green) return true;
  if (modelCompra.data(rowCompra, "quantUpd") == Green) return true;

  //-------------------------------

  const double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
  const double quantCompra = modelCompra.data(rowCompra, "quant").toDouble();
  const double quantConsumida = modelCompra.data(rowCompra, "quantConsumida").toDouble();

  const double espaco = quantCompra - quantConsumida;
  const double estoqueDisponivel = quantEstoque - estoqueConsumido;
  const double quantAdicionar = estoqueDisponivel > espaco ? espaco : estoqueDisponivel;
  estoqueConsumido += quantAdicionar;

  if (not modelCompra.setData(rowCompra, "quantConsumida", quantConsumida + quantAdicionar)) {
    return false;
  }

  if (not modelEstoque.setData(rowEstoque, "quantUpd",
                               qFuzzyCompare(estoqueConsumido, quantEstoque) ? Green : Yellow)) {
    return false;
  }

  if (not modelCompra.setData(rowCompra, "quantUpd",
                              qFuzzyCompare((quantConsumida + quantAdicionar), quantCompra) ? Green : Yellow)) {
    return false;
  }

  const int idCompra = modelCompra.data(rowCompra, "idCompra").toInt();
  const int idEstoque = modelEstoque.data(rowEstoque, "idEstoque").toInt();

  QSqlQuery query;
  query.prepare("SELECT idEstoque FROM estoque_has_compra WHERE idEstoque = :idEstoque AND idCompra = :idCompra");
  query.bindValue(":idEstoque", idEstoque);
  query.bindValue(":idCompra", idCompra);

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando em estoque_has_compra: " + query.lastError().text());
    return false;
  }

  if (query.size() == 0) {
    query.prepare("INSERT INTO estoque_has_compra (idEstoque, idCompra) VALUES (:idEstoque, :idCompra)");
    query.bindValue(":idEstoque", idEstoque);
    query.bindValue(":idCompra", idCompra);

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro salvando em estoque_has_compra: " + query.lastError().text());
      return false;
    }
  }

  return true;
}

bool ImportarXML::lerXML(QFile &file) {
  XML xml(file.readAll(), file.fileName());
  if (not xml.cadastrarNFe("ENTRADA")) return false;
  if (not xml.mostrarNoSqlModel(modelEstoque)) return false;

  return true;
}

bool ImportarXML::criarConsumo() {
  for (int row = 0; row < modelEstoque.rowCount(); ++row) {
    const QString codComercial = modelEstoque.data(row, "codComercial").toString();
    const int idEstoque = modelEstoque.data(row, "idEstoque").toInt();

    QSqlQuery queryIdCompra;
    queryIdCompra.prepare("SELECT idCompra FROM estoque_has_compra WHERE idEstoque = :idEstoque");
    queryIdCompra.bindValue(":idEstoque", idEstoque);

    if (not queryIdCompra.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando idCompra: " + queryIdCompra.lastError().text());
      return false;
    }

    while (queryIdCompra.next()) {
      QSqlQuery queryProduto;
      queryProduto.prepare(
          "SELECT quant, idVendaProduto, idProduto FROM venda_has_produto WHERE codComercial = :codComercial AND "
          "idCompra = :idCompra AND status = 'EM FATURAMENTO'");
      queryProduto.bindValue(":codComercial", codComercial);
      queryProduto.bindValue(":idCompra", queryIdCompra.value("idCompra"));

      if (not queryProduto.exec()) {
        QMessageBox::critical(this, "Erro!", "Erro buscando idVendaProduto: " + queryIdCompra.lastError().text());
        return false;
      }

      while (queryProduto.next()) {
        const auto list = modelEstoque.match(modelEstoque.index(0, modelEstoque.fieldIndex("idEstoque")),
                                             Qt::DisplayRole, idEstoque, -1);

        double quantTemp = 0;

        for (auto const &item : list) quantTemp += modelEstoque.data(item.row(), "quant").toDouble();

        // TODO: 2em vez de pular criar um consumo parcial? (e na proxima passada consumir a sobra no lugar do total)
        if (queryProduto.value("quant") > quantTemp) continue;

        const int newRow = modelConsumo.rowCount();
        modelConsumo.insertRow(newRow);

        for (int column = 0; column < modelEstoque.columnCount(); ++column) {
          const QString field = modelEstoque.record().fieldName(column);
          const int index = modelConsumo.fieldIndex(field);
          const QVariant value = modelEstoque.data(row, column);

          if (index != -1 and not modelConsumo.setData(newRow, index, value)) return false;
        }

        const double quant = queryProduto.value("quant").toDouble();

        // -------------------------------------

        QSqlQuery queryTemp;
        queryTemp.prepare("SELECT un, kgcx, m2cx, pccx FROM produto WHERE idProduto = :idProduto");
        queryTemp.bindValue(":idProduto", queryProduto.value("idProduto"));

        if (not queryTemp.exec()) {
          QMessageBox::critical(this, "Erro!", "Erro buscando dados do produto: " + queryTemp.lastError().text());
          return false;
        }

        int caixas = 0;

        if (queryTemp.first()) {
          const QString un = queryTemp.value("un").toString();
          const double kgcx = queryTemp.value("kgcx").toDouble();
          const double m2cx = queryTemp.value("m2cx").toDouble();
          const double pccx = queryTemp.value("pccx").toDouble();

          if (un == "KG") {
            if (kgcx > 0.) caixas = quant / kgcx;
          } else if (un == "M2" or un == "M²") {
            if (m2cx > 0.) caixas = quant / m2cx;
          } else {
            if (pccx > 0.) caixas = quant / pccx;
          }
        }

        const double proporcao = quant / modelEstoque.data(row, "idEstoque").toDouble();

        const double valor = modelEstoque.data(row, "valor").toDouble() * proporcao;
        const double vBC = modelEstoque.data(row, "vBC").toDouble() * proporcao;
        const double vICMS = modelEstoque.data(row, "vICMS").toDouble() * proporcao;
        const double vBCST = modelEstoque.data(row, "vBCST").toDouble() * proporcao;
        const double vICMSST = modelEstoque.data(row, "vICMSST").toDouble() * proporcao;
        const double vBCPIS = modelEstoque.data(row, "vBCPIS").toDouble() * proporcao;
        const double vPIS = modelEstoque.data(row, "vPIS").toDouble() * proporcao;
        const double vBCCOFINS = modelEstoque.data(row, "vBCCOFINS").toDouble() * proporcao;
        const double vCOFINS = modelEstoque.data(row, "vCOFINS").toDouble() * proporcao;

        // -------------------------------------

        if (not modelConsumo.setData(newRow, "quant", quant * -1)) return false;
        if (not modelConsumo.setData(newRow, "caixas", caixas)) return false;
        if (not modelConsumo.setData(newRow, "quantUpd", DarkGreen)) return false;
        if (not modelConsumo.setData(newRow, "idVendaProduto", queryProduto.value("idVendaProduto"))) return false;
        if (not modelConsumo.setData(newRow, "idEstoque", modelEstoque.data(row, "idEstoque"))) return false;

        if (not modelConsumo.setData(newRow, "valor", valor)) return false;
        if (not modelConsumo.setData(newRow, "vBC", vBC)) return false;
        if (not modelConsumo.setData(newRow, "vICMS", vICMS)) return false;
        if (not modelConsumo.setData(newRow, "vBCST", vBCST)) return false;
        if (not modelConsumo.setData(newRow, "vICMSST", vICMSST)) return false;
        if (not modelConsumo.setData(newRow, "vBCPIS", vBCPIS)) return false;
        if (not modelConsumo.setData(newRow, "vPIS", vPIS)) return false;
        if (not modelConsumo.setData(newRow, "vBCCOFINS", vBCCOFINS)) return false;
        if (not modelConsumo.setData(newRow, "vCOFINS", vCOFINS)) return false;

        QSqlQuery query;
        query.prepare("UPDATE venda_has_produto SET status = 'EM COLETA', dataRealFat = :dataRealFat WHERE "
                      "idVendaProduto = :idVendaProduto");
        query.bindValue(":dataRealFat", dataReal);
        query.bindValue(":idVendaProduto", queryProduto.value("idVendaProduto"));

        if (not query.exec()) {
          QMessageBox::critical(this, "Erro!",
                                "Erro atualizando status do produto da venda: " + query.lastError().text());
          return false;
        }
      }
    }
  }

  return true;
}

void ImportarXML::on_pushButtonCancelar_clicked() { close(); }

void ImportarXML::on_pushButtonReparear_clicked() {
  QSqlQuery("SAVEPOINT parear").exec();

  if (not parear()) {
    QMessageBox::critical(this, "Erro!", "Erro pareando!");
    QSqlQuery("ROLLBACK TO parear").exec();
    return;
  }

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();
}

bool ImportarXML::parear() {
  if (not limparAssociacoes()) return false;

  for (int rowEstoque = 0; rowEstoque < modelEstoque.rowCount(); ++rowEstoque) {
    const auto temp = modelCompra.match(modelCompra.index(0, modelCompra.fieldIndex("codComercial")), Qt::DisplayRole,
                                        modelEstoque.data(rowEstoque, "codComercial"), -1,
                                        Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchWrap));

    if (temp.size() == 0) {
      if (not modelEstoque.setData(rowEstoque, "quantUpd", Red)) return false;

      continue;
    }

    // remover da lista linhas que nao estao 'em faturamento'

    QModelIndexList list;

    for (auto const &item : temp) {
      if (modelCompra.data(item.row(), "status").toString() == "EM FATURAMENTO") list << item;
    }

    QSqlQuery query;
    query.prepare("SELECT idProduto FROM produto WHERE codComercial = :codComercial");
    query.bindValue(":codComercial", modelEstoque.data(rowEstoque, "codComercial"));

    if (not query.exec()) {
      QMessageBox::critical(0, "Erro!", "Erro lendo tabela produto: " + query.lastError().text());
      return false;
    }

    if (not query.first()) {
      QMessageBox::critical(0, "Erro!", "Não encontrou o produto!");
      return false;
    }

    if (not modelEstoque.setData(rowEstoque, "idProduto", query.value("idProduto"))) return false;

    double estoqueConsumido = 0;

    //------------------------ procurar quantidades iguais
    for (auto const &item : list) {
      const double quantEstoque = modelEstoque.data(rowEstoque, "quant").toDouble();
      const double quantCompra = modelCompra.data(item.row(), "quant").toDouble();

      if (quantEstoque == quantCompra) {
        if (not associarItens(item.row(), rowEstoque, estoqueConsumido)) return false;
      }
    }
    //------------------------

    for (auto const &item : list) {
      if (not associarItens(item.row(), rowEstoque, estoqueConsumido)) return false;
    }
  }

  //---------------------------
  if (not criarConsumo()) return false;
  //---------------------------

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando estoque: " + modelEstoque.lastError().text());
    return false;
  }

  if (not modelConsumo.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro salvando consumo: " + modelConsumo.lastError().text());
    return false;
  }

  ui->tableEstoque->clearSelection();
  ui->tableCompra->clearSelection();

  return true;
}

void ImportarXML::on_tableEstoque_entered(const QModelIndex &) { ui->tableEstoque->resizeColumnsToContents(); }

void ImportarXML::on_tableCompra_entered(const QModelIndex &) { ui->tableCompra->resizeColumnsToContents(); }

void ImportarXML::on_pushButtonRemover_clicked() {
  const auto list = ui->tableEstoque->selectionModel()->selectedRows();

  if (list.size() == 0) {
    QMessageBox::critical(this, "Erro!", "Nenhuma linha selecionada!");
    return;
  }

  for (auto const &item : list) {
    QSqlQuery query;

    query.prepare("DELETE FROM estoque_has_nfe WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro removendo de estoque_has_nfe: " + query.lastError().text());
      return;
    }

    query.prepare("DELETE FROM estoque_has_compra WHERE idEstoque = :idEstoque");
    query.bindValue(":idEstoque", modelEstoque.data(item.row(), "idEstoque"));

    if (not query.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro removendo de estoque_has_compra: " + query.lastError().text());
      return;
    }

    modelEstoque.removeRow(item.row());
  }

  if (not modelEstoque.submitAll()) {
    QMessageBox::critical(this, "Erro!", "Erro removendo linhas: " + modelEstoque.lastError().text());
    return;
  }
}

void ImportarXML::on_tableConsumo_entered(const QModelIndex &) { ui->tableConsumo->resizeColumnsToContents(); }

// NOTE: revisar codigo para verificar se ao pesquisar um produto pelo codComercial+status nao estou alterando outros
// produtos junto
// NOTE: utilizar tabela em arvore (qtreeview) para agrupar consumos com seu estoque (para cada linha do model inserir
// items na arvore?)
