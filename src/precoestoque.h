#ifndef PRECOESTOQUE_H
#define PRECOESTOQUE_H

#include <QDialog>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class PrecoEstoque;
}

class PrecoEstoque final : public QDialog {
  Q_OBJECT

public:
  explicit PrecoEstoque(QWidget *parent = nullptr);
  ~PrecoEstoque();
  auto show() -> void;

private:
  // attributes
  SqlRelationalTableModel modelProduto;
  Ui::PrecoEstoque *ui;
  // methods
  auto on_lineEditBusca_textChanged(const QString &text) -> void;
  auto on_pushButtonCancelar_clicked() -> void;
  auto on_pushButtonSalvar_clicked() -> void;
  auto setupTables() -> void;
};

#endif // PRECOESTOQUE_H
