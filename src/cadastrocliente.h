#ifndef CADASTROCLIENTE_H
#define CADASTROCLIENTE_H

#include "registeraddressdialog.h"

namespace Ui {
  class CadastroCliente;
}

class CadastroCliente : public RegisterAddressDialog {
    Q_OBJECT

  public:
    explicit CadastroCliente(QWidget *parent = 0);
    ~CadastroCliente();

  private slots:
    void on_checkBoxMostrarInativos_clicked(const bool &checked);
    void on_lineEditCEP_textChanged(const QString &cep);
    void on_lineEditCNPJ_textEdited(const QString &text);
    void on_lineEditContatoCPF_textEdited(const QString &text);
    void on_lineEditCPF_textEdited(const QString &text);
    void on_pushButtonAdicionarEnd_clicked();
    void on_pushButtonAtualizar_clicked();
    void on_pushButtonAtualizarEnd_clicked();
    void on_pushButtonBuscar_clicked();
    void on_pushButtonCadastrar_clicked();
    void on_pushButtonEndLimpar_clicked();
    void on_pushButtonNovoCad_clicked();
    void on_pushButtonRemover_clicked();
    void on_pushButtonRemoverEnd_clicked();
    void on_radioButtonPF_toggled(const bool &checked);
    void on_tableEndereco_clicked(const QModelIndex &index);

  private:
    // attributes
    Ui::CadastroCliente *ui;
    QString tipoPFPJ;
    // methods
    bool cadastrarEndereco(const bool &isUpdate);
    bool verifyFields(const bool &isUpdate);
    virtual bool save(const bool &isUpdate = false) override;
    virtual bool savingProcedures() override;
    virtual bool verifyFields() override;
    virtual bool viewRegister(const QModelIndex &index) override;
    virtual void clearFields() override;
    virtual void registerMode() override;
    virtual void setupMapper() override;
    virtual void successMessage() override;
    virtual void updateMode() override;
    void clearEndereco();
    void novoEndereco();
    void setupTables();
    void setupUi();
};

#endif // CADASTROCLIENTE_H
