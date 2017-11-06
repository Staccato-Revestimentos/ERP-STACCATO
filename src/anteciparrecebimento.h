#ifndef ANTECIPARRECEBIMENTO_H
#define ANTECIPARRECEBIMENTO_H

#include <QDialog>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class AnteciparRecebimento;
}

class AnteciparRecebimento : public QDialog {
  Q_OBJECT

public:
  explicit AnteciparRecebimento(QWidget *parent = 0);
  ~AnteciparRecebimento();

private slots:
  void on_comboBox_currentTextChanged(const QString &text);
  void on_doubleSpinBoxValorPresente_valueChanged(double);
  void on_pushButtonGerar_clicked();
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  // REFAC: 0refactor those out
  bool isBlockedPresente = false;
  bool isBlockedMes = false;
  SqlRelationalTableModel model;
  Ui::AnteciparRecebimento *ui;
  // methods
  void calcularTotais();
  void setupTables();
};

#endif // ANTECIPARRECEBIMENTO_H
