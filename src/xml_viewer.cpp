#include "xml_viewer.h"
#include "ui_xml_viewer.h"
#include "xml.h"

XML_Viewer::XML_Viewer(QWidget *parent) : QDialog(parent), ui(new Ui::XML_Viewer) {
  ui->setupUi(this);

  setWindowFlags(Qt::Window);
  setAttribute(Qt::WA_DeleteOnClose);

  ui->treeView->setModel(&model);
  ui->treeView->setUniformRowHeights(true);
  ui->treeView->setAnimated(true);
  ui->treeView->setEditTriggers(QTreeView::NoEditTriggers);
}

XML_Viewer::~XML_Viewer() { delete ui; }

void XML_Viewer::exibirXML(const QByteArray &fileContent) {
  if (fileContent.isEmpty()) return;

  XML xml(fileContent);
  xml.montarArvore(model);

  ui->treeView->expandAll();

  show();
}
