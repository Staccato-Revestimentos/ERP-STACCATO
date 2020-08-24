#pragma once

#include <QTextStream>

class CNAB {

public:
  struct Gare {
    int idNFe;
    int mesAnoReferencia;
    int dataVencimento;
    int valor;
    QString numeroNF;
    QString cnpjOrig;
  };

  explicit CNAB(QWidget *parent);
  auto remessaGareSantander240(QVector<Gare> gares) -> std::optional<QString>;
  auto remessaGareItau240(QVector<Gare> gares) -> std::optional<QString>;
  auto retornoGareSantander240() -> void;
  auto retornoGareItau240(const QString &filePath) -> void;
  // TODO: adicionar funcoes para boleto e outros pagamentos
  ~CNAB() = default;

private:
  // attributes
  QWidget *parent;
  // methods
  auto decodeCodeItau(const QString &code) -> QString;
  auto writeBlanks(QTextStream &stream, const int count) -> void;
  auto writeNumber(QTextStream &stream, const int number, const int count) -> void;
  auto writeText(QTextStream &stream, const QString &text, const int count) -> void;
  auto writeZeros(QTextStream &stream, const int count) -> void;
};
