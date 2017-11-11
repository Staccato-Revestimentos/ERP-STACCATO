#ifndef BAIXAORCAMENTO_H
#define BAIXAORCAMENTO_H

#include <QDialog>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class BaixaOrcamento;
}

class BaixaOrcamento final : public QDialog {
  Q_OBJECT

public:
  explicit BaixaOrcamento(const QString &idOrcamento, QWidget *parent = 0);
  ~BaixaOrcamento();

private slots:
  void on_pushButtonCancelar_clicked();
  void on_pushButtonSalvar_clicked();

private:
  SqlRelationalTableModel model;
  Ui::BaixaOrcamento *ui;
};

#endif // BAIXAORCAMENTO_H
