#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QMessageBox>
#include <QShortcut>
#include <QSqlError>
#include <QStyleFactory>
#include <QUrl>
#include <cmath>

#include "application.h"
#include "cadastrocliente.h"
#include "cadastrofornecedor.h"
#include "cadastroloja.h"
#include "cadastroproduto.h"
#include "cadastroprofissional.h"
#include "cadastrotransportadora.h"
#include "cadastrousuario.h"
#include "calculofrete.h"
#include "importaprodutos.h"
#include "mainwindow.h"
#include "orcamento.h"
#include "precoestoque.h"
#include "ui_mainwindow.h"
#include "userconfig.h"
#include "usersession.h"
#include "xlsxdocument.h"

// QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  connect(ui->actionCadastrarCliente, &QAction::triggered, this, &MainWindow::on_actionCadastrarCliente_triggered);
  connect(ui->actionCadastrarFornecedor, &QAction::triggered, this, &MainWindow::on_actionCadastrarFornecedor_triggered);
  connect(ui->actionCadastrarProdutos, &QAction::triggered, this, &MainWindow::on_actionCadastrarProdutos_triggered);
  connect(ui->actionCadastrarProfissional, &QAction::triggered, this, &MainWindow::on_actionCadastrarProfissional_triggered);
  connect(ui->actionCadastrarUsuario, &QAction::triggered, this, &MainWindow::on_actionCadastrarUsuario_triggered);
  connect(ui->actionCalculadora, &QAction::triggered, this, &MainWindow::on_actionCalculadora_triggered);
  connect(ui->actionCalcular_frete, &QAction::triggered, this, &MainWindow::on_actionCalcular_frete_triggered);
  connect(ui->actionClaro, &QAction::triggered, this, &MainWindow::on_actionClaro_triggered);
  connect(ui->actionConfiguracoes, &QAction::triggered, this, &MainWindow::on_actionConfiguracoes_triggered);
  connect(ui->actionCriarOrcamento, &QAction::triggered, this, &MainWindow::on_actionCriarOrcamento_triggered);
  connect(ui->actionEscuro, &QAction::triggered, this, &MainWindow::on_actionEscuro_triggered);
  connect(ui->actionEstoque, &QAction::triggered, this, &MainWindow::on_actionEstoque_triggered);
  connect(ui->actionGerenciar_Lojas, &QAction::triggered, this, &MainWindow::on_actionGerenciar_Lojas_triggered);
  connect(ui->actionGerenciar_Transportadoras, &QAction::triggered, this, &MainWindow::on_actionGerenciar_Transportadoras_triggered);
  connect(ui->actionGerenciar_preco_estoque, &QAction::triggered, this, &MainWindow::on_actionGerenciar_preco_estoque_triggered);
  connect(ui->actionProdutos, &QAction::triggered, this, &MainWindow::on_actionProdutos_triggered);
  connect(ui->actionPromocao, &QAction::triggered, this, &MainWindow::on_actionPromocao_triggered);
  connect(ui->actionSobre, &QAction::triggered, this, &MainWindow::on_actionSobre_triggered);
  connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::on_tabWidget_currentChanged);

  setWindowIcon(QIcon("Staccato.ico"));
  setWindowTitle("ERP Staccato");

  QShortcut *shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this);
  connect(shortcut, &QShortcut::activated, this, &QWidget::close);

  const auto hostname = UserSession::getSetting("Login/hostname");

  if (not hostname) {
    QMessageBox::critical(nullptr, "Erro!", "A chave 'hostname' não está configurada!");
    return;
  }

  const QString hostnameText = qApp->getMapLojas().key(hostname.value().toString());

  setWindowTitle(windowTitle() + " - " + UserSession::nome() + " - " + UserSession::tipoUsuario() + " - " + (hostnameText.isEmpty() ? hostname.value().toString() : hostnameText));

  if (UserSession::tipoUsuario() != "ADMINISTRADOR" and UserSession::tipoUsuario() != "GERENTE LOJA") {
    ui->actionGerenciar_Lojas->setDisabled(true);
    ui->actionGerenciar_Transportadoras->setDisabled(true);
    ui->menuImportar_tabela_fornecedor->setDisabled(true);
    ui->actionCadastrarUsuario->setDisabled(true);
    ui->actionCadastrarProfissional->setDisabled(true);
    ui->actionCadastrarFornecedor->setDisabled(true);
    ui->actionGerenciar_preco_estoque->setDisabled(true);
  }

  //

  QSqlQuery query;
  query.prepare("SELECT * FROM usuario_has_permissao WHERE idUsuario = :idUsuario");
  query.bindValue(":idUsuario", UserSession::idUsuario());

  if (not query.exec() or not query.first()) QMessageBox::critical(this, "Erro!", "Erro lendo permissões: " + query.lastError().text());

  // REFAC: dont harcode numbers
  ui->tabWidget->setTabEnabled(0, query.value("view_tab_orcamento").toBool());
  ui->tabWidget->setTabEnabled(1, query.value("view_tab_venda").toBool());
  ui->tabWidget->setTabEnabled(2, query.value("view_tab_compra").toBool());
  ui->tabWidget->setTabEnabled(3, query.value("view_tab_logistica").toBool());
  ui->tabWidget->setTabEnabled(4, query.value("view_tab_nfe").toBool());
  ui->tabWidget->setTabEnabled(5, query.value("view_tab_estoque").toBool());
  ui->tabWidget->setTabEnabled(6, query.value("view_tab_financeiro").toBool());
  ui->tabWidget->setTabEnabled(7, query.value("view_tab_relatorio").toBool());

  //

  ui->tabWidget->setTabEnabled(8, false);

  //

  pushButtonStatus = new QPushButton(this);
  pushButtonStatus->setIcon(QIcon(":/reconnect.png"));
  pushButtonStatus->setText("Conectado: " + UserSession::getSetting("Login/hostname").value().toString());
  pushButtonStatus->setStyleSheet("color: rgb(0, 255, 0);");

  ui->statusBar->addWidget(pushButtonStatus);

  connect(pushButtonStatus, &QPushButton::clicked, this, &MainWindow::verifyDb);
  connect(qApp, &Application::verifyDb, this, &MainWindow::verifyDb);

  //  QSqlQuery queryChart;

  //  qDebug() << queryChart.exec("SELECT * FROM view_relatorio_temp");

  //  int dia = 1;

  //  QLineSeries *seriesJan = new QLineSeries();
  //  QLineSeries *seriesFev = new QLineSeries();
  //  QLineSeries *seriesMar = new QLineSeries();

  //  while (queryChart.next()) {

  //    seriesJan->append(dia, queryChart.value("jan").toDouble());
  //    seriesFev->append(dia, queryChart.value("fev").toDouble());
  //    seriesMar->append(dia, queryChart.value("mar").toDouble());

  //    dia++;
  //  }

  //  QChart *chart = new QChart();
  //  chart->legend()->hide();
  //  chart->addSeries(seriesJan);
  //  chart->addSeries(seriesFev);
  //  chart->addSeries(seriesMar);
  //  chart->createDefaultAxes();
  //  chart->setTitle("Simple line chart example");

  //  QChartView *chartView = new QChartView(chart);
  //  chartView->setRenderHint(QPainter::Antialiasing);

  //  ui->tabWidget->widget(8)->layout()->addWidget(chartView);

  // NOTE: fazer o mes atual ate o dia corrente
  // fazer o mes atual com a linha em bold
  // fazer o mesmo mes do ano anterior em bold
  // fazer uma linha diferente com a media

  //  gerarEnviarRelatorio();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::verifyDb() {
  const bool conectado = qApp->dbConnect();

  pushButtonStatus->setText(conectado ? "Conectado: " + UserSession::getSetting("Login/hostname").value().toString() : "Desconectado");
  pushButtonStatus->setStyleSheet(conectado ? "color: rgb(0, 255, 0);" : "color: rgb(255, 0, 0);");

  if (conectado) updateTables();
}

