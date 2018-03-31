#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

#include "acbr.h"
#include "doubledelegate.h"
#include "lrreportengine.h"
#include "reaisdelegate.h"
#include "sendmail.h"
#include "ui_widgetnfesaida.h"
#include "usersession.h"
#include "widgetnfesaida.h"
#include "xml_viewer.h"

WidgetNfeSaida::WidgetNfeSaida(QWidget *parent) : Widget(parent), ui(new Ui::WidgetNfeSaida) {
  ui->setupUi(this);

  ui->dateEdit->setDate(QDate::currentDate());

  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetNfeSaida::montaFiltro);
  connect(ui->groupBoxMes, &QGroupBox::toggled, this, &WidgetNfeSaida::montaFiltro);
  connect(ui->dateEdit, &QDateEdit::dateChanged, this, &WidgetNfeSaida::montaFiltro);
  connect(ui->checkBoxAutorizado, &QCheckBox::toggled, this, &WidgetNfeSaida::montaFiltro);
  connect(ui->checkBoxPendente, &QCheckBox::toggled, this, &WidgetNfeSaida::montaFiltro);
  connect(ui->checkBoxCancelado, &QCheckBox::toggled, this, &WidgetNfeSaida::montaFiltro);

  connect(ui->groupBoxStatus, &QGroupBox::toggled, this, &WidgetNfeSaida::on_groupBoxStatus_toggled);
  connect(ui->pushButtonCancelarNFe, &QPushButton::clicked, this, &WidgetNfeSaida::on_pushButtonCancelarNFe_clicked);
  connect(ui->pushButtonConsultarNFe, &QPushButton::clicked, this, &WidgetNfeSaida::on_pushButtonConsultarNFe_clicked);
  connect(ui->pushButtonExportar, &QPushButton::clicked, this, &WidgetNfeSaida::on_pushButtonExportar_clicked);
  connect(ui->pushButtonRelatorio, &QPushButton::clicked, this, &WidgetNfeSaida::on_pushButtonRelatorio_clicked);
  connect(ui->table, &TableView::activated, this, &WidgetNfeSaida::on_table_activated);
  connect(ui->table, &TableView::entered, this, &WidgetNfeSaida::on_table_entered);
}

WidgetNfeSaida::~WidgetNfeSaida() { delete ui; }

bool WidgetNfeSaida::updateTables() {
  if (modelViewNFeSaida.tableName().isEmpty()) setupTables();

  // REFAC: 1refatorar
  //  if (not model.select()) {
  //    emit errorSignal("Erro lendo tabela NFe: " + model.lastError().text());
  //    return false;
  //  }
  if (not montaFiltro()) { return false; }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeSaida::setupTables() {
  modelViewNFeSaida.setTable("view_nfe_saida");
  modelViewNFeSaida.setEditStrategy(QSqlTableModel::OnManualSubmit);

  modelViewNFeSaida.setHeaderData("created", "Criado em");
  modelViewNFeSaida.setHeaderData("valor", "R$");

  ui->table->setModel(&modelViewNFeSaida);
  ui->table->hideColumn("idNFe");
  ui->table->hideColumn("chaveAcesso");
  ui->table->setItemDelegate(new DoubleDelegate(this));
  ui->table->setItemDelegateForColumn("valor", new ReaisDelegate(this));
}

void WidgetNfeSaida::on_table_activated(const QModelIndex &index) {
  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE idNFe = :idNFe");
  query.bindValue(":idNFe", modelViewNFeSaida.data(index.row(), "idNFe"));

  if (not query.exec() or not query.first()) {
    emit errorSignal("Erro buscando xml da nota: " + query.lastError().text());
    return;
  }

  auto *viewer = new XML_Viewer(this);
  viewer->setAttribute(Qt::WA_DeleteOnClose);
  viewer->exibirXML(query.value("xml").toByteArray());
}

bool WidgetNfeSaida::montaFiltro() {
  // TODO: 5ordenar por 'data criado'

  const QString text = ui->lineEditBusca->text();

  const QString filtroBusca = "(NFe LIKE '%" + text + "%' OR Venda LIKE '%" + text + "%' OR `CPF/CNPJ` LIKE '%" + text + "%' OR Cliente LIKE '%" + text + "%')";

  const QString filtroData = ui->groupBoxMes->isChecked() ? " AND DATE_FORMAT(`Criado em`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "'" : "";

  QString filtroCheck;

  Q_FOREACH (const auto &child, ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) filtroCheck += filtroCheck.isEmpty() ? "status = '" + child->text().toUpper() + "'" : " OR status = '" + child->text().toUpper() + "'";
  }

  filtroCheck = filtroCheck.isEmpty() ? "" : " AND (" + filtroCheck + ")";

  modelViewNFeSaida.setFilter(filtroBusca + filtroData + filtroCheck);

  if (not modelViewNFeSaida.select()) {
    emit errorSignal("Erro lendo tabela: " + modelViewNFeSaida.lastError().text());
    return false;
  }

  ui->table->resizeColumnsToContents();

  return true;
}

