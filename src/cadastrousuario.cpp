#include "cadastrousuario.h"
#include "ui_cadastrousuario.h"

#include "application.h"
#include "checkboxdelegate.h"
#include "file.h"
#include "usersession.h"

#include <QSqlError>
#include <QTransposeProxyModel>

CadastroUsuario::CadastroUsuario(QWidget *parent) : RegisterDialog("usuario", "idUsuario", parent), ui(new Ui::CadastroUsuario) {
  ui->setupUi(this);

  ui->labelEspecialidade->hide();
  ui->comboBoxEspecialidade->hide();

  if (UserSession::tipoUsuario != "ADMINISTRADOR") { ui->table->hide(); }

  connectLineEditsToDirty();
  setupTables();
  fillComboBoxLoja();
  setupMapper();
  newRegister();
  setConnections();
}

CadastroUsuario::~CadastroUsuario() { delete ui; }

void CadastroUsuario::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(sdUsuario, &SearchDialog::itemSelected, this, &CadastroUsuario::viewRegisterById, connectionType);
  connect(ui->comboBoxTipo, &QComboBox::currentTextChanged, this, &CadastroUsuario::on_comboBoxTipo_currentTextChanged, connectionType);
  connect(ui->lineEditUser, &QLineEdit::textEdited, this, &CadastroUsuario::on_lineEditUser_textEdited, connectionType);
  connect(ui->pushButtonAtualizar, &QPushButton::clicked, this, &CadastroUsuario::on_pushButtonAtualizar_clicked, connectionType);
  connect(ui->pushButtonBuscar, &QPushButton::clicked, this, &CadastroUsuario::on_pushButtonBuscar_clicked, connectionType);
  connect(ui->pushButtonCadastrar, &QPushButton::clicked, this, &CadastroUsuario::on_pushButtonCadastrar_clicked, connectionType);
  connect(ui->pushButtonNovoCad, &QPushButton::clicked, this, &CadastroUsuario::on_pushButtonNovoCad_clicked, connectionType);
  connect(ui->pushButtonRemover, &QPushButton::clicked, this, &CadastroUsuario::on_pushButtonRemover_clicked, connectionType);
}

void CadastroUsuario::setupTables() {
  modelPermissoes.setTable("usuario_has_permissao");

  modelPermissoes.setHeaderData("view_tab_orcamento", "Ver Orçamentos?");
  modelPermissoes.setHeaderData("view_tab_venda", "Ver Vendas?");
  modelPermissoes.setHeaderData("view_tab_compra", "Ver Compras?");
  modelPermissoes.setHeaderData("view_tab_logistica", "Ver Logística?");
  modelPermissoes.setHeaderData("view_tab_nfe", "Ver NFe?");
  modelPermissoes.setHeaderData("view_tab_estoque", "Ver Estoque?");
  modelPermissoes.setHeaderData("view_tab_galpao", "Ver Galpão?");
  modelPermissoes.setHeaderData("view_tab_financeiro", "Ver Financeiro?");
  modelPermissoes.setHeaderData("view_tab_relatorio", "Ver Relatório?");
  modelPermissoes.setHeaderData("view_tab_grafico", "Ver Gráfico?");
  modelPermissoes.setHeaderData("view_tab_rh", "Ver RH?");

  auto *transposeProxyModel = new QTransposeProxyModel(this);
  transposeProxyModel->setSourceModel(&modelPermissoes);

  ui->table->setModel(transposeProxyModel);

  ui->table->hideRow(0);                                  // idUsuario
  ui->table->hideRow(ui->table->model()->rowCount() - 1); // lastUpdated
  ui->table->hideRow(ui->table->model()->rowCount() - 2); // created

  for (int row = 1; row < 12; ++row) { ui->table->setItemDelegateForRow(row, new CheckBoxDelegate(this)); }

  ui->table->horizontalHeader()->hide();
}