// REFAC: put this in a class
void MainWindow::gerarEnviarRelatorio() {
  // REFAC: 0finish
  // verificar em que etapa eu guardo a linha do dia seguinte no BD

  QSqlQuery query;
  // TODO: replace *
  query.prepare("SELECT * FROM jobs WHERE dataReferente = :dataReferente AND status = 'PENDENTE'");
  query.bindValue(":dataAgendado", QDate::currentDate());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando relatórios agendados: " + query.lastError().text());
    return;
  }

  while (query.next()) {
    const QString relatorioPagar = "C:/temp/pagar.xlsx";     // guardar direto no servidor?
    const QString relatorioReceber = "C:/temp/receber.xlsx"; // e se o computador nao tiver o servidor mapeado?

    //

    QXlsx::Document xlsxPagar(relatorioPagar);

    //    xlsx.currentWorksheet()->setFitToPage(true);
    //    xlsx.currentWorksheet()->setFitToHeight(true);
    //    xlsx.currentWorksheet()->setOrientationVertical(false);

    QSqlQuery queryView;

    if (not queryView.exec("SELECT * FROM view_relatorio_pagar")) {
      QMessageBox::critical(this, "Erro!", "Erro lendo relatorio pagar: " + queryView.lastError().text());
      return;
    }

    xlsxPagar.write("A1", "Data Emissão");
    xlsxPagar.write("B1", "Data Realizado");
    xlsxPagar.write("C1", "Valor R$");
    xlsxPagar.write("D1", "Conta");
    xlsxPagar.write("E1", "Obs.");
    xlsxPagar.write("F1", "Contraparte");
    xlsxPagar.write("G1", "Grupo");
    xlsxPagar.write("H1", "Subgrupo");

    int row = 1;

    while (queryView.next()) {
      xlsxPagar.write("A" + QString::number(row), queryView.value("dataEmissao"));
      xlsxPagar.write("B" + QString::number(row), queryView.value("dataRealizado"));
      xlsxPagar.write("C" + QString::number(row), queryView.value("valorReal"));
      xlsxPagar.write("D" + QString::number(row), queryView.value("Conta"));
      xlsxPagar.write("E" + QString::number(row), queryView.value("observacao"));
      xlsxPagar.write("F" + QString::number(row), queryView.value("contraParte"));
      xlsxPagar.write("G" + QString::number(row), queryView.value("grupo"));
      xlsxPagar.write("H" + QString::number(row), queryView.value("subGrupo"));

      ++row;
    }

    //

    QXlsx::Document xlsxReceber(relatorioReceber);

    //    xlsx.currentWorksheet()->setFitToPage(true);
    //    xlsx.currentWorksheet()->setFitToHeight(true);
    //    xlsx.currentWorksheet()->setOrientationVertical(false);

    if (not queryView.exec("SELECT * FROM view_relatorio_receber")) {
      QMessageBox::critical(this, "Erro!", "Erro lendo relatorio receber: " + queryView.lastError().text());
      return;
    }

    xlsxReceber.write("A1", "dataEmissao");
    xlsxReceber.write("B1", "dataRealizado");
    xlsxReceber.write("C1", "valorReal");
    xlsxReceber.write("D1", "Conta");
    xlsxReceber.write("E1", "observacao");
    xlsxReceber.write("F1", "contraParte");
    xlsxReceber.write("G1", "grupo");
    xlsxReceber.write("H1", "subGrupo");

    row = 1;

    while (queryView.next()) {
      xlsxReceber.write("A" + QString::number(row), queryView.value("dataEmissao"));
      xlsxReceber.write("B" + QString::number(row), queryView.value("dataRealizado"));
      xlsxReceber.write("C" + QString::number(row), queryView.value("valorReal"));
      xlsxReceber.write("D" + QString::number(row), queryView.value("Conta"));
      xlsxReceber.write("E" + QString::number(row), queryView.value("observacao"));
      xlsxReceber.write("F" + QString::number(row), queryView.value("contraParte"));
      xlsxReceber.write("G" + QString::number(row), queryView.value("grupo"));
      xlsxReceber.write("H" + QString::number(row), queryView.value("subGrupo"));

      ++row;
    }

    //

    QSqlQuery query2;
    query2.prepare("INSERT INTO jobs (dataEnviado, dataReferente, status) VALUES (:dataEnviado, :dataReferente, 'ENVIADO')");

    const int diaSemana = QDate::currentDate().dayOfWeek();

    query2.bindValue(":dataReferente", QDate::currentDate().addDays(diaSemana < 4 ? 5 : diaSemana - 3));
    query2.bindValue(":dataEnviado", QDate::currentDate());

    if (not query2.exec()) {
      QMessageBox::critical(this, "Erro!", "Erro guardando relatórios financeiro: " + query2.lastError().text());
      return;
    }

    //    SendMail *mail = new SendMail(this, anexo, fornecedor);
    //    mail->setAttribute(Qt::WA_DeleteOnClose);

    //    mail->exec();
  }
}

