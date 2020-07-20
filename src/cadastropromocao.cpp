#include "cadastropromocao.h"
#include "ui_cadastropromocao.h"

#include "application.h"
#include "searchdialogproxymodel.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

CadastroPromocao::CadastroPromocao(QWidget *parent) : QDialog(parent), ui(new Ui::CadastroPromocao) {
  ui->setupUi(this);

  setWindowTitle("Gerenciar promoção");
  setWindowFlags(Qt::Window);

  ui->itemBoxFornecedor->setSearchDialog(SearchDialog::fornecedor(this));

  ui->dateEditValidade->setDate(QDate::currentDate());

  setupTables();

  connect(ui->itemBoxFornecedor, &ItemBox::textChanged, this, &CadastroPromocao::on_itemBoxFornecedor_textChanged);
  connect(ui->pushButtonCadastrar, &QPushButton::clicked, this, &CadastroPromocao::on_pushButtonCadastrar_clicked);
}

CadastroPromocao::~CadastroPromocao() { delete ui; }

void CadastroPromocao::setupTables() {
  model.setTable("view_produto");

  model.setFilter("estoque = TRUE AND descontinuado = FALSE AND desativado = FALSE");

  if (not model.select()) { return; }

  model.setHeaderData("fornecedor", "Fornecedor");
  model.setHeaderData("statusEstoque", "Estoque");
  model.setHeaderData("descricao", "Descrição");
  model.setHeaderData("estoqueRestante", "Estoque Disp.");
  model.setHeaderData("estoqueCaixa", "Estoque Cx.");
  model.setHeaderData("lote", "Lote");
  model.setHeaderData("un", "Un.");
  model.setHeaderData("un2", "Un.2");
  model.setHeaderData("colecao", "Coleção");
  model.setHeaderData("tipo", "Tipo");
  model.setHeaderData("minimo", "Mínimo");
  model.setHeaderData("multiplo", "Múltiplo");
  model.setHeaderData("m2cx", "M/Cx.");
  model.setHeaderData("pccx", "Pç./Cx.");
  model.setHeaderData("kgcx", "Kg./Cx.");
  model.setHeaderData("formComercial", "Form. Com.");
  model.setHeaderData("codComercial", "Cód. Com.");
  model.setHeaderData("precoVenda", "R$");
  model.setHeaderData("validade", "Validade");
  model.setHeaderData("ui", "UI");

  model.proxyModel = new SearchDialogProxyModel(&model, this);

  ui->tableView->setModel(&model);

  ui->tableView->hideColumn("idProduto");
  ui->tableView->hideColumn("estoque");
  ui->tableView->hideColumn("promocao");
  ui->tableView->hideColumn("descontinuado");
  ui->tableView->hideColumn("desativado");
  ui->tableView->hideColumn("representacao");

  for (int column = 0, columnCount = model.columnCount(); column < columnCount; ++column) {
    if (model.record().fieldName(column).endsWith("Upd")) { ui->tableView->setColumnHidden(column, true); }
  }
}

void CadastroPromocao::on_itemBoxFornecedor_textChanged(const QString &text) {
  model.setFilter("fornecedor = '" + text + "' AND estoque = TRUE AND descontinuado = FALSE AND desativado = FALSE");

  if (not model.select()) { return; }
}

void CadastroPromocao::on_radioButtonTodos_toggled(bool checked) {
  if (not checked) { return; }

  const QString fornecedor = ui->itemBoxFornecedor->text().isEmpty() ? "" : " AND fornecedor = '" + ui->itemBoxFornecedor->text() + "'";

  model.setFilter("estoque = TRUE AND descontinuado = FALSE AND desativado = FALSE" + fornecedor);

  if (not model.select()) { return; }
}

void CadastroPromocao::on_radioButtonStaccatoOFF_toggled(bool checked) {
  if (not checked) { return; }

  const QString fornecedor = ui->itemBoxFornecedor->text().isEmpty() ? "" : " AND fornecedor = '" + ui->itemBoxFornecedor->text() + "'";

  model.setFilter("estoque = TRUE AND promocao = 2 AND descontinuado = FALSE AND desativado = FALSE" + fornecedor);

  if (not model.select()) { return; }
}

