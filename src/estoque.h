#ifndef ESTOQUE_H
#define ESTOQUE_H

#include "dialog.h"
#include "sqlrelationaltablemodel.h"

namespace Ui {
class Estoque;
}

class Estoque final : public Dialog {
  Q_OBJECT

public:
  // REFAC: turn showWindow into a enum
  Estoque(QString idEstoque, const bool showWindow = true, QWidget *parent = nullptr);
  ~Estoque();
  auto criarConsumo(const int idVendaProduto, const double quant = 0) -> bool;

private:
  // attributes
  const QString idEstoque;
  SqlRelationalTableModel model;
  SqlRelationalTableModel modelConsumo;
  SqlRelationalTableModel modelViewConsumo;
  SqlRelationalTableModel modelCompra;
  Ui::Estoque *ui;

  enum class FieldColors {
    White = 0,     // Não processado
    Green = 1,     // Ok
    Yellow = 2,    // Quant difere
    Red = 3,       // Não encontrado
    DarkGreen = 4, // Consumo
    Cyan = 5       // Devolução
  };

  // methods
  auto atualizaQuantEstoque(const int idVendaProduto) -> bool;
  auto calcularRestante() -> void;
  auto exibirNota() -> void;
  auto on_pushButtonExibirNfe_clicked() -> void;
  auto on_tableConsumo_entered(const QModelIndex) -> void;
  auto on_tableEstoque_activated(const QModelIndex &) -> void;
  auto on_tableEstoque_entered(const QModelIndex) -> void;
  auto quebrarCompra(const int idVendaProduto, const double quant) -> bool;
  auto setupTables() -> void;
  auto viewRegisterById(const bool showWindow) -> bool;
};

#endif // ESTOQUE_H