void CadastroUsuario::modificarUsuario() {
  limitado = true;

  ui->pushButtonBuscar->hide();
  ui->pushButtonNovoCad->hide();
  ui->pushButtonRemover->hide();

  ui->lineEditNome->setDisabled(true);
  ui->lineEditUser->setDisabled(true);
  ui->comboBoxLoja->setDisabled(true);
  ui->comboBoxTipo->setDisabled(true);
}

void CadastroUsuario::verifyFields() { // TODO: deve marcar uma loja?
  const auto children = findChildren<QLineEdit *>(QRegularExpression("lineEdit"));

  for (const auto &line : children) { verifyRequiredField(*line); }

  if (ui->lineEditPasswd->text() != ui->lineEditPasswd_2->text()) { throw RuntimeError("As senhas não batem!"); }
}

void CadastroUsuario::clearFields() { RegisterDialog::clearFields(); }

void CadastroUsuario::setupMapper() {
  addMapping(ui->comboBoxLoja, "idLoja", "currentValue");
  addMapping(ui->comboBoxTipo, "tipo");
  addMapping(ui->lineEditEmail, "email");
  addMapping(ui->lineEditNome, "nome");
  addMapping(ui->lineEditUser, "user");
}

void CadastroUsuario::registerMode() {
  if (limitado) { return; }

  ui->pushButtonCadastrar->show();

  ui->pushButtonAtualizar->hide();
  ui->pushButtonRemover->hide();
}

void CadastroUsuario::updateMode() {
  if (limitado) { return; }

  ui->pushButtonCadastrar->hide();

  ui->pushButtonAtualizar->show();
  ui->pushButtonRemover->show();
}

void CadastroUsuario::savingProcedures() {
  setData("nome", ui->lineEditNome->text());
  setData("idLoja", ui->comboBoxLoja->getCurrentValue());
  setData("tipo", ui->comboBoxTipo->currentText());
  setData("user", ui->lineEditUser->text());
  setData("email", ui->lineEditEmail->text());
  setData("user", ui->lineEditUser->text());

  if (ui->lineEditPasswd->text() != "********") {
    SqlQuery query;

    if (not query.exec("SELECT SHA1_PASSWORD('" + ui->lineEditPasswd->text() + "')") or not query.first()) { throw RuntimeException("Erro gerando senha: " + query.lastError().text()); }

    setData("passwd", query.value(0));
  }

  if (ui->comboBoxTipo->currentText() == "VENDEDOR ESPECIAL") { setData("especialidade", ui->comboBoxEspecialidade->currentText().left(1).toInt()); }
}

bool CadastroUsuario::viewRegister() {
  if (not RegisterDialog::viewRegister()) { return false; }

  ui->lineEditPasswd->setText("********");
  ui->lineEditPasswd_2->setText("********");

  modelPermissoes.setFilter("idUsuario = " + data("idUsuario").toString());

  modelPermissoes.select();

  for (int row = 0; row < ui->table->model()->rowCount(); ++row) { ui->table->openPersistentEditor(ui->table->model()->index(row, 0)); }

  if (ui->comboBoxTipo->currentText() == "VENDEDOR ESPECIAL") { ui->comboBoxEspecialidade->setCurrentIndex(data("especialidade").toString().left(1).toInt()); }

  return true;
}

void CadastroUsuario::fillComboBoxLoja() {
  SqlQuery query;

  if (not query.exec("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE ORDER BY descricao")) { return; }

  while (query.next()) { ui->comboBoxLoja->addItem(query.value("descricao").toString(), query.value("idLoja")); }

  ui->comboBoxLoja->setCurrentValue(UserSession::idLoja);
}

void CadastroUsuario::on_pushButtonCadastrar_clicked() { save(); }

void CadastroUsuario::on_pushButtonAtualizar_clicked() { save(); }

void CadastroUsuario::on_pushButtonNovoCad_clicked() { newRegister(); }

void CadastroUsuario::on_pushButtonRemover_clicked() { remove(); }

void CadastroUsuario::on_pushButtonBuscar_clicked() {
  if (not confirmationMessage()) { return; }

  sdUsuario->show();
}

