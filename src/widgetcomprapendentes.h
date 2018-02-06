#ifndef WIDGETCOMPRAPENDENTES_H
#define WIDGETCOMPRAPENDENTES_H

#include "sqlrelationaltablemodel.h"
#include "widget.h"

namespace Ui {
class WidgetCompraPendentes;
}

class WidgetCompraPendentes final : public Widget {
  Q_OBJECT

public:
  explicit WidgetCompraPendentes(QWidget *parent = nullptr);
  ~WidgetCompraPendentes();
  auto updateTables() -> bool;

private:
  // attributes
  SqlRelationalTableModel model;
  Ui::WidgetCompraPendentes *ui;
  // methods
  auto insere(const QDate &dataPrevista) -> bool;
  auto makeConnections() -> void;
  auto montaFiltro() -> void;
  auto on_doubleSpinBoxQuantAvulsoCaixas_valueChanged(const double value) -> void;
  auto on_doubleSpinBoxQuantAvulso_valueChanged(const double value) -> void;
  auto on_groupBoxStatus_toggled(bool enabled) -> void;
  auto on_pushButtonComprarAvulso_clicked() -> void;
  auto on_pushButtonExcel_clicked() -> void;
  auto on_pushButtonPDF_clicked() -> void;
  auto on_table_activated(const QModelIndex &index) -> void;
  auto on_table_entered(const QModelIndex &) -> void;
  auto setarDadosAvulso() -> void;
  auto setupTables() -> void;
};

#endif // WIDGETCOMPRAPENDENTES_H
