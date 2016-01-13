#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "estoque.h"
#include "importarxml.h"
#include "ui_widgetestoque.h"
#include "widgetestoque.h"
#include "xml.h"

WidgetEstoque::WidgetEstoque(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetEstoque) {
  ui->setupUi(this);

  setupTables();
}

WidgetEstoque::~WidgetEstoque() { delete ui; }

void WidgetEstoque::setupTables() {
  model.setTable("view_estoque");
  model.setEditStrategy(QSqlTableModel::OnManualSubmit);

  ui->table->setModel(&model);
  ui->table->setItemDelegate(new DoubleDelegate(this));
}

QString WidgetEstoque::updateTables() {
  if (not model.select()) return "Erro lendo tabela estoque: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetEstoque::on_table_activated(const QModelIndex &index) {
  Estoque *estoque = new Estoque(this);
  estoque->viewRegisterById(model.data(index.row(), "Cód Com").toString());
}

void WidgetEstoque::on_pushButtonEntradaEstoque_clicked() {
  QString filePath = QFileDialog::getOpenFileName(this, "Importar arquivo XML", QDir::currentPath(), ("XML (*.xml)"));

  if (filePath.isEmpty()) return;

  QFile file(filePath);

  if (not file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this, "Erro!", "Erro lendo arquivo: " + file.errorString());
    return;
  }

  XML xml(file.readAll(), file.fileName());
  xml.cadastrarNFe("ENTRADA");

  updateTables();
}

void WidgetEstoque::on_pushButtonTesteFaturamento_clicked() {
  ImportarXML *import = new ImportarXML(this);
  import->showMaximized();
}

// NOTE: gerenciar lugares de estoque (cadastro/permissoes)
