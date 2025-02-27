#include "userconfig.h"
#include "ui_userconfig.h"

#include "application.h"
#include "cadastrousuario.h"
#include "sendmail.h"
#include "user.h"

#include <QDebug>
#include <QFileDialog>
#include <QSqlError>

UserConfig::UserConfig(QWidget *parent) : QDialog(parent), ui(new Ui::UserConfig) {
  ui->setupUi(this);

  ui->itemBoxLoja->setSearchDialog(SearchDialog::loja(this));

  if (not User::isAdministrativo()) { hideWidgets(); }

  if (User::isAdministrativo()) { preencherComboBoxMonitorar(); }

  getSettings();

  setConnections();

  adjustSize();
}

UserConfig::~UserConfig() { delete ui; }

void UserConfig::getSettings() {
  if (not ui->groupBoxAcbr->isHidden()) {
    SqlQuery query;

    if (not query.exec("SELECT servidorACBr, portaACBr, lojaACBr, emailContabilidade, emailLogistica FROM config")) {
      throw RuntimeException("Erro buscando dados do emissor de NF-e: " + query.lastError().text());
    }

    if (query.first()) {
      ui->lineEditACBrServidor->setText(query.value("servidorACBr").toString());
      ui->lineEditACBrPorta->setText(query.value("portaACBr").toString());
      ui->itemBoxLoja->setId(query.value("lojaACBr"));
      ui->lineEditEmailContabilidade->setText(query.value("emailContabilidade").toString());
      ui->lineEditEmailLogistica->setText(query.value("emailLogistica").toString());
    }
  }

  if (not ui->groupBoxDownloadNFe->isHidden()) {
    SqlQuery query;

    if (not query.exec("SELECT monitorarCNPJ1, monitorarServidor1, monitorarPorta1, monitorarCNPJ2, monitorarServidor2, monitorarPorta2 FROM config")) {
      throw RuntimeException("Erro buscando dados do monitor de NF-e: " + query.lastError().text());
    }

    if (query.first()) {
      ui->groupBoxDownloadNFe->setChecked(User::getSetting("User/monitorarNFe").toBool());

      ui->comboBoxMonitorar1->setCurrentIndex(ui->comboBoxMonitorar1->findData(query.value("monitorarCNPJ1")));
      ui->lineEditMonitorarServidor1->setText(query.value("monitorarServidor1").toString());
      ui->lineEditMonitorarPorta1->setText(query.value("monitorarPorta1").toString());

      ui->comboBoxMonitorar2->setCurrentIndex(ui->comboBoxMonitorar2->findData(query.value("monitorarCNPJ2")));
      ui->lineEditMonitorarServidor2->setText(query.value("monitorarServidor2").toString());
      ui->lineEditMonitorarPorta2->setText(query.value("monitorarPorta2").toString());
    }
  }

  if (not ui->groupBoxEmail->isHidden()) {
    SqlQuery query;

    if (not query.exec("SELECT servidorEmail, portaEmail, email, senhaEmail, copiaParaEmail FROM usuario_has_config WHERE idUsuario = " + User::idUsuario)) {
      throw RuntimeException("Erro buscando dados do email: " + query.lastError().text());
    }

    if (query.first()) {
      ui->lineEditServidorSMTP->setText(query.value("servidorEmail").toString());
      ui->lineEditPortaSMTP->setText(query.value("portaEmail").toString());
      ui->lineEditEmail->setText(query.value("email").toString());
      ui->lineEditEmailSenha->setText(query.value("senhaEmail").toString());
      ui->lineEditEmailCopia->setText(query.value("copiaParaEmail").toString());
    }
  }

  ui->lineEditOrcamentosFolder->setText(User::getSetting("User/OrcamentosFolder").toString());
  ui->lineEditVendasFolder->setText(User::getSetting("User/VendasFolder").toString());
  ui->lineEditComprasFolder->setText(User::getSetting("User/ComprasFolder").toString());
  ui->lineEditEntregasXmlFolder->setText(User::getSetting("User/EntregasXmlFolder").toString());
  ui->lineEditEntregasPdfFolder->setText(User::getSetting("User/EntregasPdfFolder").toString());
}

void UserConfig::hideWidgets() {
  ui->groupBoxAcbr->hide();
  ui->groupBoxDownloadNFe->hide();
  ui->groupBoxEmail->hide();
  ui->labelCompras->hide();
  ui->labelEntregas->hide();
  ui->labelEntregas_2->hide();
  ui->lineEditComprasFolder->hide();
  ui->lineEditEntregasPdfFolder->hide();
  ui->lineEditEntregasXmlFolder->hide();
  ui->pushButtonComprasFolder->hide();
  ui->pushButtonEntregasPdfFolder->hide();
  ui->pushButtonEntregasXmlFolder->hide();
}

