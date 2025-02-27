#pragma once

#include "registeraddressdialog.h"
#include "searchdialog.h"

namespace Ui {
class CadastroLoja;
}

class CadastroLoja final : public RegisterAddressDialog {
  Q_OBJECT

public:
  explicit CadastroLoja(QWidget *parent);
  ~CadastroLoja();

private:
  // attributes
  int currentRowConta = -1;
  // TODO: make backup for Pagamentos/Taxas records
  QDataWidgetMapper mapperConta;
  QList<QSqlRecord> backupConta;
  SearchDialog *const sdLoja = SearchDialog::loja(this);
  SqlTableModel modelConta;
  Ui::CadastroLoja *ui;
  // methods
  auto cadastrar() -> void final;
  auto cadastrarConta(const Tipo tipoConta = Tipo::Cadastrar) -> void;
  auto cadastrarEndereco(const Tipo tipoEndereco = Tipo::Cadastrar) -> void;
  auto clearConta() -> void;
  auto clearEndereco() -> void;
  auto clearFields() -> void final;
  auto connectLineEditsToDirty() -> void final;
  auto newRegister() -> bool final;
  auto novaConta() -> void;
  auto novoEndereco() -> void;
  auto on_checkBoxMostrarInativosConta_clicked(const bool checked) -> void;
  auto on_checkBoxMostrarInativos_clicked(const bool checked) -> void;
  auto on_lineEditCEP_textChanged(const QString &cep) -> void;
  auto on_lineEditCNPJ_textEdited(const QString &text) -> void;
  auto on_pushButtonAdicionarConta_clicked() -> void;
  auto on_pushButtonAdicionarEnd_clicked() -> void;
  auto on_pushButtonAtualizarConta_clicked() -> void;
  auto on_pushButtonAtualizarEnd_clicked() -> void;
  auto on_pushButtonAtualizar_clicked() -> void;
  auto on_pushButtonBuscar_clicked() -> void;
  auto on_pushButtonCadastrar_clicked() -> void;
  auto on_pushButtonDesativarConta_clicked() -> void;
  auto on_pushButtonDesativarEnd_clicked() -> void;
  auto on_pushButtonDesativar_clicked() -> void;
  auto on_pushButtonNovoCad_clicked() -> void;
  auto on_tableConta_selectionChanged() -> void;
  auto on_tableEndereco_selectionChanged() -> void;
  auto registerMode() -> void final;
  auto savingProcedures() -> void final;
  auto setConnections() -> void;
  auto setupMapper() -> void final;
  auto setupTables() -> void;
  auto setupUi() -> void;
  auto successMessage() -> void final;
  auto updateMode() -> void final;
  auto verificaEndereco() -> void;
  auto verifyFields() -> void final;
  auto viewRegister() -> bool final;
};
