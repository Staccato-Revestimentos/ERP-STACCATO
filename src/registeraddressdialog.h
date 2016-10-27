#ifndef REGISTERADDRESSDIALOG_H
#define REGISTERADDRESSDIALOG_H

#include "registerdialog.h"

class RegisterAddressDialog : public RegisterDialog {

public:
  explicit RegisterAddressDialog(const QString &table, const QString &primaryKey, QWidget *parent);

public slots:
  bool viewRegisterById(const QVariant &id) override;

protected:
  // attributes
  QDataWidgetMapper mapperEnd;
  SqlTableModel modelEnd;
  // methods
  bool setDataEnd(const QString &key, const QVariant &value);
  int getCodigoUF(QString uf) const;
  virtual bool newRegister() override;
  virtual bool save() override;

private:
  void setupTables(const QString &table);
  bool cadastrar() override;
//  bool isUpdateEnd = false; // TODO: fix
};

#endif // REGISTERADDRESSDIALOG_H