void UserConfig::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->pushButtonAlterarDados, &QPushButton::clicked, this, &UserConfig::on_pushButtonAlterarDados_clicked, connectionType);
  connect(ui->pushButtonComprasFolder, &QPushButton::clicked, this, &UserConfig::on_pushButtonComprasFolder_clicked, connectionType);
  connect(ui->pushButtonEmailTeste, &QPushButton::clicked, this, &UserConfig::on_pushButtonEmailTeste_clicked, connectionType);
  connect(ui->pushButtonEntregasPdfFolder, &QPushButton::clicked, this, &UserConfig::on_pushButtonEntregasPdfFolder_clicked, connectionType);
  connect(ui->pushButtonEntregasXmlFolder, &QPushButton::clicked, this, &UserConfig::on_pushButtonEntregasXmlFolder_clicked, connectionType);
  connect(ui->pushButtonOrcamentosFolder, &QPushButton::clicked, this, &UserConfig::on_pushButtonOrcamentosFolder_clicked, connectionType);
  connect(ui->pushButtonSalvar, &QPushButton::clicked, this, &UserConfig::on_pushButtonSalvar_clicked, connectionType);
  connect(ui->pushButtonVendasFolder, &QPushButton::clicked, this, &UserConfig::on_pushButtonVendasFolder_clicked, connectionType);
}

void UserConfig::on_pushButtonOrcamentosFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel");

  if (path.isEmpty()) { return; }

  ui->lineEditOrcamentosFolder->setText(path);
}

void UserConfig::on_pushButtonSalvar_clicked() {
  if (not ui->groupBoxAcbr->isHidden()) { salvarDadosEmissorNFe(); }

  if (not ui->groupBoxDownloadNFe->isHidden()) {
    User::setSetting("User/monitorarNFe", ui->groupBoxDownloadNFe->isChecked());

    salvarDadosMonitorNFe();
  }

  if (not ui->groupBoxEmail->isHidden()) { salvarDadosEmail(); }

  // TODO: caso as pastas estejam vazias usar /arquivos como padrao
  User::setSetting("User/OrcamentosFolder", ui->lineEditOrcamentosFolder->text());
  User::setSetting("User/VendasFolder", ui->lineEditVendasFolder->text());
  User::setSetting("User/ComprasFolder", ui->lineEditComprasFolder->text());
  User::setSetting("User/EntregasXmlFolder", ui->lineEditEntregasXmlFolder->text());
  User::setSetting("User/EntregasPdfFolder", ui->lineEditEntregasPdfFolder->text());

  QDialog::accept();

  close();
}

void UserConfig::on_pushButtonAlterarDados_clicked() {
  auto *usuario = new CadastroUsuario(this);
  usuario->viewRegisterById(User::idUsuario);
  usuario->modificarUsuario();

  usuario->show();
}

void UserConfig::on_pushButtonVendasFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel");

  if (path.isEmpty()) { return; }

  ui->lineEditVendasFolder->setText(path);
}

void UserConfig::on_pushButtonComprasFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel");

  if (path.isEmpty()) { return; }

  ui->lineEditComprasFolder->setText(path);
}

void UserConfig::on_pushButtonEntregasXmlFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta XML");

  if (path.isEmpty()) { return; }

  ui->lineEditEntregasXmlFolder->setText(path);
}

void UserConfig::on_pushButtonEntregasPdfFolder_clicked() {
  const QString path = QFileDialog::getExistingDirectory(this, "Pasta PDF/Excel");

  if (path.isEmpty()) { return; }

  ui->lineEditEntregasPdfFolder->setText(path);
}

void UserConfig::on_pushButtonEmailTeste_clicked() {
  if (ui->lineEditServidorSMTP->text().isEmpty() or ui->lineEditPortaSMTP->text().isEmpty() or ui->lineEditEmail->text().isEmpty() or ui->lineEditEmailSenha->text().isEmpty()) {
    throw RuntimeError("Preencha os dados do email!", this);
  }

  salvarDadosEmail();

  auto *mail = new SendMail(SendMail::Tipo::Teste, this);
  mail->setAttribute(Qt::WA_DeleteOnClose);

  mail->show();
}

