#include <QMessageBox>
#include <QNetworkReply>
#include <QSqlError>
#include <QSqlQuery>
#include <QXmlStreamReader>

#include "calculofrete.h"
#include "ui_calculofrete.h"

CalculoFrete::CalculoFrete(QWidget *parent) : Dialog(parent), ui(new Ui::CalculoFrete) {
  ui->setupUi(this);

  ui->itemBoxCliente->setSearchDialog(SearchDialog::cliente(this));

  connect(&networkManager, &QNetworkAccessManager::finished, this, &CalculoFrete::handleNetworkData);
  connect(ui->itemBoxCliente, &ItemBox::textChanged, this, &CalculoFrete::on_itemBoxCliente_textChanged);
  connect(ui->pushButton, &QPushButton::clicked, this, &CalculoFrete::on_pushButton_clicked);
}

CalculoFrete::~CalculoFrete() { delete ui; }

void CalculoFrete::setCliente(const QVariant idCliente) {
  ui->itemBoxCliente->setValue(idCliente);
  //  on_itemBoxCliente_textChanged("");
}

double CalculoFrete::getDistancia() {
  const double distancia = ui->lineEditDistancia->text().remove(" km").replace(",", ".").toDouble();
  qDebug() << "double: " << distancia;

  return distancia;
}

void CalculoFrete::on_pushButton_clicked() {
  ui->lineEditDistancia->clear();

  const QString url = searchUrl.arg(ui->comboBoxOrigem->currentText().replace(" ", "+"), ui->comboBoxDestino->currentText().replace(" ", "+"));
  qDebug() << "url: " << url;
  QNetworkRequest request;
  request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
  request.setUrl(url);
  networkManager.get(request);
}

void CalculoFrete::handleNetworkData(QNetworkReply *networkReply) {
  if (networkReply->error() == QNetworkReply::NoError) {
    QByteArray response(networkReply->readAll());
    qDebug() << "response: " << response;
    QXmlStreamReader xml(response);
    while (!xml.atEnd()) {
      xml.readNext();
      if (xml.tokenType() == QXmlStreamReader::StartElement) {
        if (xml.name() == "origin_address") {
          xml.readNext();
          ui->comboBoxOrigem->setCurrentText(xml.text().toString());
        }

        if (xml.name() == "destination_address") {
          xml.readNext();
          ui->comboBoxDestino->setCurrentText(xml.text().toString());
        }

        if (xml.name() == "distance") {
          while (not xml.atEnd()) {
            xml.readNext();
            if (xml.tokenType() == QXmlStreamReader::StartElement) {
              if (xml.name() == "text") {
                xml.readNext();
                qDebug() << "distance: " << xml.text();
                ui->lineEditDistancia->setText(xml.text().toString());
              }
            }
          }
        }
      }
    }
  }

  networkReply->deleteLater();

  getDistancia();
}

void CalculoFrete::on_itemBoxCliente_textChanged(const QString &) {
  ui->comboBoxOrigem->clear();
  ui->comboBoxDestino->clear();
  ui->lineEditDistancia->clear();

  QSqlQuery query;
  query.prepare("SELECT logradouro, numero, cidade, uf FROM cliente_has_endereco WHERE idCliente = :idCliente");
  query.bindValue(":idCliente", ui->itemBoxCliente->getValue());

  if (not query.exec()) {
    emit errorSignal("Erro buscando endereço do cliente: " + query.lastError().text());
    return;
  }

  while (query.next()) {
    ui->comboBoxDestino->addItem(query.value("logradouro").toString() + ", " + query.value("numero").toString() + ", " + query.value("cidade").toString() + ", " + query.value("uf").toString());
  }

  ui->comboBoxOrigem->addItem("Rua Salesópolis, 27, Barueri, SP");
}
