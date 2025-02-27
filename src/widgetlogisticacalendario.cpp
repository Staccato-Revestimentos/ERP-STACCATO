#include "widgetlogisticacalendario.h"
#include "ui_widgetlogisticacalendario.h"

#include "application.h"
#include "collapsiblewidget.h"
#include "sqlquery.h"

#include <QCheckBox>
#include <QDebug>
#include <QSqlError>

WidgetLogisticaCalendario::WidgetLogisticaCalendario(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaCalendario) { ui->setupUi(this); }

WidgetLogisticaCalendario::~WidgetLogisticaCalendario() { delete ui; }

void WidgetLogisticaCalendario::setConnections() {
  if (not blockingSignals.isEmpty()) { blockingSignals.pop(); } // avoid crashing on first setConnections

  if (not blockingSignals.isEmpty()) { return; } // delay setting connections until last unset/set block

  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &WidgetLogisticaCalendario::on_calendarWidget_selectionChanged, connectionType);
  connect(ui->checkBoxMostrarFiltros, &QCheckBox::toggled, this, &WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled, connectionType);
  connect(ui->groupBoxVeiculos, &QGroupBox::toggled, this, &WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled, connectionType);
  connect(ui->pushButtonAnterior, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonAnterior_clicked, connectionType);
  connect(ui->pushButtonProximo, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonProximo_clicked, connectionType);
}

void WidgetLogisticaCalendario::unsetConnections() {
  blockingSignals.push(0);

  disconnect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &WidgetLogisticaCalendario::on_calendarWidget_selectionChanged);
  disconnect(ui->checkBoxMostrarFiltros, &QCheckBox::toggled, this, &WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled);
  disconnect(ui->groupBoxVeiculos, &QGroupBox::toggled, this, &WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled);
  disconnect(ui->pushButtonAnterior, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonAnterior_clicked);
  disconnect(ui->pushButtonProximo, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonProximo_clicked);
}

