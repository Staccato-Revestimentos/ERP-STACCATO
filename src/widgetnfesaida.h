#ifndef WIDGETNFESAIDA_H
#define WIDGETNFESAIDA_H

#include <QWidget>

#include "sqltablemodel.h"

namespace Ui {
class WidgetNfeSaida;
}

class WidgetNfeSaida : public QWidget {
  Q_OBJECT

public:
  explicit WidgetNfeSaida(QWidget *parent = 0);
  ~WidgetNfeSaida();
  bool updateTables();

signals:
  void errorSignal(const QString &error);

private slots:
  void on_lineEditBusca_textChanged(const QString &text);
  void on_pushButtonCancelarNFe_clicked();
  void on_pushButtonExportar_clicked();
  void on_pushButtonRelatorio_clicked();
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlTableModel model;
  Ui::WidgetNfeSaida *ui;
  // methods
  void setupTables();
};

#endif // WIDGETNFESAIDA_H