void CadastroPromocao::on_radioButtonEstoque_toggled(bool checked) {
  if (not checked) { return; }

  const QString fornecedor = ui->itemBoxFornecedor->text().isEmpty() ? "" : " AND fornecedor = '" + ui->itemBoxFornecedor->text() + "'";

  model.setFilter("estoque = TRUE AND promocao = 0 AND descontinuado = FALSE AND desativado = FALSE" + fornecedor);

  if (not model.select()) { return; }
}

void CadastroPromocao::on_pushButtonCadastrar_clicked() {
  auto list = ui->tableView->selectionModel()->selectedRows();

  if (list.isEmpty()) { return qApp->enqueueError("Nenhuma linha selecionada!", this); }

  if (ui->doubleSpinBoxDesconto->value() == 0) { return qApp->enqueueError("Selecione um desconto!", this); }

  for (auto index : list) {
    if (model.data(index.row(), "promocao").toInt() == 2) { return qApp->enqueueError("Linha com promoção selecionada!", this); }
  }

  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja cadastrar promoção?", QMessageBox::Yes | QMessageBox::Cancel, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cadastrar");
  msgBox.setButtonText(QMessageBox::Cancel, "Cancelar");

  const int escolha = msgBox.exec();

  if (escolha == QMessageBox::Cancel) { return; }

  //--------------------------------------------

  for (auto index : list) {
    const QString idProduto = model.data(index.row(), "idProduto").toString();

    QSqlQuery query;

    if (not query.exec("UPDATE produto SET oldPrecoVenda = precoVenda, precoVenda = precoVenda * " + QString::number(1 - (ui->doubleSpinBoxDesconto->value() / 100)) +
                       ", promocao = 2, descricao = CONCAT(descricao, ' (PROMOÇÃO STACCATO OFF " + QString::number(ui->doubleSpinBoxDesconto->value()) + "%)'), validade = '" +
                       ui->dateEditValidade->date().toString("yyyy-MM-dd") + "' WHERE idProduto = " + idProduto)) {
      return qApp->enqueueError("Erro alterando estoque: " + query.lastError().text(), this);
    }
  }

  if (not model.select()) { return; }
  qApp->enqueueInformation("Dados salvos com sucesso!", this);
}

void CadastroPromocao::on_pushButtonDescadastrar_clicked() {
  auto list = ui->tableView->selectionModel()->selectedRows();

  if (list.isEmpty()) { return qApp->enqueueError("Nenhuma linha selecionada!", this); }

  for (auto index : list) {
    if (model.data(index.row(), "promocao").toInt() != 2) { return qApp->enqueueError("Linha sem promoção selecionada!", this); }
  }

  QMessageBox msgBox(QMessageBox::Question, "Atenção!", "Tem certeza que deseja descadastrar promoção?", QMessageBox::Yes | QMessageBox::Cancel, this);
  msgBox.setButtonText(QMessageBox::Yes, "Descadastrar");
  msgBox.setButtonText(QMessageBox::Cancel, "Cancelar");

  const int escolha = msgBox.exec();

  if (escolha == QMessageBox::Cancel) { return; }

  //--------------------------------------------

  for (auto index : list) {
    const QString idProduto = model.data(index.row(), "idProduto").toString();

    QSqlQuery query;

    if (not query.exec(
            "UPDATE produto SET precoVenda = oldPrecoVenda, oldPrecoVenda = NULL, promocao = 0, descricao = LEFT(descricao, CHAR_LENGTH(descricao) - 28), validade = NULL WHERE idProduto = " +
            idProduto)) {
      return qApp->enqueueError("Erro alterando estoque: " + query.lastError().text(), this);
    }
  }

  if (not model.select()) { return; }
  qApp->enqueueInformation("Dados salvos com sucesso!", this);
}
