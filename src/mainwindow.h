#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

public slots:
  void updateTables();

private slots:
  void on_actionCadastrarCliente_triggered();
  void on_actionCadastrarFornecedor_triggered();
  void on_actionCadastrarProdutos_triggered();
  void on_actionCadastrarProfissional_triggered();
  void on_actionCadastrarUsuario_triggered();
  void on_actionCalculadora_triggered();
  void on_actionClaro_triggered();
  void on_actionConfiguracoes_triggered();
  void on_actionCriarOrcamento_triggered();
  void on_actionEscuro_triggered();
  void on_actionEstoque_triggered();
  void on_actionGerenciar_Lojas_triggered();
  void on_actionGerenciar_preco_estoque_triggered();
  void on_actionGerenciar_Transportadoras_triggered();
  void on_actionProdutos_triggered();
  void on_actionPromocao_triggered();
  void on_actionSobre_triggered();
  void on_tabWidget_currentChanged(const int);

private:
  // attributes
  Ui::MainWindow *ui;
  // methods
  bool event(QEvent *event);
  void gerarEnviarRelatorio();
  void setConnections();
};

#endif // MAINWINDOW_H
