#ifndef IMPORTAPRODUTOS_H
#define IMPORTAPRODUTOS_H

#include <QDialog>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QSqlRelationalTableModel>

#include "importaprodutosproxy.h"
#include "editablesqlmodel.h"

namespace Ui {
  class ImportaProdutos;
}

class ImportaProdutos : public QDialog {
    Q_OBJECT

  public:
    explicit ImportaProdutos(QWidget *parent = 0);
    ~ImportaProdutos();
    void consistenciaDados();
    bool readFile();
    bool readValidade();
    bool verificaTabela();
    int buscarCadastrarFornecedor(QString fornecedor);
    void atualizaCamposProduto(QSqlQuery &produto, QString idProduto);
    void cadastraFornecedores(QSqlQuery &query);
    void cadastraProduto();
    void contaProdutos();
    void expiraPrecosAntigos(QSqlQuery produto, QString idProduto);
    void guardaNovoPrecoValidade(QSqlQuery &produto, QString idProduto);
    void importar();
    void leituraProduto(QSqlQuery &queryProd);
    void marcaProdutoNaoDescontinuado(QSqlQuery &produto, QString idProduto);
    void marcaTodosProdutosDescontinuados();
    void mostraApenasEstesFornecedores();
    void setModelAndTable();
    void setProgressDialog();
    void TestImportacao();
    void verificaSeProdutoJaCadastrado(QSqlQuery &produto);
    void pintarCamposForaDoPadrao(int row);
    void setVariantMap();

    enum FieldColors {
      White = 0,  // no change
      Green = 1,  // new value
      Yellow = 2, // value changed
      Gray = 3,   // wrong value but accepted
      Red = 4     // wrong value, must be fixed
    };

    FieldColors colors;

  private slots:
    void on_pushButtonCancelar_clicked();
    void on_pushButtonSalvar_clicked();
    void on_checkBoxRepresentacao_clicked(bool checked);

  private:
    // attributes
    Ui::ImportaProdutos *ui;
    EditableSqlModel model;
    QProgressDialog *progressDialog;
    QString file, ids;
    int validade;
    QMap<QString, int> fornecedores;
    QSqlDatabase db;
    QVariantMap variantMap;
    // methods
    void importarTabela();
};

#endif // IMPORTAPRODUTOS_H
