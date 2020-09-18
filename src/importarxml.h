#pragma once

#include "estoqueproxymodel.h"
#include "sqltablemodel.h"
#include "xml.h"

#include <QDate>
#include <QDialog>

namespace Ui {
class ImportarXML;
}

class ImportarXML final : public QDialog {
  Q_OBJECT

  struct NCM {
    double mva4;
    double mva12;
    double aliq;
  };

  struct ProdutoEstoque {
    int idProduto;
    int idEstoque;
    double estoqueRestante;
    double valorUnid;
  };

public:
  explicit ImportarXML(const QStringList &idsCompra, const QDate &dataFaturamento, QWidget *parent);
  ~ImportarXML();

private:
  // attributes
  const QDate dataFaturamento;
  const QStringList idsCompra;
  QMap<QString, double> mapNFes;
  SqlTableModel modelCompra;
  SqlTableModel modelConsumo;
  SqlTableModel modelEstoque;
  SqlTableModel modelVenda;
  SqlTableModel modelEstoque_compra;
  SqlTableModel modelNFe;
  SqlTableModel modelPagamento;
  Ui::ImportarXML *ui;

  enum class FieldColors {
    None = 0,      // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
  };

  // methods
  auto associarDiferente(const int rowCompra, const int rowEstoque, double &estoquePareado, bool &repareado) -> bool;
  auto associarIgual(const int rowCompra, const int rowEstoque) -> bool;
  auto atualizarNFes() -> void;
  auto buscaNCM(const QString &ncm) -> ImportarXML::NCM;
  auto buscarCaixas(const int rowEstoque) -> double;
  auto cadastrarNFe(XML &xml, const double gare) -> bool;
  auto cadastrarProdutoEstoque(const QVector<ProdutoEstoque> &tuples) -> void;
  auto calculaGare(XML &xml) -> double;
  auto criarConsumo(const int rowCompra, const int rowEstoque) -> bool;
  auto criarPagamentoGare(const double valor, const XML &xml) -> bool;
  auto dividirCompra(const int rowCompra, const double quantAdicionar) -> void;
  auto dividirVenda(const int rowVenda, const double quantAdicionar) -> int;
  auto encontraInfCpl(const QString &xml) -> QString;
  auto importar() -> void;
  auto lerXML() -> bool;
  auto limparAssociacoes() -> bool;
  auto mapTuples() -> QVector<ImportarXML::ProdutoEstoque>;
  auto on_checkBoxSemLote_toggled(const bool checked) -> void;
  auto on_itemBoxNFe_textChanged(const QString &text) -> void;
  auto on_pushButtonCancelar_clicked() -> void;
  auto on_pushButtonImportar_clicked() -> void;
  auto on_pushButtonProcurar_clicked() -> void;
  auto parear() -> bool;
  auto percorrerXml(XML &xml) -> bool;
  auto perguntarLocal(XML &xml) -> bool;
  auto reparear(const QModelIndex &index) -> void;
  auto salvarDadosCompra() -> void;
  auto salvarDadosVenda() -> void;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto updateTableData(const QModelIndex &topLeft) -> void;
  auto usarXMLBaixado() -> bool;
  auto verificaExiste(const XML &xml) -> bool;
  auto verifyFields() -> bool;
};
