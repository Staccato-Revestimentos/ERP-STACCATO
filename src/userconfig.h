#ifndef USERCONFIG_H
#define USERCONFIG_H

#include <QDialog>

namespace Ui {
class UserConfig;
}

class UserConfig : public QDialog {
  Q_OBJECT

public:
  explicit UserConfig(QWidget *parent = 0);
  ~UserConfig();

private slots:
  void on_pushButtonACBrEntrada_clicked();
  void on_pushButtonACBrSaida_clicked();
  void on_pushButtonACBrXML_clicked();
  void on_pushButtonAlterarDados_clicked();
  void on_pushButtonSalvar_clicked();
  void on_pushButtonOrcamentosFolder_clicked();
  void on_pushButtonVendasFolder_clicked();

private:
  Ui::UserConfig *ui;
};

#endif // USERCONFIG_H
