#include "widgetlogisticacalendario.h"
#include "ui_widgetlogisticacalendario.h"

#include "application.h"
#include "collapsiblewidget.h"

#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

WidgetLogisticaCalendario::WidgetLogisticaCalendario(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetLogisticaCalendario) { ui->setupUi(this); }

WidgetLogisticaCalendario::~WidgetLogisticaCalendario() { delete ui; }

void WidgetLogisticaCalendario::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &WidgetLogisticaCalendario::on_calendarWidget_selectionChanged, connectionType);
  connect(ui->checkBoxMostrarFiltros, &QCheckBox::toggled, this, &WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled, connectionType);
  connect(ui->groupBoxVeiculos, &QGroupBox::toggled, this, &WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled, connectionType);
  connect(ui->pushButtonAnterior, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonAnterior_clicked, connectionType);
  connect(ui->pushButtonProximo, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonProximo_clicked, connectionType);
}

void WidgetLogisticaCalendario::unsetConnections() {
  disconnect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &WidgetLogisticaCalendario::on_calendarWidget_selectionChanged);
  disconnect(ui->checkBoxMostrarFiltros, &QCheckBox::toggled, this, &WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled);
  disconnect(ui->groupBoxVeiculos, &QGroupBox::toggled, this, &WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled);
  disconnect(ui->pushButtonAnterior, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonAnterior_clicked);
  disconnect(ui->pushButtonProximo, &QPushButton::clicked, this, &WidgetLogisticaCalendario::on_pushButtonProximo_clicked);
}

