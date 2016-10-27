#ifndef CONTAS_H
#define CONTAS_H

#include <QDialog>

#include "sqltablemodel.h"

namespace Ui {
class Contas;
}

class Contas : public QDialog {
  Q_OBJECT

public:
  enum Tipo { Pagar, Receber };
  explicit Contas(Tipo tipo, QWidget *parent = 0);
  ~Contas();
  void viewConta(const QString &idPagamento, const QString contraparte);

private slots:
  void on_pushButtonSalvar_clicked();
  void on_tablePendentes_entered(const QModelIndex &);
  void on_tableProcessados_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel modelPendentes;
  SqlTableModel modelProcessados;
  Tipo tipo;
  Ui::Contas *ui;
  //  QString idVenda;
  // methods
  void preencher(const QModelIndex index);
  void setupTables();
};

#endif // CONTAS_H
