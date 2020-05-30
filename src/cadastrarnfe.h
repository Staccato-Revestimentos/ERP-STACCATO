#pragma once

#include "acbr.h"
#include "sqltablemodel.h"

#include <QDataWidgetMapper>
#include <QDialog>
#include <QSqlQuery>
#include <QTextStream>

namespace Ui {
class CadastrarNFe;
}

class CadastrarNFe final : public QDialog {
  Q_OBJECT

public:
  enum class Tipo { Futura, Normal, NormalAposFutura };
  explicit CadastrarNFe(const QString &idVenda, const QStringList &items, const Tipo tipo, QWidget *parent);
  ~CadastrarNFe();

private:
  // attributes
  const Tipo tipo;
  const QString idVenda;
  QDataWidgetMapper mapper;
  QSqlQuery queryCliente;
  QSqlQuery queryEndereco;
  QSqlQuery queryIBGEDest;
  QSqlQuery queryIBGEEmit;
  QSqlQuery queryLojaEnd;
  QSqlQuery queryPartilhaInter;
  QSqlQuery queryPartilhaIntra;
  QString arquivo;
  QString chaveNum;
  QString xml;
  SqlTableModel modelLoja;
  SqlTableModel modelViewProdutoEstoque;
  SqlTableModel modelVenda;
  Ui::CadastrarNFe *ui;
  // methods
  auto alterarCertificado(const QString &text) -> void;
  auto buscarAliquotas() -> bool;
  auto cadastrar(const int &idNFe) -> bool;
  auto calculaCofins() -> void;
  auto calculaDigitoVerificador(QString &chave) -> bool;
  auto calculaIcms() -> void;
  auto calculaPis() -> void;
  auto calculaSt() -> void;
  auto clearStr(const QString &str) const -> QString;
  auto criarChaveAcesso() -> bool;
  auto gerarNota() -> QString;
  auto listarCfop() -> bool;
  auto on_checkBoxFrete_toggled(bool checked) -> void;
  auto on_comboBoxCOFINScst_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxCfop_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxDestinoOperacao_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxICMSModBcSt_currentIndexChanged(const int index) -> void;
  auto on_comboBoxICMSModBc_currentIndexChanged(const int index) -> void;
  auto on_comboBoxICMSOrig_currentIndexChanged(const int index) -> void;
  auto on_comboBoxIPIcst_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxPIScst_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxRegime_currentTextChanged(const QString &text) -> void;
  auto on_comboBoxSituacaoTributaria_currentTextChanged(const QString &text) -> void;
  auto on_doubleSpinBoxCOFINSpcofins_valueChanged(const double) -> void;
  auto on_doubleSpinBoxCOFINSvbc_valueChanged(const double) -> void;
  auto on_doubleSpinBoxCOFINSvcofins_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSpicms_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSpicmsst_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSvbc_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSvbcst_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSvicms_valueChanged(const double) -> void;
  auto on_doubleSpinBoxICMSvicmsst_valueChanged(const double) -> void;
  auto on_doubleSpinBoxPISppis_valueChanged(const double) -> void;
  auto on_doubleSpinBoxPISvbc_valueChanged(const double) -> void;
  auto on_doubleSpinBoxPISvpis_valueChanged(const double) -> void;
  auto on_doubleSpinBoxValorFrete_valueChanged(const double value) -> void;
  auto on_itemBoxCliente_textChanged(const QString &) -> void;
  auto on_itemBoxEnderecoEntrega_textChanged(const QString &) -> void;
  auto on_itemBoxEnderecoFaturamento_textChanged(const QString &) -> void;
  auto on_itemBoxVeiculo_textChanged(const QString &) -> void;
  auto on_pushButtonConsultarCadastro_clicked() -> void;
  auto on_pushButtonEnviarNFE_clicked() -> void;
  auto on_tableItens_clicked(const QModelIndex &index) -> void;
  auto on_tableItens_dataChanged(const QModelIndex index) -> void;
  auto preCadastrarNota() -> std::optional<int>;
  auto preencherNumeroNFe() -> bool;
  auto prepararNFe(const QStringList &items) -> void;
  auto processarResposta(const QString &resposta, const QString &filePath, const int &idNFe, ACBr &acbrRemoto) -> bool;
  auto removerNota(const int idNFe) -> void;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto updateComplemento() -> void;
  auto updateTotais() -> void;
  auto validar() -> bool;
  auto writeDestinatario(QTextStream &stream) const -> void;
  auto writeEmitente(QTextStream &stream) const -> void;
  auto writeIdentificacao(QTextStream &stream) -> void;
  auto writePagamento(QTextStream &stream) -> void;
  auto writeProduto(QTextStream &stream) const -> void;
  auto writeTotal(QTextStream &stream) const -> void;
  auto writeTransportadora(QTextStream &stream) const -> void;
  auto writeVolume(QTextStream &stream) const -> void;
};
