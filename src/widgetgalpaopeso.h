#pragma once

#include "chartview.h"
#include "sqltablemodel.h"

#include <QWidget>
#include <QtCharts>

namespace Ui {
class WidgetGalpaoPeso;
}

class WidgetGalpaoPeso final : public QWidget {
  Q_OBJECT

public:
  explicit WidgetGalpaoPeso(QWidget *parent);
  ~WidgetGalpaoPeso() final;

  auto resetTables() -> void;
  auto updateTables() -> void;

private:
  // attributes
  bool isSet = false;
  ChartView *chartView = nullptr;
  QChart chart;
  QDateTimeAxis axisX;
  QLineSeries series;
  QProgressDialog progressDialog;
  QValueAxis axisY;
  SqlTableModel model;
  Ui::WidgetGalpaoPeso *ui;
  // methods
  auto on_pushButtonAtualizar_clicked() -> void;
  auto setChart() -> void;
  auto setConnections() -> void;
  auto setProgressDialog() -> void;
  auto setupTables() -> void;
  auto updateChart() -> void;
};
