#include "doubledelegate.h"

DoubleDelegate::DoubleDelegate(QObject *parent, const int decimais)
    : QStyledItemDelegate(parent), decimais(decimais) {}

DoubleDelegate::~DoubleDelegate() {}

QString DoubleDelegate::displayText(const QVariant &value, const QLocale &locale) const {
  return value.userType() == QVariant::Double ? QLocale(QLocale::Portuguese).toString(value.toDouble(), 'f', decimais)
                                              : QStyledItemDelegate::displayText(value, locale);
}

// TODO: add point separator 50000 -> 50.000
