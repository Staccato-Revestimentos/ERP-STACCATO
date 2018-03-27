#ifndef WIDGETFINANCEIROFLUXOCAIXA_H
#define WIDGETFINANCEIROFLUXOCAIXA_H

#include "sqlquerymodel.h"
#include "sqlrelationaltablemodel.h"
#include "widget.h"

namespace Ui {
class WidgetFinanceiroFluxoCaixa;
}

class WidgetFinanceiroFluxoCaixa final : public Widget {
  Q_OBJECT

public:
  explicit WidgetFinanceiroFluxoCaixa(QWidget *parent = nullptr);
  ~WidgetFinanceiroFluxoCaixa();
  auto updateTables() -> bool;

private:
  // attributes
  bool isReady = false;
  SqlQueryModel modelCaixa;
  SqlQueryModel modelCaixa2;
  SqlQueryModel modelFuturo;
  Ui::WidgetFinanceiroFluxoCaixa *ui;
  // methods
  auto montaFiltro() -> bool;
  auto on_groupBoxCaixa1_toggled(const bool checked) -> void;
  auto on_groupBoxCaixa2_toggled(const bool checked) -> void;
  auto on_tableCaixa2_activated(const QModelIndex &index) -> void;
  auto on_tableCaixa2_entered(const QModelIndex &) -> void;
  auto on_tableCaixa_activated(const QModelIndex &index) -> void;
  auto on_tableCaixa_entered(const QModelIndex &) -> void;
};

#endif // WIDGETFINANCEIROFLUXOCAIXA_H