void WidgetNfeSaida::on_table_entered(const QModelIndex) { ui->table->resizeColumnsToContents(); }

void WidgetNfeSaida::on_pushButtonCancelarNFe_clicked() {
  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    emit errorSignal("Nenhuma linha selecionada!");
    return;
  }

  QMessageBox msgBox(QMessageBox::Question, "Cancelar?", "Tem certeza que deseja cancelar?", QMessageBox::Yes | QMessageBox::No, this);
  msgBox.setButtonText(QMessageBox::Yes, "Cancelar");
  msgBox.setButtonText(QMessageBox::No, "Voltar");

  if (msgBox.exec() == QMessageBox::No) { return; }

  const QString justificativa = QInputDialog::getText(this, "Justificativa", "Entre 15 e 200 caracteres: ");

  if (justificativa.size() < 15 or justificativa.size() > 200) {
    emit errorSignal("Justificativa fora do tamanho!");
    return;
  }

  const int row = list.first().row();

  const QString chaveAcesso = modelViewNFeSaida.data(row, "chaveAcesso").toString();

  const auto resposta = ACBr::enviarComando("NFE.CancelarNFe(" + chaveAcesso + ", " + justificativa + ")");

  if (not resposta) { return; }

  // TODO: verificar outras possiveis respostas (tinha algo como 'cancelamento registrado fora do prazo')
  if (not resposta->contains("xEvento=Cancelamento registrado")) {
    emit errorSignal("Resposta: " + *resposta);
    return;
  }

  emit transactionStarted();

  if (not QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec()) { return; }
  if (not QSqlQuery("START TRANSACTION").exec()) { return; }

  if (not cancelarNFe(chaveAcesso, row)) {
    QSqlQuery("ROLLBACK").exec();
    emit transactionEnded();
    return;
  }

  if (not QSqlQuery("COMMIT").exec()) { return; }

  emit transactionEnded();

  emit informationSignal(*resposta);

  if (not gravarArquivo(*resposta)) { return; }

  const QString anexoPath = QDir::currentPath() + "/cancelamento.xml";

  //

  const auto emailContabilidade = UserSession::getSetting("User/emailContabilidade");

  if (not emailContabilidade) {
    emit errorSignal("A chave 'emailContabilidade' não está configurada!");
    return;
  }

  const auto emailLogistica = UserSession::getSetting("User/emailLogistica");

  if (not emailLogistica) {
    emit errorSignal("A chave 'emailLogistica' não está configurada!");
    return;
  }

  const QString assunto = "Cancelamento NFe - " + modelViewNFeSaida.data(row, "NFe").toString() + " - STACCATO REVESTIMENTOS COMERCIO E REPRESENTACAO LTDA";

  ACBr::enviarEmail(emailContabilidade.value().toString(), emailLogistica.value().toString(), assunto, anexoPath);
}

// TODO: 1verificar se ao cancelar nota ela é removida do venda_produto/veiculo_produto
// TODO: 1botao para gerar relatorio igual ao da receita

