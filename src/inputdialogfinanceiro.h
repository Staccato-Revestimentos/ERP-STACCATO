#ifndef INPUTDIALOGFINANCEIRO_H
#define INPUTDIALOGFINANCEIRO_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class InputDialogFinanceiro;
}

class InputDialogFinanceiro : public QDialog {
  Q_OBJECT

public:
  enum Type { ConfirmarCompra, Financeiro };

  explicit InputDialogFinanceiro(const Type &type, QWidget *parent = 0);
  ~InputDialogFinanceiro();
  QDateTime getDate() const;
  QDateTime getNextDate() const;
  bool setFilter(const QString &idCompra);

private slots:
  void on_checkBoxMarcarTodos_toggled(bool checked);
  void on_comboBoxPgt1_currentTextChanged(const QString &text);
  void on_comboBoxPgt2_currentTextChanged(const QString &text);
  void on_comboBoxPgt3_currentTextChanged(const QString &text);
  void on_dateEditEvento_dateChanged(const QDate &date);
  void on_doubleSpinBoxAdicionais_valueChanged(double);
  void on_doubleSpinBoxFrete_valueChanged(double);
  void on_doubleSpinBoxPgt1_valueChanged(double);
  void on_doubleSpinBoxPgt2_valueChanged(double);
  void on_doubleSpinBoxPgt3_valueChanged(double);
  void on_pushButtonCorrigirFluxo_clicked();
  void on_pushButtonLimparPag_clicked();
  void on_pushButtonSalvar_clicked();

private:
  // attributes
  const Type type;
  bool representacao;
  SqlTableModel model;
  SqlTableModel modelFluxoCaixa;
  Ui::InputDialogFinanceiro *ui;
  // methods
  bool cadastrar();
  bool verifyFields();
  void calcularTotal();
  void calculoSpinBox1() const;
  void calculoSpinBox2() const;
  void montarFluxoCaixa();
  void resetarPagamentos();
  void setupTables();
  void updateTableData(const QModelIndex &topLeft);
};

#endif // INPUTDIALOGFINANCEIRO_H
