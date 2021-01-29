#pragma once

#include "sqltablemodel.h"

#include <QDialog>

namespace Ui {
class AnteciparRecebimento;
}

class AnteciparRecebimento final : public QDialog {
  Q_OBJECT

public:
  explicit AnteciparRecebimento(QWidget *parent);
  ~AnteciparRecebimento();

private:
  // attributes
  SqlTableModel modelContaReceber;
  Ui::AnteciparRecebimento *ui;
  // methods
  auto cadastrar(const QModelIndexList &list) -> void;
  auto calcularTotais() -> void;
  auto fillComboBoxLoja() -> void;
  auto fillComboBoxPagamento() -> void;
  auto montaFiltro() -> void;
  auto on_comboBoxLoja_currentTextChanged(const QString &) -> void;
  auto on_comboBox_currentTextChanged(const QString &) -> void;
  auto on_doubleSpinBoxValorPresente_valueChanged(double) -> void;
  auto on_pushButtonGerar_clicked() -> void;
  auto setConnections() -> void;
  auto setupTables() -> void;
  auto unsetConnections() -> void;
  auto verifyFields(const QModelIndexList &list) -> void;
};
