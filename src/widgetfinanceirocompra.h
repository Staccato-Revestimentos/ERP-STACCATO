#ifndef WIDGETFINANCEIROCOMPRA_H
#define WIDGETFINANCEIROCOMPRA_H

#include <QWidget>

#include "sqlrelationaltablemodel.h"

namespace Ui {
class WidgetFinanceiroCompra;
}

class WidgetFinanceiroCompra : public QWidget {
  Q_OBJECT

public:
  explicit WidgetFinanceiroCompra(QWidget *parent = 0);
  ~WidgetFinanceiroCompra();
  bool updateTables();

signals:
  void errorSignal(const QString &error);
  void transactionEnded();
  void transactionStarted();

private slots:
  void on_lineEditBusca_textChanged(const QString &text);
  void on_table_activated(const QModelIndex &index);
  void on_table_entered(const QModelIndex &);

private:
  // attributes
  SqlRelationalTableModel model;
  Ui::WidgetFinanceiroCompra *ui;
  // methods
  void setupTables();
};

#endif // WIDGETFINANCEIROCOMPRA_H