void WidgetNfeSaida::on_pushButtonRelatorio_clicked() {
  // TODO: 3formatar decimais no padrao BR
  // TODO: 3perguntar um intervalo de tempo para filtrar as notas
  // TODO: 3verificar quais as tags na nota dos campos que faltam preencher

  if (not ui->groupBoxMes->isChecked()) {
    emit errorSignal("Selecione um mês para gerar o relatório!");
    return;
  }

  LimeReport::ReportEngine report;
  auto dataManager = report.dataManager();

  SqlRelationalTableModel view;
  view.setTable("view_relatorio_nfe");
  view.setFilter("DATE_FORMAT(`Criado em`, '%Y-%m') = '" + ui->dateEdit->date().toString("yyyy-MM") + "' AND (status = 'AUTORIZADO')");

  if (not view.select()) {
    emit errorSignal("Erro comunicando com banco de dados: " + view.lastError().text());
    return;
  }

  dataManager->addModel("view", &view, true);

  if (not report.loadFromFile("view.lrxml")) {
    emit errorSignal("Não encontrou o modelo de impressão!");
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT SUM(icms), SUM(icmsst), SUM(frete), SUM(totalnfe), SUM(desconto), SUM(impimp), SUM(ipi), SUM(cofins), SUM(0), SUM(0), SUM(seguro), SUM(pis), SUM(0) FROM view_relatorio_nfe "
                "WHERE DATE_FORMAT(`Criado em`, '%Y-%m') = :data AND (status = 'AUTORIZADO')");
  query.bindValue(":data", ui->dateEdit->date().toString("yyyy-MM"));

  if (not query.exec() or not query.first()) {
    emit errorSignal("Erro buscando dados: " + query.lastError().text());
    return;
  }

  dataManager->setReportVariable("TotalIcms", "R$ " + QString::number(query.value("sum(icms)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalIcmsSt", "R$ " + QString::number(query.value("sum(icmsst)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalFrete", "R$ " + QString::number(query.value("sum(frete)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalNfe", "R$ " + QString::number(query.value("sum(totalnfe)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalDesconto", "R$ " + QString::number(query.value("sum(desconto)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalImpImp", "R$ " + QString::number(query.value("sum(impimp)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalIpi", "R$ " + QString::number(query.value("sum(ipi)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalCofins", "R$ " + QString::number(query.value("sum(cofins)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalPisSt", "R$ XXX");
  dataManager->setReportVariable("TotalCofinsSt", "R$ XXX");
  dataManager->setReportVariable("TotalSeguro", "R$ " + QString::number(query.value("sum(seguro)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalPis", "R$ " + QString::number(query.value("sum(pis)").toDouble(), 'f', 2));
  dataManager->setReportVariable("TotalIssqn", "R$ XXX");

  if (not report.printToPDF(QDir::currentPath() + "/relatorio.pdf")) {
    emit errorSignal("Erro gerando relatório!");
    return;
  }

  if (not QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath() + "/relatorio.pdf"))) {
    emit errorSignal("Erro abrindo arquivo: " + QDir::currentPath() + "/relatorio.pdf'!");
    return;
  }
}

void WidgetNfeSaida::on_pushButtonExportar_clicked() {
  // TODO: 5zipar arquivos exportados com nome descrevendo mes/notas/etc

  const auto list = ui->table->selectionModel()->selectedRows();

  if (list.isEmpty()) {
    emit errorSignal("Nenhum item selecionado!");
    return;
  }

  QSqlQuery query;
  query.prepare("SELECT xml FROM nfe WHERE chaveAcesso = :chaveAcesso");

  const auto folderKeyXml = UserSession::getSetting("User/EntregasXmlFolder");

  if (not folderKeyXml or folderKeyXml.value().toString().isEmpty()) {
    emit errorSignal("Não há uma pasta definida para salvar XML. Por favor escolha uma nas configurações do ERP!");
    return;
  }

  const auto folderKeyPdf = UserSession::getSetting("User/EntregasPdfFolder");

  if (not folderKeyPdf or folderKeyPdf.value().toString().isEmpty()) {
    emit errorSignal("Não há uma pasta definida para salvar PDF. Por favor escolha uma nas configurações do ERP!");
    return;
  }

  const QString xmlFolder = folderKeyXml.value().toString();
  const QString pdfFolder = folderKeyPdf.value().toString();

  // TODO: create folders if they dont exist (it wont work it they dont)

  for (const auto &item : list) {
    // TODO: 3wrap this in a function so if connection to acbr is dropped the erp retries
    // quando enviar para o acbr guardar a nota com status 'pendente' para consulta na receita
    // quando conseguir consultar se a receita retornar que a nota nao existe lá apagar aqui
    // se ela existir lá verificar se consigo pegar o xml autorizado e atualizar a nota pendente

    if (modelViewNFeSaida.data(item.row(), "status").toString() != "AUTORIZADO") continue;

    // pegar xml do bd e salvar em arquivo

    const QString chaveAcesso = modelViewNFeSaida.data(item.row(), "chaveAcesso").toString();

    query.bindValue(":chaveAcesso", chaveAcesso);

    if (not query.exec() or not query.first()) {
      emit errorSignal("Erro buscando xml: " + query.lastError().text());
      return;
    }

    QFile fileXml(xmlFolder + "/" + chaveAcesso + ".xml");

    qDebug() << "xml: " << xmlFolder + "/" + chaveAcesso + ".xml";

    if (not fileXml.open(QFile::WriteOnly)) {
      emit errorSignal("Erro abrindo arquivo para escrita xml: " + fileXml.errorString());
      return;
    }

    fileXml.write(query.value("xml").toByteArray());

    fileXml.flush();
    fileXml.close();

    // mandar xml para acbr gerar pdf

    auto pdfOrigem = ACBr::gerarDanfe(query.value("xml").toByteArray(), false);

    if (not pdfOrigem) { return; }

    if (pdfOrigem->isEmpty()) {
      emit errorSignal("Resposta vazia!");
      return;
    }

    // copiar para pasta predefinida

    const QString pdfDestino = pdfFolder + "/" + chaveAcesso + ".pdf";

    QFile filePdf(pdfDestino);

    if (filePdf.exists()) filePdf.remove();

    if (not QFile::copy(*pdfOrigem, pdfDestino)) {
      emit errorSignal("Erro copiando pdf!");
      return;
    }
  }

  emit informationSignal("Arquivos exportados com sucesso para " + pdfFolder + "!");
}

void WidgetNfeSaida::on_groupBoxStatus_toggled(const bool enabled) {
  Q_FOREACH (const auto &child, ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetNfeSaida::on_pushButtonConsultarNFe_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) {
    emit errorSignal("Nenhuma linha selecionada!");
    return;
  }

  //  if (model.data(selection.first().row(), "status").toString() != "NOTA PENDENTE") {
  //    emit errorSignal("Nota não está pendente!");
  //    return;
  //  }

  const int idNFe = modelViewNFeSaida.data(selection.first().row(), "idNFe").toInt();

  if (auto tuple = ACBr::consultarNFe(idNFe); tuple) {
    auto [xml, resposta] = *tuple;

    emit transactionStarted();

    if (not QSqlQuery("SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE").exec()) { return; }
    if (not QSqlQuery("START TRANSACTION").exec()) { return; }

    if (not atualizarNFe(idNFe, xml)) {
      QSqlQuery("ROLLBACK").exec();
      emit transactionEnded();
      return;
    }

    if (not QSqlQuery("COMMIT").exec()) { return; }

    emit transactionEnded();

    emit informationSignal(resposta);
  }
}

bool WidgetNfeSaida::atualizarNFe(const int idNFe, const QString &xml) {
  QSqlQuery query;
  query.prepare("UPDATE nfe SET status = 'AUTORIZADO', xml = :xml WHERE idNFe = :idNFe");
  query.bindValue(":xml", xml);
  query.bindValue(":idNFe", idNFe);

  if (not query.exec()) {
    emit errorSignal("Erro marcando nota como 'AUTORIZADO': " + query.lastError().text());
    return false;
  }

  return true;
}

bool WidgetNfeSaida::cancelarNFe(const QString &chaveAcesso, const int row) {
  QSqlQuery query;
  query.prepare("UPDATE nfe SET status = 'CANCELADO' WHERE chaveAcesso = :chaveAcesso");
  query.bindValue(":chaveAcesso", chaveAcesso);

  if (not query.exec()) {
    emit errorSignal("Erro marcando nota como cancelada: " + query.lastError().text());
    return false;
  }

  const int idNFe = modelViewNFeSaida.data(row, "idNFe").toInt();

  query.prepare("UPDATE venda_has_produto SET status = 'ENTREGA AGEND.', idNFeSaida = NULL WHERE idNFeSaida = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec()) {
    emit errorSignal("Erro removendo NFe da venda_produto: " + query.lastError().text());
    return false;
  }

  query.prepare("UPDATE veiculo_has_produto SET status = 'ENTREGA AGEND.', idNFeSaida = NULL WHERE idNFeSaida = :idNFe");
  query.bindValue(":idNFe", idNFe);

  if (not query.exec()) {
    emit errorSignal("Erro removendo NFe do veiculo_produto: " + query.lastError().text());
    return false;
  }

  return true;
}

bool WidgetNfeSaida::gravarArquivo(const QString &resposta) {
  QFile arquivo(QDir::currentPath() + "/cancelamento.xml");

  if (not arquivo.open(QFile::WriteOnly)) {
    emit errorSignal("Erro abrindo arquivo para escrita: " + arquivo.errorString());
    return false;
  }

  QTextStream stream(&arquivo);

  stream << resposta;

  arquivo.close();

  return true;
}

// TODO: guardar idVenda/outros dados na nfe para quando cancelar nao perder os vinculos
// TODO: 2tela para importar notas de amostra (aba separada)
// TODO: 0nao estou guardando o valor na nota
// TODO: 0algumas notas nao estao mostrando valor
// TODO: nesta tela colocar um campo dizendo qual loja que emitiu a nota (nao precisa mostrar o cnpj, apenas o nome da loja) (e talvez poder filtrar pela loja)
// TODO: fazer sistema para trocar notas futuras pela nota real
