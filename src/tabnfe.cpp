#include "tabnfe.h"
#include "ui_tabnfe.h"

TabNFe::TabNFe(QWidget *parent) : QWidget(parent), ui(new Ui::TabNFe) {
  ui->setupUi(this);
  setConnections();
}

TabNFe::~TabNFe() { delete ui; }

void TabNFe::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->tabWidgetNfe, &QTabWidget::currentChanged, this, &TabNFe::on_tabWidgetNfe_currentChanged, connectionType);
}

void TabNFe::updateTables() {
  const QString currentTab = ui->tabWidgetNfe->tabText(ui->tabWidgetNfe->currentIndex());

  if (currentTab == "Entrada") { ui->widgetEntrada->updateTables(); }
  if (currentTab == "Saída") { ui->widgetSaida->updateTables(); }
  if (currentTab == "Distribuição") { ui->widgetDistribuicao->updateTables(); }
}

void TabNFe::resetTables() {
  ui->widgetEntrada->resetTables();
  ui->widgetSaida->resetTables();
  ui->widgetDistribuicao->resetTables();
}

void TabNFe::on_tabWidgetNfe_currentChanged() { updateTables(); }

// TODO: colocar uma terceira aba para NF-es de produtos comprados em lojas de terceiro para reposicao (não devem entrar na tela de NF-es da staccato pois não entram no financeiro/não devem ter
// impostos calculados/não devem ser enviadas para a contabilidade) não deve criar estoque??
