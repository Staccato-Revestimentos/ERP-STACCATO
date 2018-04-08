#ifndef CADASTRARNFE_H
#define CADASTRARNFE_H

#include <QDataWidgetMapper>
#include <QSqlQuery>
#include <QTextStream>
#include <optional>

#include "dialog.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class CadastrarNFe;
}

class CadastrarNFe final : public Dialog {
  Q_OBJECT

public:
  explicit CadastrarNFe(const QString &idVenda, QWidget *parent = nullptr);
  ~CadastrarNFe();
  auto prepararNFe(const QList<int> &items) -> void;

private:
  // attributes
  const QString idVenda;
  QDataWidgetMapper mapper;
  QSqlQuery queryCliente;
  QSqlQuery queryEndereco;
  QSqlQuery queryIBGE;
  QSqlQuery queryLojaEnd;
  QSqlQuery queryPartilhaInter;
  QSqlQuery queryPartilhaIntra;
  QString arquivo;
  QString chaveNum;
  QString xml;
  SqlRelationalTableModel modelLoja;
  SqlRelationalTableModel modelViewProdutoEstoque;
  SqlRelationalTableModel modelVenda;
  Ui::CadastrarNFe *ui;
  // methods
  auto alterarCertificado(const QString &text) -> void;
  auto cadastrar(const int &idNFe) -> bool;
  auto calculaDigitoVerificador(QString &chave) -> bool;
  auto clearStr(const QString &str) const -> QString;
  auto criarChaveAcesso() -> bool;
  auto gravarNota() -> QString;
  auto listarCfop() -> bool;
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
  auto on_tabWidget_currentChanged(const int index) -> void;
  auto on_tableItens_clicked(const QModelIndex &index) -> void;
  auto on_tableItens_entered(const QModelIndex &) -> void;
  auto preCadastrarNota(const QString &fileName) -> std::optional<int>;
  auto preencherNumeroNFe() -> bool;
  auto processarResposta(const QString &resposta, const QString &fileName, const int &idNFe) -> bool;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto updateImpostos() -> void;
  auto validar() -> bool;
  auto verificarConfiguracaoEmail() -> bool;
  auto writeDestinatario(QTextStream &stream) const -> void;
  auto writeEmitente(QTextStream &stream) const -> void;
  auto writeIdentificacao(QTextStream &stream) const -> void;
  auto writePagamento(QTextStream &stream) -> void;
  auto writeProduto(QTextStream &stream) const -> void;
  auto writeTotal(QTextStream &stream) const -> void;
  auto writeTransportadora(QTextStream &stream) const -> void;
  auto writeVolume(QTextStream &stream) const -> void;
};

#endif // CADASTRARNFE_H
