#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlRelationalTableModel>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool dbConnect();
    QString getHostname() const;
    void setHostname(const QString &value);
    QString getUsername() const;
    void setUsername(const QString &value);
    QString getPassword() const;
    void setPassword(const QString &value);
    QString getPort() const;
    void setPort(const QString &value);
    void readSettings();
    void showMaximized();

    bool TestCadastroClienteIncompleto();
    bool TestInitDB();
    bool TestCadastroClienteEndereco();
    bool TestCadastroClienteCompleto();
    void TestImportacao();

  public slots:
    void showError(const QSqlError &err);
    void updateTables();

  private slots:
    void on_actionCadastrarCliente_triggered();
    void on_actionCadastrarFornecedor_triggered();
    void on_actionCadastrarProdutos_triggered();
    void on_actionCadastrarProfissional_triggered();
    void on_actionCadastrarUsuario_triggered();
    void on_actionCriarOrcamento_triggered();
    void on_actionGerenciar_Lojas_triggered();
    void on_actionGerenciar_Transportadoras_triggered();
    void on_actionImportaProdutos_triggered();
    void on_lineEditBuscaContasPagar_textChanged(const QString &text);
    void on_lineEditBuscaContasReceber_textChanged(const QString &text);
    void on_lineEditBuscaEntregas_textChanged(const QString &arg1);
    void on_lineEditBuscaNFe_textChanged(const QString &text);
    void on_lineEditBuscaOrcamentos_textChanged(const QString &text);
    void on_lineEditBuscaProdutosPend_textChanged(const QString &text);
    void on_lineEditBuscaRecebimentos_textChanged(const QString &text);
    void on_lineEditBuscaVendas_textChanged(const QString &text);
    void on_pushButtonCriarOrc_clicked();
    void on_radioButtonContaPagarLimpar_clicked();
    void on_radioButtonContaPagarPago_clicked();
    void on_radioButtonContaPagarPendente_clicked();
    void on_radioButtonContaReceberLimpar_clicked();
    void on_radioButtonContaReceberPendente_clicked();
    void on_radioButtonContaReceberRecebido_clicked();
    void on_radioButtonEntregaEnviado_clicked();
    void on_radioButtonEntregaLimpar_clicked();
    void on_radioButtonEntregaPendente_clicked();
    void on_radioButtonFornAberto_clicked();
    void on_radioButtonFornFechado_clicked();
    void on_radioButtonFornLimpar_clicked();
    void on_radioButtonNFeAutorizado_clicked();
    void on_radioButtonNFeEnviado_clicked();
    void on_radioButtonNFeLimpar_clicked();
    void on_radioButtonOrcExpirado_clicked();
    void on_radioButtonOrcLimpar_clicked();
    void on_radioButtonOrcValido_clicked();
    void on_radioButtonRecebimentoLimpar_clicked();
    void on_radioButtonRecebimentoPendente_clicked();
    void on_radioButtonRecebimentoRecebido_clicked();
    void on_radioButtonVendAberto_clicked();
    void on_radioButtonVendFechado_clicked();
    void on_radioButtonVendLimpar_clicked();
    void on_tableContasPagar_activated(const QModelIndex &index);
    void on_tableContasReceber_activated(const QModelIndex &index);
    void on_tableEntregasCliente_activated(const QModelIndex &index);
    void on_tableNFE_activated(const QModelIndex &index);
    void on_tableOrcamentos_activated(const QModelIndex &index);
    void on_tablePedidosCompra_activated(const QModelIndex &index);
    void on_tableRecebimentosFornecedor_activated(const QModelIndex &index);
    void on_tableVendas_activated(const QModelIndex &index);
    void on_radioButtonOrcProprios_clicked();
    void on_tableEstoque_activated(const QModelIndex &index);
    void on_pushButtonEntradaEstoque_clicked();

  private:
    // attributes
    Ui::MainWindow *ui;
    QSqlTableModel *modelOrcamento, *modelCAPagar, *modelCAReceber, *modelEntregasCliente, *modelRecebimentosForn,
    *modelNFe, *modelEstoque;
    QSqlRelationalTableModel *modelVendas, *modelPedCompra;
    QString hostname = "localhost";
    QString username = "test";
    QString password = "1234";
    QString port = "3306";
    // Methods
    void initializeTables();
    bool event(QEvent *e);
    void darkTheme();
};

#endif // MAINWINDOW_H
