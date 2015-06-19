#include <QCheckBox>

#include "usersession.h"
#include "cadastrousuario.h"
#include "ui_cadastrousuario.h"
#include "searchdialog.h"

CadastroUsuario::CadastroUsuario(QWidget *parent)
  : RegisterDialog("Usuario", "idUsuario", parent), ui(new Ui::CadastroUsuario) {
  ui->setupUi(this);

  setupTablePermissoes();
  fillCombobox();

  ui->lineEditSigla->setInputMask(">AAA");
  ui->tablePermissoes->setEnabled(false);
  ui->tablePermissoes->setToolTip("Função indisponível nesta versão!");
  ui->tablePermissoes->resizeColumnsToContents();

  ui->tabWidget->setTabEnabled(1, false);

  setupMapper();
  newRegister();

  foreach (QLineEdit *line, findChildren<QLineEdit *>()) {
    connect(line, &QLineEdit::textEdited, this, &RegisterDialog::marcarDirty);
  }
}

CadastroUsuario::~CadastroUsuario() { delete ui; }

void CadastroUsuario::setupTablePermissoes() {
  ui->tablePermissoes->resizeColumnsToContents();
  ui->tablePermissoes->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->tablePermissoes->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  for (int i = 0; i < ui->tablePermissoes->rowCount(); ++i) {
    for (int j = 0; j < ui->tablePermissoes->columnCount(); ++j) {
      QWidget *widget = new QWidget();
      QCheckBox *checkBox = new QCheckBox();
      QHBoxLayout *layout = new QHBoxLayout(widget);
      layout->addWidget(checkBox);
      layout->setAlignment(Qt::AlignCenter);
      layout->setContentsMargins(0, 0, 0, 0);
      widget->setLayout(layout);
      ui->tablePermissoes->setCellWidget(i, j, widget);
    }
  }
}

bool CadastroUsuario::verifyFields(int row) {
  Q_UNUSED(row);

  if (not RegisterDialog::verifyFields({ui->lineEditNome, ui->lineEditUser, ui->lineEditSigla, ui->lineEditPasswd})) {
    return false;
  }

  if (ui->lineEditPasswd->text() != ui->lineEditPasswd_2->text()) {
    ui->lineEditPasswd->setFocus();
    QMessageBox::warning(this, "Atenção!", "As senhas não batem!", QMessageBox::Ok, QMessageBox::NoButton);
    return false;
  }

  return true;
}

void CadastroUsuario::clearFields() { RegisterDialog::clearFields(); }

void CadastroUsuario::setupMapper() {
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditUser, "user");
  addMapping(ui->comboBoxTipo, "tipo");
  addMapping(ui->comboBoxLoja, "idLoja", "currentValue");
  addMapping(ui->lineEditSigla, "sigla");
}

void CadastroUsuario::registerMode() {
  ui->pushButtonCadastrar->show();
  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroUsuario::updateMode() {
  ui->pushButtonCadastrar->hide();
  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

bool CadastroUsuario::savingProcedures(int row) {
  setData(row, "nome", ui->lineEditNome->text());
  setData(row, "idLoja", ui->comboBoxLoja->getCurrentValue());
  setData(row, "tipo", ui->comboBoxTipo->currentText());
  setData(row, "user", ui->lineEditUser->text());
  setData(row, "sigla", ui->lineEditSigla->text());
  setData(row, "user", ui->lineEditUser->text());

  if (ui->lineEditPasswd->text() != "********") {
    QSqlQuery query;
    query.prepare("SELECT PASSWORD(:password)");
    query.bindValue(":password", ui->lineEditPasswd->text());
    query.first();

    setData(row, "passwd", query.value(0));
  }

  return true;
}

bool CadastroUsuario::viewRegister(QModelIndex index) {
  if (not RegisterDialog::viewRegister(index)) {
    return false;
  }

  ui->lineEditPasswd->setText("********");
  ui->lineEditPasswd_2->setText("********");

  return true;
}

void CadastroUsuario::fillCombobox() {
  QSqlQuery query("SELECT * FROM Loja");

  while (query.next()) {
    ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja"));
  }

  ui->comboBoxLoja->setCurrentValue(UserSession::getLoja());
}

void CadastroUsuario::on_pushButtonCadastrar_clicked() { save(); }

void CadastroUsuario::on_pushButtonAtualizar_clicked() { update(); }

void CadastroUsuario::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroUsuario::on_pushButtonRemover_clicked() { remove(); }

void CadastroUsuario::on_pushButtonCancelar_clicked() { close(); }

void CadastroUsuario::on_pushButtonBuscar_clicked() {
  SearchDialog *sdUsuario = SearchDialog::usuario(this);
  connect(sdUsuario, &SearchDialog::itemSelected, this, &CadastroUsuario::changeItem);
  sdUsuario->show();
}

void CadastroUsuario::show() {
  QWidget::show();
  adjustSize();
}