void MainWindow::on_actionCriarOrcamento_triggered() {
  auto *orcamento = new Orcamento(this);
  orcamento->setAttribute(Qt::WA_DeleteOnClose);
  orcamento->show();
}

void MainWindow::on_actionCadastrarProdutos_triggered() {
  auto *cad = new CadastroProduto(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarCliente_triggered() {
  auto *cad = new CadastroCliente(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarUsuario_triggered() {
  auto *cad = new CadastroUsuario(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionCadastrarProfissional_triggered() {
  auto *cad = new CadastroProfissional(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionGerenciar_Transportadoras_triggered() {
  auto *cad = new CadastroTransportadora(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::on_actionGerenciar_Lojas_triggered() {
  auto *cad = new CadastroLoja(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

void MainWindow::updateTables() {
  if (qApp->getUpdating()) return;
  if (not qApp->getIsConnected()) return;
  if (qApp->getShowingErrors()) return;

  qApp->setUpdating(true);

  const QString currentText = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

  if (currentText == "Orçamentos") ui->widgetOrcamento->updateTables();
  if (currentText == "Vendas") ui->widgetVenda->updateTables();
  if (currentText == "Compras") ui->widgetCompra->updateTables();
  if (currentText == "Logística") ui->widgetLogistica->updateTables();
  if (currentText == "NFe") ui->widgetNfe->updateTables();
  if (currentText == "Estoque") ui->widgetEstoque->updateTables();
  if (currentText == "Financeiro") ui->widgetFinanceiro->updateTables();
  if (currentText == "Relatórios") ui->widgetRelatorio->updateTables();

  qApp->setUpdating(false);
}

void MainWindow::on_actionCadastrarFornecedor_triggered() {
  auto *cad = new CadastroFornecedor(this);
  cad->setAttribute(Qt::WA_DeleteOnClose);
  cad->show();
}

bool MainWindow::event(QEvent *event) {
  switch (event->type()) {
  case QEvent::WindowActivate:
    updateTables();
    break;

  default:
    break;
  }

  return QMainWindow::event(event);
}

void MainWindow::on_tabWidget_currentChanged(const int) { updateTables(); }

void MainWindow::on_actionSobre_triggered() {
  QMessageBox::about(this, "Sobre ERP Staccato", "Versão " + qApp->applicationVersion() + "\nDesenvolvedor: Rodrigo Torres\nCelular/WhatsApp: (12)98138-3504\nE-mail: torres.dark@gmail.com");
}

void MainWindow::on_actionClaro_triggered() { qApp->lightTheme(); }

void MainWindow::on_actionEscuro_triggered() { qApp->darkTheme(); }

void MainWindow::on_actionConfiguracoes_triggered() {
  auto *config = new UserConfig(this);
  config->setAttribute(Qt::WA_DeleteOnClose);
  config->show();
}

void MainWindow::on_actionCalculadora_triggered() { QDesktopServices::openUrl(QUrl::fromLocalFile(R"(C:\Windows\System32\calc.exe)")); }

void MainWindow::on_actionProdutos_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarProduto();
}

void MainWindow::on_actionEstoque_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarEstoque();
}

void MainWindow::on_actionPromocao_triggered() {
  auto *importa = new ImportaProdutos(this);
  importa->setAttribute(Qt::WA_DeleteOnClose);
  importa->importarPromocao();
}

// TODO: 0montar relatorio dos caminhoes com graficos e total semanal, mensal, custos etc
// NOTE: colocar logo da staccato na mainwindow

// NOTE: prioridades atuais:
// TODO: logistica da devolucao

void MainWindow::on_actionGerenciar_preco_estoque_triggered() {
  auto *estoque = new PrecoEstoque(this);
  estoque->setAttribute(Qt::WA_DeleteOnClose);
  estoque->show();
}

void MainWindow::on_actionCalcular_frete_triggered() {
  auto *frete = new CalculoFrete(this);
  frete->setAttribute(Qt::WA_DeleteOnClose);
  frete->show();
}

// TASK: cancelamento de nfe: terminar de arrumar formato do email
// TASK: arrumar cadastrarNFe para quando guardar a nota pendente associar ela com venda_has_produto para aparecer na tela de consultarNFe (depois disso só vai precisar atualizar a nota com a
// autorizacao e os status)
// TASK: verificar porque os estoques 10649, 10650 e 10651 nao mudaram de status (pararam em 'em coleta')
// TASK: anotar alteracoes que Anderson pediu nos audios do whats
// TASK: ao cancelar a nota verificar se todos os campos relacionados foram corrigidos e enviar email para contabilidade com xml de canc.
// TASK: arrumar items no workbench na tabela pf que possuam idVendaProduto mas nao idVenda
// TASK: terminar a parte de alteracao de certificado
//         alterar emitente
//         pedir para alterar cartao
// TASK: -reescrever view_estoque para retroativo (usar view_estoque2)
// TASK: botao de consultarNFe nao esta atualizando corretamente o xml
// TASK: protocolo entrega (falta o Anderson validar antes de integrar com a geracao da nota)
// TASK: verificar com Conrado os itens com minimo mas sem multiplo (tabela produto)
// TASK: verificar load balancing com proxysql
// TASK: montar chart do faturamento dia/mes (foto no skype)
// TASK: caixinha na tabela 'agendar entrega' para marcar quais pedidos foram enviados pelo anderson para a edna
// TASK: arrumar consumos em que as unidades do estoque estejam diferentes das do consumo (converter)
// TASK: pendencias conrado - nfe 118248
// TASK: terminar funcao de marcar caixas quebradas no recebimento
// TASK: verificar os 3 pedidos com totalItem que nao corresponde ao valor correto
// TASK: bloquear acesso dos usuarios apenas pela intranet (permissoes mysql) precisa de ip fixo primeiro
// TASK: quando muda a validade de um produto descontinuado ele continua descontinuado porque o sistema leva em consideracao o produto_has_preco
// TASK: alterar consumo de estoque para fazer as ligacoes idVenda/idVendaProduto na tabela de compra
// TASK: terminar de implantar quebra/reposicao
// TASK: reimportar notas do pedido 172646
// TODO: na reposicao concatenar '(REPOSICAO)' no comeco da descricao do produto
// REFAC: make a code that checks if the program is inside a transaction and if it is it postpones showing error messages
// TODO: diff defaultPalette and darkPalette to find the stuff that is missing implementing
// TODO: verificar no banco de dados as tabelas que usam coluna 'caixas', tem varias linhas com casas decimais (4.999999999), verificar os calculos

// NOTE: *quebralinha venda_produto/pedido_fornecedor
// *compra/consumo parcial
// *agendamento parcial
// *devolucao (separar no pf em 2 linhas, uma mantem o vinculo da parte que foi entregue e a outra fica sem vinculo da porte que foi devolvida)
// *quebra/reposicao
