#pragma once

#include <QStyledItemDelegate>

class ReaisDelegate final : public QStyledItemDelegate {
  Q_OBJECT

public:
  explicit ReaisDelegate(const int decimais, const bool readOnly, QObject *parent);
  explicit ReaisDelegate(QObject *parent);
  ~ReaisDelegate() = default;

private:
  // attributes
  bool const readOnly;
  int const decimais;
  // methods
  auto createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const -> QWidget * override;
  auto displayText(const QVariant &value, const QLocale &locale) const -> QString override;
};
