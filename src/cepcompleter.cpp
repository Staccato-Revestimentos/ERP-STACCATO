#include "cepcompleter.h"

#include "application.h"
#include "sqlquery.h"

#include <QSqlError>

void CepCompleter::buscaCEP(const QString &cep, QWidget *parent) {
  SqlQuery query;
  query.prepare("SELECT log_logradouro.log_tipo_logradouro, log_logradouro.log_no AS logradouro, log_bairro.bai_no AS bairro, log_localidade.loc_no AS cidade, log_localidade.ufe_sg AS uf, "
                "log_logradouro.cep FROM cep.`log_logradouro`, cep.`log_localidade`, cep.`log_bairro` WHERE log_logradouro.loc_nu_sequencial = log_localidade.loc_nu_sequencial AND "
                "log_logradouro.bai_nu_sequencial_ini = log_bairro.bai_nu_sequencial AND log_logradouro.cep = :cep");
  query.bindValue(":cep", QString(cep).remove("-"));

  if (not query.exec()) { throw RuntimeException("Erro buscando CEP: " + query.lastError().text(), parent); }

  if (not query.first()) { throw RuntimeError("CEP não encontrado!", parent); }

  cidade = query.value("cidade").toString();
  endereco = query.value("log_tipo_logradouro").toString() + " " + query.value("logradouro").toString();
  bairro = query.value("bairro").toString();
  uf = query.value("uf").toString();
}

QString CepCompleter::getUf() const { return uf; }

QString CepCompleter::getBairro() const { return bairro; }

QString CepCompleter::getEndereco() const { return endereco; }

QString CepCompleter::getCidade() const { return cidade; }