void UserConfig::preencherComboBoxMonitorar() {
  SqlQuery query;

  if (not query.exec("SELECT DISTINCT(LEFT(cnpj, 10)) AS raiz, razaoSocial FROM loja WHERE cnpj IS NOT NULL")) { throw RuntimeException("Erro buscando empresas: " + query.lastError().text()); }

  while (query.next()) {
    ui->comboBoxMonitorar1->addItem(query.value("razaoSocial").toString(), query.value("raiz"));
    ui->comboBoxMonitorar2->addItem(query.value("razaoSocial").toString(), query.value("raiz"));
  }
}

void UserConfig::salvarDadosEmail() {
  SqlQuery queryUsuario;
  queryUsuario.prepare("INSERT INTO usuario_has_config (idUsuario, servidorEmail, portaEmail, email, senhaEmail, copiaParaEmail) "
                       "VALUES (:idUsuario, :servidorEmail, :portaEmail, :email, :senhaEmail, :copiaParaEmail) AS new "
                       "ON DUPLICATE KEY UPDATE servidorEmail = new.servidorEmail, portaEmail = new.portaEmail, email = new.email, senhaEmail = new.senhaEmail, copiaParaEmail = new.copiaParaEmail");
  queryUsuario.bindValue(":idUsuario", User::idUsuario);
  queryUsuario.bindValue(":servidorEmail", ui->lineEditServidorSMTP->text());
  queryUsuario.bindValue(":portaEmail", ui->lineEditPortaSMTP->text());
  queryUsuario.bindValue(":email", ui->lineEditEmail->text());
  queryUsuario.bindValue(":senhaEmail", ui->lineEditEmailSenha->text());
  queryUsuario.bindValue(":copiaParaEmail", ui->lineEditEmailCopia->text());

  if (not queryUsuario.exec()) { throw RuntimeException("Erro salvando dados do e-mail: " + queryUsuario.lastError().text()); }
}

void UserConfig::salvarDadosEmissorNFe() {
  SqlQuery queryLoja;
  queryLoja.prepare(
      "INSERT INTO config (idConfig, servidorACBr, portaACBr, lojaACBr, emailContabilidade, emailLogistica) "
      "VALUES (1, :servidorACBr, :portaACBr, :lojaACBr, :emailContabilidade, :emailLogistica) AS new "
      "ON DUPLICATE KEY UPDATE servidorACBr = new.servidorACBr, portaACBr = new.portaACBr, lojaACBr = new.lojaACBr, emailContabilidade = new.emailContabilidade, emailLogistica = new.emailLogistica");
  queryLoja.bindValue(":servidorACBr", ui->lineEditACBrServidor->text());
  queryLoja.bindValue(":portaACBr", ui->lineEditACBrPorta->text());
  queryLoja.bindValue(":lojaACBr", ui->itemBoxLoja->getId());
  queryLoja.bindValue(":emailContabilidade", ui->lineEditEmailContabilidade->text());
  queryLoja.bindValue(":emailLogistica", ui->lineEditEmailLogistica->text());

  if (not queryLoja.exec()) { throw RuntimeException("Erro salvando dados do emissor de NF-e: " + queryLoja.lastError().text()); }
}

void UserConfig::salvarDadosMonitorNFe() {
  SqlQuery queryLoja;
  queryLoja.prepare("INSERT INTO config (idConfig, monitorarCNPJ1, monitorarServidor1, monitorarPorta1, monitorarCNPJ2, monitorarServidor2, monitorarPorta2) "
                    "VALUES (1, :monitorarCNPJ1, :monitorarServidor1, :monitorarPorta1, :monitorarCNPJ2, :monitorarServidor2, :monitorarPorta2) AS new "
                    "ON DUPLICATE KEY UPDATE monitorarCNPJ1 = new.monitorarCNPJ1, monitorarServidor1 = new.monitorarServidor1, monitorarPorta1 = new.monitorarPorta1, "
                    "monitorarCNPJ2 = new.monitorarCNPJ2, monitorarServidor2 = new.monitorarServidor2, monitorarPorta2 = new.monitorarPorta2");
  queryLoja.bindValue(":monitorarCNPJ1", ui->comboBoxMonitorar1->currentData());
  queryLoja.bindValue(":monitorarServidor1", ui->lineEditMonitorarServidor1->text());
  queryLoja.bindValue(":monitorarPorta1", ui->lineEditMonitorarPorta1->text());
  queryLoja.bindValue(":monitorarCNPJ2", ui->comboBoxMonitorar2->currentData());
  queryLoja.bindValue(":monitorarServidor2", ui->lineEditMonitorarServidor2->text());
  queryLoja.bindValue(":monitorarPorta2", ui->lineEditMonitorarPorta2->text());

  if (not queryLoja.exec()) { throw RuntimeException("Erro salvando dados do monitor de NF-e: " + queryLoja.lastError().text()); }
}
