#ifndef CADASTROTRANSPORTADORA_H
#define CADASTROTRANSPORTADORA_H

#include "registeraddressdialog.h"
#include "searchdialog.h"

namespace Ui {
class CadastroTransportadora;
}

class CadastroTransportadora final : public RegisterAddressDialog {
  Q_OBJECT

public:
  explicit CadastroTransportadora(QWidget *parent = nullptr);
  ~CadastroTransportadora();

private:
  // attributes
  QDataWidgetMapper mapperVeiculo;
  SearchDialog *sdTransportadora;
  SqlRelationalTableModel modelVeiculo;
  Ui::CadastroTransportadora *ui;
  // methods
  auto cadastrar() -> bool final;
  auto cadastrarEndereco(const bool isUpdate = false) -> bool;
  auto cadastrarVeiculo(const bool isUpdate = false) -> bool;
  auto clearEndereco() -> void;
  auto clearFields() -> void final;
  auto clearVeiculo() -> void;
  auto novoEndereco() -> void;
  auto novoVeiculo() -> void;
  auto on_checkBoxMostrarInativosVeiculo_toggled(bool checked) -> void;
  auto on_checkBoxMostrarInativos_clicked(const bool checked) -> void;
  auto on_lineEditCEP_textChanged(const QString &cep) -> void;
  auto on_lineEditCNPJ_textEdited(const QString &text) -> void;
  auto on_pushButtonAdicionarEnd_clicked() -> void;
  auto on_pushButtonAdicionarVeiculo_clicked() -> void;
  auto on_pushButtonAtualizarEnd_clicked() -> void;
  auto on_pushButtonAtualizarVeiculo_clicked() -> void;
  auto on_pushButtonAtualizar_clicked() -> void;
  auto on_pushButtonBuscar_clicked() -> void;
  auto on_pushButtonCadastrar_clicked() -> void;
  auto on_pushButtonEndLimpar_clicked() -> void;
  auto on_pushButtonNovoCad_clicked() -> void;
  auto on_pushButtonRemoverEnd_clicked() -> void;
  auto on_pushButtonRemoverVeiculo_clicked() -> void;
  auto on_pushButtonRemover_clicked() -> void;
  auto on_pushButtonVeiculoLimpar_clicked() -> void;
  auto on_tableEndereco_clicked(const QModelIndex &index) -> void;
  auto on_tableEndereco_entered(const QModelIndex &) -> void;
  auto on_tableVeiculo_clicked(const QModelIndex &index) -> void;
  auto on_tableVeiculo_entered(const QModelIndex &) -> void;
  auto registerMode() -> void final;
  auto savingProcedures() -> bool final;
  auto setupMapper() -> void final;
  auto setupTables() -> void;
  auto setupUi() -> void;
  auto successMessage() -> void final;
  auto updateMode() -> void final;
  auto verifyFields() -> bool final;
  auto viewRegister() -> bool final;
};

#endif // CADASTROTRANSPORTADORA_H
