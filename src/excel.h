#pragma once

#include "sqlquery.h"
#include "xlsxdocument.h"

class Excel final {

public:
  enum class Tipo { Orcamento, Venda };

  Excel(const QString &id, const Tipo tipo, QWidget *parent);

  auto gerarExcel(const int oc = 0, const bool isRepresentacao = false, const QString &representacao = QString()) -> void;
  auto getFileName() const -> QString;

private:
  // attributes
  QString const id;
  QString fileName;
  QWidget *parent;
  SqlQuery query;
  SqlQuery queryCliente;
  SqlQuery queryEndEnt;
  SqlQuery queryEndFat;
  SqlQuery queryLoja;
  SqlQuery queryLojaEnd;
  SqlQuery queryProduto;
  SqlQuery queryProfissional;
  SqlQuery queryVendedor;
  Tipo const tipo;
  // methods
  auto hideUnusedRows(QXlsx::Document &xlsx) -> void;
  auto setQuerys() -> void;
};