void CadastroUsuario::cadastrar() {
  try {
    qApp->startTransaction("CadastroUsuario::cadastrar");

    if (tipo == Tipo::Cadastrar) { currentRow = model.insertRowAtEnd(); }

    savingProcedures();

    model.submitAll();

    primaryId = (tipo == Tipo::Atualizar) ? data(primaryKey).toString() : model.query().lastInsertId().toString();

    if (primaryId.isEmpty()) { throw RuntimeException("Id vazio!"); }

    if (tipo == Tipo::Cadastrar) { modelPermissoes.setData(0, "idUsuario", primaryId); }

    modelPermissoes.submitAll();

    qApp->endTransaction();

    if (tipo == Tipo::Cadastrar) { criarUsuarioMySQL(); }
  } catch (std::exception &) {
    qApp->rollbackTransaction();
    model.select();
    modelPermissoes.select();

    throw;
  }
}

void CadastroUsuario::criarUsuarioMySQL() {
  File file("mysql.txt");

  if (not file.open(QFile::ReadOnly)) { throw RuntimeException("Erro lendo mysql.txt: " + file.errorString(), this); }

  const QString password = file.readAll();

  // those query's below commit transaction so have to be done outside transaction
  SqlQuery query;
  query.prepare("CREATE USER :user@'%' IDENTIFIED WITH mysql_native_password BY '" + password + "'");
  query.bindValue(":user", ui->lineEditUser->text().toLower());

  if (not query.exec()) { throw RuntimeException("Erro criando usuário do banco de dados: " + query.lastError().text(), this); }

  query.prepare("GRANT ALL PRIVILEGES ON *.* TO :user@'%' WITH GRANT OPTION");
  query.bindValue(":user", ui->lineEditUser->text().toLower());

  if (not query.exec()) { throw RuntimeException("Erro guardando privilégios do usuário do banco de dados: " + query.lastError().text(), this); }

  if (not QSqlQuery("FLUSH PRIVILEGES").exec()) { return; }
}

void CadastroUsuario::successMessage() { qApp->enqueueInformation((tipo == Tipo::Atualizar) ? "Cadastro atualizado!" : "Usuário cadastrado com sucesso!", this); }

void CadastroUsuario::on_lineEditUser_textEdited(const QString &text) {
  SqlQuery query;
  query.prepare("SELECT idUsuario FROM usuario WHERE user = :user");
  query.bindValue(":user", text);

  if (not query.exec()) { throw RuntimeException("Erro buscando usuário: " + query.lastError().text(), this); }

  if (query.first()) { throw RuntimeError("Nome de usuário já existe!", this); }
}

void CadastroUsuario::on_comboBoxTipo_currentTextChanged(const QString &text) {
  if (text == "VENDEDOR ESPECIAL") {
    ui->labelEspecialidade->show();
    ui->comboBoxEspecialidade->show();
  } else {
    ui->labelEspecialidade->hide();
    ui->comboBoxEspecialidade->hide();
  }
}

bool CadastroUsuario::newRegister() {
  if (not RegisterDialog::newRegister()) { return false; }

  modelPermissoes.setFilter("0");

  modelPermissoes.select();

  const int row = modelPermissoes.insertRowAtEnd();

  modelPermissoes.setData(row, "view_tab_orcamento", 1);
  modelPermissoes.setData(row, "view_tab_venda", 1);
  modelPermissoes.setData(row, "view_tab_estoque", 1);
  modelPermissoes.setData(row, "view_tab_relatorio", 1);

  for (int row = 0; row < ui->table->model()->rowCount(); ++row) { ui->table->openPersistentEditor(ui->table->model()->index(row, 0)); }

  return true;
}

// TODO: 1colocar permissoes padroes para cada tipo de usuario
// TODO: colocar uma coluna 'ultimoAcesso' no BD (para saber quais usuarios nao estao mais ativos e desativar depois de x dias)
// FIXME: quando o usuario é alterado o usuario do MySql não é, fazendo com que o login não funcione mais

// FIXME: nao está mostrando mensagem de confirmacao apos desativar usuario
// TODO: mudar botao de 'remover' para 'desativar'