void WidgetLogisticaCalendario::listarVeiculos() {
  SqlQuery query;

  if (not query.exec("SELECT t.razaoSocial, tv.modelo FROM transportadora t LEFT JOIN transportadora_has_veiculo tv ON t.idTransportadora = tv.idTransportadora WHERE t.desativado = FALSE AND "
                     "tv.desativado = FALSE ORDER BY razaoSocial, modelo")) {
    throw RuntimeException("Erro buscando veiculos: " + query.lastError().text(), this);
  }

  int index = 1;

  while (query.next()) {
    auto *checkbox = new QCheckBox(this);
    checkbox->setObjectName("checkBox" + QString::number(index++));
    checkbox->setText(query.value("razaoSocial").toString() + " / " + query.value("modelo").toString());
    checkbox->setChecked(true);
    connect(checkbox, &QAbstractButton::toggled, this, &WidgetLogisticaCalendario::updateFilter);
    ui->groupBoxVeiculos->layout()->addWidget(checkbox);
  }

  ui->groupBoxVeiculos->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void WidgetLogisticaCalendario::updateTables() {
  if (not isSet) {
    listarVeiculos();
    setConnections();
    ui->checkBoxMostrarFiltros->toggle();
    isSet = true;
  }

  updateFilter();
}

void WidgetLogisticaCalendario::resetTables() {}

void WidgetLogisticaCalendario::updateFilter() {
  const QDate date = ui->calendarWidget->selectedDate();
  updateCalendar(date.addDays(date.dayOfWeek() * -1));
}

void WidgetLogisticaCalendario::updateCalendar(const QDate startDate) {
  ui->tableWidget->clearContents();

  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  int veiculos = 0;
  const int start = startDate.day();

  QStringList list;

  const auto children = ui->groupBoxVeiculos->findChildren<QCheckBox *>(QRegularExpression("checkBox"));

  for (const auto &child : children) {
    if (not child->isChecked()) { continue; }

    veiculos++;

    QStringList temp = child->text().split(" / ");

    list << "Manhã\n" + temp.at(0).left(15) + "\n" + temp.at(1).left(15);
    list << "Tarde\n" + temp.at(0).left(15) + "\n" + temp.at(1).left(15);
  }

  ui->tableWidget->setRowCount(veiculos * 2); // manha/tarde
  ui->tableWidget->setVerticalHeaderLabels(list);

  ui->tableWidget->setColumnCount(7); // dias
  ui->tableWidget->setHorizontalHeaderLabels({"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"});

  int dia = start;
  const QDate date = ui->calendarWidget->selectedDate();
  const int diasMes = date.addDays(date.dayOfWeek() * -1).daysInMonth();

  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
    auto *const item = ui->tableWidget->horizontalHeaderItem(col);
    item->setText(QString::number(dia) + " " + item->text());
    dia++;
    if (dia > diasMes) { dia = 1; }
  }

  SqlQuery query;
  query.prepare("SELECT * FROM view_calendario WHERE DATE(data) BETWEEN :start AND :end");
  query.bindValue(":start", startDate);
  query.bindValue(":end", startDate.addDays(6));

  if (not query.exec()) { throw RuntimeException("Erro query: " + query.lastError().text(), this); }

  while (query.next()) {
    const QString transportadora = query.value("razaoSocial").toString().left(15) + "\n" + query.value("modelo").toString().left(15);

    int row = -1;

    for (int i = 0; i < list.size(); ++i) {
      if (list.at(i).contains(transportadora)) {
        row = (query.value("data").toTime().hour() < 12) ? i : i + 1; // manha/tarde
        break;
      }
    }

    if (row == -1) { continue; }

    const int diaSemana = query.value("data").toDate().dayOfWeek();

    auto *widget = (ui->tableWidget->cellWidget(row, diaSemana)) ? dynamic_cast<CollapsibleWidget *>(ui->tableWidget->cellWidget(row, diaSemana)) : new CollapsibleWidget(this);

    const QString oldText = widget->getHtml();

    QString text = oldText.isEmpty() ? "" : oldText + R"(<p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;"> </p>
                                                         <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">-----------------------------------------</p>
                                                         <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;"> </p>)";

    QStringList produtos = query.value("produtos").toString().split("/");
    QString produtosList;

    for (auto &produto : produtos) { produtosList += QString(R"(<li style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">%1</li>)").arg(produto); }

    const QString origem = query.value("origem").toString();
    const QString destino = query.value("destino").toString();

    text += QString(R"(<p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">10:00 Kg: %1, Cx.: %2</p>
           <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">%3</p>
           <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">%4</p>
           <ul>
           %5
           </ul>
           <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">Status: %6</p>
           <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">
           <a href="https://www.google.com/maps/dir/?api=1&origin=%7&destination=%8&
           travelmode=driving" target="_blank" rel="noopener">Google Maps</a></p>)")
                .arg(query.value("kg").toString(), query.value("caixas").toString(), query.value("idVenda").toString(), query.value("bairro").toString() + " - " + query.value("cidade").toString(),
                     produtosList, query.value("status").toString(), origem, destino);

    widget->setHtml(text);
    ui->tableWidget->setCellWidget(row, diaSemana, widget);
    connect(widget, &CollapsibleWidget::toggled, ui->tableWidget, &QTableWidget::resizeColumnsToContents);
    connect(widget, &CollapsibleWidget::toggled, ui->tableWidget, &QTableWidget::resizeRowsToContents);
  }

  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  const QString range = startDate.toString("dd-MM-yyyy") + " - " + startDate.addDays(6).toString("dd-MM-yyyy");

  ui->lineEditRange->setText(range);
}

void WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled(const bool checked) {
  ui->calendarWidget->setVisible(checked);
  ui->groupBoxVeiculos->setVisible(checked);
}

void WidgetLogisticaCalendario::on_pushButtonProximo_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(7)); }

void WidgetLogisticaCalendario::on_pushButtonAnterior_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(-7)); }

void WidgetLogisticaCalendario::on_calendarWidget_selectionChanged() { updateFilter(); }

void WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled(const bool enabled) {
  unsetConnections();

  try {
    const auto children = ui->groupBoxVeiculos->findChildren<QCheckBox *>(QRegularExpression("checkBox"));

    for (const auto &child : children) {
      child->blockSignals(true);
      child->setEnabled(true);
      child->setChecked(enabled);
      child->blockSignals(false);
    }
  } catch (std::exception &) {
    setConnections();
    throw;
  }

  setConnections();

  updateTables();
}

// TODO: esconder veiculos que não possuem agendamento na semana
