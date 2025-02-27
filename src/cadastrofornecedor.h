#pragma once

#include "registeraddressdialog.h"
#include "searchdialog.h"

namespace Ui {
class CadastroFornecedor;
}

class CadastroFornecedor final : public RegisterAddressDialog {
  Q_OBJECT

public:
  // TODO: usar/remover
  //  enum class Especialidade { Revestimentos = 1, Loucas_Metais = 2, Acessorios = 3, Insumos = 4, Servicos = 5 };
  //  Q_ENUM(Especialidade)

  explicit CadastroFornecedor(QWidget *parent);
  ~CadastroFornecedor() final;

private:
  // attributes
  SearchDialog *const sdFornecedor = SearchDialog::fornecedor(this);
  Ui::CadastroFornecedor *ui;
  // methods
  auto ajustarValidade(const int novaValidade) -> void;
  auto cadastrar() -> void final;
  auto cadastrarEndereco(const CadastroFornecedor::Tipo tipoEndereco = Tipo::Cadastrar) -> void;
  auto clearEndereco() -> void;
  auto clearFields() -> void final;
  auto connectLineEditsToDirty() -> void final;
  auto novoEndereco() -> void;
  auto on_checkBoxMostrarInativos_clicked(const bool checked) -> void;
  auto on_lineEditCEP_textChanged(const QString &cep) -> void;
  auto on_lineEditCNPJBancario_textEdited(const QString &text) -> void;
  auto on_lineEditCNPJ_textEdited(const QString &text) -> void;
  auto on_lineEditContatoCPF_textEdited(const QString &text) -> void;
  auto on_pushButtonAdicionarEnd_clicked() -> void;
  auto on_pushButtonAtualizarEnd_clicked() -> void;
  auto on_pushButtonAtualizar_clicked() -> void;
  auto on_pushButtonBuscar_clicked() -> void;
  auto on_pushButtonCadastrar_clicked() -> void;
  auto on_pushButtonDesativarEnd_clicked() -> void;
  auto on_pushButtonDesativar_clicked() -> void;
  auto on_pushButtonNovoCad_clicked() -> void;
  auto on_pushButtonSalvarPrazos_clicked() -> void;
  auto on_pushButtonValidade_clicked() -> void;
  auto on_tabWidget_currentChanged(const int index) -> void;
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
