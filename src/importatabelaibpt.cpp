#include "importatabelaibpt.h"

#include "application.h"
#include "file.h"
#include "sqlquery.h"

#include <QFileDialog>
#include <QProgressDialog>
#include <QSqlError>
#include <QTextStream>

ImportaTabelaIBPT::ImportaTabelaIBPT(QWidget *parent) : QDialog(parent) { setupTables(); }

void ImportaTabelaIBPT::importar() {
  const QString filePath = QFileDialog::getOpenFileName(this, "Impotar tabela IBPT", "", "Excel (*.csv)");

  if (filePath.isEmpty()) { return; }

  QFileInfo fileInfo(filePath);

  if (not fileInfo.completeBaseName().contains("SP")) { throw RuntimeError("Tabela escolhida não é de SP!", this); }

  const QString versao = fileInfo.completeBaseName().right(6);

  SqlQuery queryVersao;

  if (not queryVersao.exec("SELECT versao FROM ibpt WHERE versao = '" + versao + "' LIMIT 1")) { throw RuntimeException("Erro verificando versão: " + queryVersao.lastError().text(), this); }

  if (queryVersao.size() == 1) { return qApp->enqueueInformation("Tabela já cadastrada!", this); }

  File file(filePath);

  if (not file.open(QFile::ReadOnly)) { throw RuntimeException("Erro abrindo arquivo para leitura: " + file.errorString(), this); }

  QProgressDialog progressDialog;
  progressDialog.reset();
  progressDialog.setCancelButton(nullptr);
  progressDialog.setLabelText("Processando...");
  progressDialog.setWindowTitle("ERP Staccato");
  progressDialog.setWindowModality(Qt::WindowModal);
  progressDialog.setMinimum(0);
  // TODO: implicit conversion qint64 -> int
  progressDialog.setMaximum(file.size() + 1);

  progressDialog.show();

  QTextStream stream(&file);

  const QString line1 = stream.readLine();

  if (line1 != "codigo;ex;tipo;descricao;nacionalfederal;importadosfederal;estadual;municipal;vigenciainicio;vigenciafim;chave;versao;fonte") { throw RuntimeError("Arquivo incompatível!", this); }

  const auto size = file.size();

  while (not stream.atEnd()) {
    QStringList line = stream.readLine().split(";");

    const int row = model.insertRowAtEnd();

    model.setData(row, "codigo", line.takeFirst());
    const QString excecao = line.takeFirst();
    model.setData(row, "excecao", excecao.isEmpty() ? 0 : excecao.toInt());
    model.setData(row, "tipo", line.takeFirst());
    model.setData(row, "descricao", line.takeFirst());
    model.setData(row, "nacionalfederal", line.takeFirst());
    model.setData(row, "importadosfederal", line.takeFirst());
    model.setData(row, "estadual", line.takeFirst());
    model.setData(row, "municipal", line.takeFirst());
    model.setData(row, "vigenciainicio", QDate::fromString(line.takeFirst(), "dd/MM/yyyy"));
    model.setData(row, "vigenciafim", QDate::fromString(line.takeFirst(), "dd/MM/yyyy"));
    model.setData(row, "chave", line.takeFirst());
    model.setData(row, "versao", line.takeFirst());

    const auto remains = file.bytesAvailable();
    // TODO: implicit conversion qint64 -> int
    progressDialog.setValue(size - remains);
  }

  qApp->startTransaction("ImportaTabelaIBPT::importar");

  model.submitAll();

  qApp->endTransaction();

  progressDialog.cancel();

  qApp->enqueueInformation("Tabela cadastrada com sucesso!", this);
}

void ImportaTabelaIBPT::setupTables() { model.setTable("ibpt"); }