void WidgetLogisticaCalendario::listarVeiculos() {
  QSqlQuery query;

  if (not query.exec("SELECT t.razaoSocial, tv.modelo FROM transportadora t LEFT JOIN transportadora_has_veiculo tv ON t.idTransportadora = tv.idTransportadora WHERE t.desativado = FALSE AND "
                     "tv.desativado = FALSE ORDER BY razaoSocial, modelo")) {
    return qApp->enqueueException("Erro buscando veiculos: " + query.lastError().text(), this);
  }

  while (query.next()) {
    auto *checkbox = new QCheckBox(this);
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

  if (not modelIsSet) { modelIsSet = true; }

  updateFilter();
}

void WidgetLogisticaCalendario::resetTables() { modelIsSet = false; }

void WidgetLogisticaCalendario::updateFilter() {
  const QDate date = ui->calendarWidget->selectedDate();
  updateCalendar(date.addDays(date.dayOfWeek() * -1));
}

void WidgetLogisticaCalendario::updateCalendar(const QDate &startDate) {
  ui->tableWidget->clearContents();

  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

  int veiculos = 0;
  const int start = startDate.day();

  QStringList list;

  const auto children = ui->groupBoxVeiculos->findChildren<QCheckBox *>();

  for (const auto &child : children) {
    if (not child->isChecked()) { continue; }

    veiculos++;

    QStringList temp = child->text().split(" / ");

    list << "Manhã\n" + temp.at(0).left(15) + "\n" + temp.at(1).left(15);
    list << "Tarde\n" + temp.at(0).left(15) + "\n" + temp.at(1).left(15);
  }

  qDebug() << list;

  //  ui->tableWidget->setRowCount(veiculos * 2); // manha/tarde
  ui->tableWidget->setRowCount(veiculos);
  ui->tableWidget->setVerticalHeaderLabels(list);

  ui->tableWidget->setColumnCount(7); // dias
  ui->tableWidget->setHorizontalHeaderLabels({"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"});

  int dia = start;
  const QDate date = ui->calendarWidget->selectedDate();
  const int diasMes = date.addDays(date.dayOfWeek() * -1).daysInMonth();

  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
    const auto item = ui->tableWidget->horizontalHeaderItem(col);
    item->setText(QString::number(dia) + " " + item->text());
    dia++;
    if (dia > diasMes) { dia = 1; }
  }

  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) { ui->tableWidget->setColumnHidden(col, true); }
  for (int row = 0; row < ui->tableWidget->rowCount(); ++row) { ui->tableWidget->setRowHidden(row, true); }

  QSqlQuery query;
  query.prepare("SELECT * FROM view_calendario WHERE DATE(data) BETWEEN :start AND :end");
  query.bindValue(":start", startDate);
  query.bindValue(":end", startDate.addDays(6));

  if (not query.exec()) { return qApp->enqueueException("Erro query: " + query.lastError().text(), this); }

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

    // TODO: make a column of CollapsibleWidgets in each cell

    qDebug() << query.value("data");

    const QString hora = query.value("data").toDateTime().toString("hh:mm") + " - " + query.value("status").toString();

    qDebug() << hora;

    auto *cell = ui->tableWidget->cellWidget(row, diaSemana);
    //    auto *widget = new CollapsibleWidget(hora, this);

    if (cell) {
      //      QFrame *frame = static_cast<QFrame *>(cell);
      //      frame->layout()->addWidget(widget);
      //      qDebug() << "a: " << frame->sizeHint();
      //      frame->adjustSize();
      //      qDebug() << "b: " << frame->sizeHint();
      auto *widget = static_cast<CollapsibleWidget *>(cell);
      widget->addButton();
    } else {
      //      auto *frame = new QFrame(this);
      //      frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      //      auto *layout = new QVBoxLayout(frame);
      //      scrollLayout->setSizeConstraint(QLayout::SetFixedSize);
      //      layout->addWidget(widget);
      //      ui->tableWidget->setCellWidget(row, diaSemana, frame);
      auto *newWidget = new CollapsibleWidget(hora, this);
      ui->tableWidget->setCellWidget(row, diaSemana, newWidget);
    }

    //    auto *widget = ui->tableWidget->cellWidget(row, diaSemana) ? static_cast<CollapsibleWidget *>(ui->tableWidget->cellWidget(row, diaSemana)) : new CollapsibleWidget(hora, this);

    QStringList enderecos = query.value("endereco").toString().replace("+", " ").replace(",", ", ").split("|");
    QString enderecosList;

    for (auto &endereco : enderecos) { enderecosList += QString(R"(<p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">%1</p>)").arg(endereco); }

    QString text;

    // TODO: get this from loja_has_endereco
    QString origem = "Rua+Sales&oacute;polis,27,Barueri,SP";
    QString destinos = query.value("endereco").toString();

    //    widget->setDestinos(destinos);

    // TODO: rota deve ser pushButton pois vai enviar requisição, não é URL pronta
    // TODO: enviar rota para a Directions API para otimização
    //       com o resultado, montar a URL Maps API

    text += QString(R"(<p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">Kg: %1, Cx.: %2</p>
                       <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">%3</p>
                       <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">Status: %4</p>
                       <p style="-qt-block-indent: 0; text-indent: 0px; margin: 0px;">
                       <a href="https://www.google.com/maps/dir/?api=1&amp;origin=%6&amp;destination=%5&amp;waypoints=%6
                       &amp;travelmode=driving" target="_blank" rel="noopener">Google Maps</a></p>)")
                .arg(query.value("kg").toString())
                .arg(query.value("caixas").toString())
                .arg(query.value("idVenda").toString().replace("|", "/"))
                //                .arg(enderecosList)
                .arg(query.value("status").toString())
                .arg(origem)
                .arg(destinos);

    ui->tableWidget->setRowHidden(row, false);
    ui->tableWidget->setColumnHidden(diaSemana, false);

    //    widget->setHtml(text);
    //    ui->tableWidget->setCellWidget(row, diaSemana, widget);
    //    connect(widget, &CollapsibleWidget::toggled, ui->tableWidget, &QTableWidget::resizeColumnsToContents);
    //    connect(widget, &CollapsibleWidget::toggled, ui->tableWidget, &QTableWidget::resizeRowsToContents);
  }

  ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  const QString range = startDate.toString("dd-MM-yyyy") + " - " + startDate.addDays(6).toString("dd-MM-yyyy");

  ui->lineEditRange->setText(range);
}

void WidgetLogisticaCalendario::on_checkBoxMostrarFiltros_toggled(bool checked) {
  ui->calendarWidget->setVisible(checked);
  ui->groupBoxVeiculos->setVisible(checked);
}

void WidgetLogisticaCalendario::on_pushButtonProximo_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(7)); }

void WidgetLogisticaCalendario::on_pushButtonAnterior_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(-7)); }

void WidgetLogisticaCalendario::on_calendarWidget_selectionChanged() { updateFilter(); }

void WidgetLogisticaCalendario::on_groupBoxVeiculos_toggled(const bool enabled) {
  unsetConnections();

  [&] {
    const auto children = ui->groupBoxVeiculos->findChildren<QCheckBox *>();

    for (const auto &child : children) {
      child->blockSignals(true);
      child->setEnabled(true);
      child->setChecked(enabled);
      child->blockSignals(false);
    }
  }();

  setConnections();

  updateTables();
}

// TODO: esconder veiculos que não possuem agendamento na semana
