#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>

class TableView : public QTableView {
  public:
    explicit TableView(QWidget *parent = 0);
    void hideColumn(const QString &column);
    void setItemDelegateForColumn(const QString &column, QAbstractItemDelegate *delegate);
    void setItemDelegateForColumn(const int &column, QAbstractItemDelegate *delegate);

  public slots:
    void sortByColumn(const QString &column);
};

#endif // TABLEVIEW_H
