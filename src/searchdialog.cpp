#include "searchdialog.h"
#include "ui_searchdialog.h"

#include "application.h"
#include "doubledelegate.h"
#include "file.h"
#include "porcentagemdelegate.h"
#include "produtoproxymodel.h"
#include "reaisdelegate.h"
#include "user.h"
#include "xml.h"

#include <QAuthenticator>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QNetworkReply>
#include <QSqlError>
#include <QSqlRecord>

SearchDialog::SearchDialog(const QString &title, const QString &table, const QString &primaryKey, const QStringList &textKeys, const QString &fullTextIndex, const QString &filter,
                           const QString &sortColumn, const bool naoListar, QWidget *parent)
    : QDialog(parent), naoListarBuscaVazia(naoListar), fullTextIndex(fullTextIndex), primaryKey(primaryKey), filter(filter), textKeys(textKeys), model(1000), ui(new Ui::SearchDialog) {
  ui->setupUi(this);

  timer.setSingleShot(true);

  setWindowTitle(title);
  setWindowModality(Qt::NonModal);
  setWindowFlags(Qt::Window);

  setupTables(table, sortColumn);

  if (fullTextIndex.isEmpty()) {
    ui->lineEditBusca->hide();
    ui->labelBusca->hide();
  }

  ui->legendaEstoque->hide();
  ui->legendaPromocao->hide();
  ui->legendaStaccatoOFF->hide();
  ui->pushButtonModelo3d->hide();
  ui->radioButtonProdAtivos->hide();
  ui->radioButtonProdDesc->hide();
  ui->treeView->hide();

  ui->lineEditBusca->setFocus();

  setConnections();
}

SearchDialog::~SearchDialog() { delete ui; }

void SearchDialog::setConnections() {
  const auto connectionType = static_cast<Qt::ConnectionType>(Qt::AutoConnection | Qt::UniqueConnection);

  connect(&timer, &QTimer::timeout, this, &SearchDialog::on_lineEditBusca_textChanged, connectionType);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &SearchDialog::delayFiltro, connectionType);
  connect(ui->pushButtonModelo3d, &QPushButton::clicked, this, &SearchDialog::on_pushButtonModelo3d_clicked, connectionType);
  connect(ui->pushButtonSelecionar, &QPushButton::clicked, this, &SearchDialog::on_pushButtonSelecionar_clicked, connectionType);
  connect(ui->radioButtonProdAtivos, &QRadioButton::clicked, this, &SearchDialog::on_radioButtonProdAtivos_toggled, connectionType);
  connect(ui->radioButtonProdDesc, &QRadioButton::clicked, this, &SearchDialog::on_radioButtonProdDesc_toggled, connectionType);
  connect(ui->table, &TableView::clicked, this, &SearchDialog::on_table_clicked, connectionType);
  connect(ui->table, &TableView::doubleClicked, this, &SearchDialog::on_table_doubleClicked, connectionType);
}

void SearchDialog::delayFiltro() { timer.start(qApp->delayedTimer); }

void SearchDialog::setupTables(const QString &table, const QString &sortColumn) {
  model.setTable(table);

  setFilter(filter);
  if (naoListarBuscaVazia) { model.setFilter("0"); }
  if (table == "profissional") { model.setFilter("idProfissional = 1"); }

  if (table == "view_produto") { model.proxyModel = new ProdutoProxyModel(&model, this); }

  ui->table->setModel(&model);

  ui->table->setItemDelegate(new DoubleDelegate(this));

  if (not sortColumn.isEmpty()) { model.setSort(sortColumn); }

  model.select();
}

void SearchDialog::buscaProduto(const QString &searchFilter) {
  const QString descontinuado = " AND descontinuado = " + QString(ui->radioButtonProdAtivos->isChecked() ? "FALSE" : "TRUE");
  const QString representacao = (showAllProdutos) ? "" : (isRepresentacao ? " AND representacao = TRUE" : " AND representacao = FALSE");
  const QString showEstoque = (compraAvulsa) ? " AND estoque = FALSE AND promocao <= 1" : "";

  model.setFilter(searchFilter + descontinuado + " AND desativado = FALSE" + representacao + showEstoque + fornecedorRep);
}

void SearchDialog::on_lineEditBusca_textChanged() {
  const QString text = qApp->sanitizeSQL(ui->lineEditBusca->text());

  if (text.isEmpty() and model.tableName() == "profissional") { return model.setFilter("idProfissional = 1"); }

  if (text.isEmpty()) { return model.setFilter(naoListarBuscaVazia ? "0" : filter); }

  // ----------------------------------------

  // MySQL Full-Text doesn't support prefix, use LIKE instead

  const QStringList textStrings = text.split(" ", Qt::SkipEmptyParts);

  QStringList filtroLike;

  const auto ftKeys = fullTextIndex.split(", ", Qt::SkipEmptyParts);

  for (const auto &ftIndex : ftKeys) {
    QStringList parteFiltro;

    for (const auto &palavra : textStrings) { parteFiltro << ftIndex + " LIKE '%" + palavra + "%'"; }

    filtroLike << ("(" + parteFiltro.join(" AND ") + ")");
  }

  QString searchFilter = filtroLike.join(" OR ").prepend("(").append(")");

  // ----------------------------------------

  if (model.tableName() == "view_produto") { return buscaProduto(searchFilter); }

  if (not filter.isEmpty()) { searchFilter.append(" AND (" + filter + ")"); }

  model.setFilter(searchFilter);
}

void SearchDialog::sendUpdateMessage(const QModelIndex &index) { emit itemSelected(model.data(index.row(), primaryKey)); }

void SearchDialog::on_table_clicked(const QModelIndex &index) {
  if (model.tableName() == "view_nfe_inutilizada" and index.isValid()) {
    XML *xml = new XML(model.data(index.row(), "xml").toByteArray(), XML::Tipo::Nulo, this);
    ui->treeView->setModel(&xml->model);
    ui->treeView->expandAll();
  }
}

void SearchDialog::on_table_doubleClicked() { on_pushButtonSelecionar_clicked(); }

void SearchDialog::setFilter(const QString &newFilter) {
  filter = newFilter;
  model.setFilter(filter);
}

void SearchDialog::hideColumns(const QStringList &columns) {
  for (const auto &column : columns) { ui->table->hideColumn(column); }
}

void SearchDialog::on_pushButtonSelecionar_clicked() {
  // TODO: if rowCount == 1 select first line with enter?

  const auto selection = ui->table->selectionModel()->selection().indexes();

  if (selection.isEmpty()) { return; }

  if (not permitirDescontinuados and ui->radioButtonProdDesc->isChecked()) { throw RuntimeError("Não pode selecionar produtos descontinuados!\nEntre em contato com o Dept. de Compras!", this); }

  if (model.tableName() == "view_produto") {
    const bool isEstoque = model.data(selection.first().row(), "estoque").toBool();

    if (not silent and isEstoque) { qApp->enqueueWarning("Verificar com o Dept. de Compras a disponibilidade do estoque antes de vender!", this); }
  }

  close();
  sendUpdateMessage(selection.first());

  ui->lineEditBusca->setFocus();
  ui->lineEditBusca->clear();
}

QString SearchDialog::getText(const QVariant &id) {
  if (id.toInt() == 0) { return QString(); }
  if (model.tableName().contains("endereco") and id.toInt() == 1) { return "NÃO HÁ/RETIRA"; }

  QString queryText;

  for (const auto &key : std::as_const(textKeys)) { queryText += queryText.isEmpty() ? key : ", " + key; }

  queryText = "SELECT " + queryText + " FROM " + model.tableName() + " WHERE " + primaryKey + " = '" + id.toString() + "'";

  if (caching and cacheSearchDialog.contains(queryText)) { return cacheSearchDialog.value(queryText); }

  SqlQuery query;

  if (not query.exec(queryText)) { throw RuntimeException("Erro na query getText: " + query.lastError().text(), this); }

  if (not query.first()) { throw RuntimeException("Texto não encontrado para id: " + id.toString(), this); }

  QString res;

  for (const auto &key : std::as_const(textKeys)) {
    if (query.value(key).isValid() and not query.value(key).toString().isEmpty()) { res += (res.isEmpty() ? "" : " - ") + query.value(key).toString(); }
  }

  if (caching) { cacheSearchDialog.insert(queryText, res); }

  return res;
}

void SearchDialog::setHeaderData(const QString &column, const QString &newHeader) { model.setHeaderData(column, newHeader); }

SearchDialog *SearchDialog::cliente(QWidget *parent) {
  auto *sdCliente = new SearchDialog("Buscar Cliente", "cliente", "idCliente", {"nome_razao"}, "nome_razao, nomeFantasia, cpf, cnpj", "desativado = FALSE", "nome_razao", true, parent);

  sdCliente->hideColumns({"idCliente", "inscEstadual", "credito", "idUsuarioRel", "idCadastroRel", "idProfissionalRel", "incompleto", "desativado"});

  sdCliente->setHeaderData("pfpj", "Tipo");
  sdCliente->setHeaderData("nome_razao", "Cliente");
  sdCliente->setHeaderData("cpf", "CPF");
  sdCliente->setHeaderData("cnpj", "CNPJ");
  sdCliente->setHeaderData("dataNasc", "Data Nasc.");
  sdCliente->setHeaderData("contatoNome", "Nome - Contato");
  sdCliente->setHeaderData("contatoCPF", "CPF - Contato");
  sdCliente->setHeaderData("contatoApelido", "Apelido - Contato");
  sdCliente->setHeaderData("contatoRG", "RG - Contato");
  sdCliente->setHeaderData("nomeFantasia", "Fantasia/Apelido");
  sdCliente->setHeaderData("tel", "Tel.");
  sdCliente->setHeaderData("telCel", "Tel. Cel.");
  sdCliente->setHeaderData("telCom", "Tel. Com.");
  sdCliente->setHeaderData("idNextel", "id Nextel");
  sdCliente->setHeaderData("nextel", "Nextel");
  sdCliente->setHeaderData("email", "E-mail");

  sdCliente->ui->lineEditBusca->setPlaceholderText("Cliente/Fantasia/CPF/CNPJ");

  return sdCliente;
}

SearchDialog *SearchDialog::conta(QWidget *parent) {
  auto *sdConta = new SearchDialog("Buscar Conta", "loja_has_conta", "idConta", {"banco", "agencia", "conta"}, {}, "", "banco", false, parent);

  sdConta->hideColumns({"idConta", "idLoja", "desativado"});

  sdConta->setHeaderData("banco", "Banco");
  sdConta->setHeaderData("agencia", "Agência");
  sdConta->setHeaderData("conta", "Conta");

  return sdConta;
}

SearchDialog *SearchDialog::enderecoCliente(QWidget *parent) {
  auto *sdEndereco = new SearchDialog("Buscar Endereço", "cliente_has_endereco", "idEndereco", {"logradouro", "numero", "bairro", "cidade", "uf"}, {}, "idEndereco = 1", "", false, parent);

  sdEndereco->hideColumns({"idEndereco", "idCliente", "codUF", "desativado"});

  sdEndereco->setHeaderData("descricao", "Descrição");
  sdEndereco->setHeaderData("cep", "CEP");
  sdEndereco->setHeaderData("logradouro", "Logradouro");
  sdEndereco->setHeaderData("numero", "Número");
  sdEndereco->setHeaderData("complemento", "Comp.");
  sdEndereco->setHeaderData("bairro", "Bairro");
  sdEndereco->setHeaderData("cidade", "Cidade");
  sdEndereco->setHeaderData("uf", "UF");

  return sdEndereco;
}

SearchDialog *SearchDialog::fornecedor(QWidget *parent) {
  auto *sdFornecedor = new SearchDialog("Buscar Fornecedor", "fornecedor", "idFornecedor", {"nomeFantasia", "razaoSocial"}, "razaoSocial, nomeFantasia, contatoCPF, cnpj", "desativado = FALSE",
                                        "razaoSocial", false, parent);

  sdFornecedor->hideColumns({"aliquotaSt", "comissao1", "comissao2",     "comissaoLoja",  "desativado", "email", "idFornecedor", "idNextel", "inscEstadual", "nextel",    "representacao", "st", "tel",
                             "telCel",     "telCom",    "especialidade", "fretePagoLoja", "vemDoSul",   "banco", "agencia",      "cc",       "poupanca",     "nomeBanco", "cnpjBanco"});

  sdFornecedor->setHeaderData("razaoSocial", "Razão Social");
  sdFornecedor->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdFornecedor->setHeaderData("contatoNome", "Nome do Contato");
  sdFornecedor->setHeaderData("cnpj", "CNPJ");
  sdFornecedor->setHeaderData("contatoCPF", "CPF do Contato");
  sdFornecedor->setHeaderData("contatoApelido", "Apelido do Contato");
  sdFornecedor->setHeaderData("contatoRG", "RG do Contato");
  sdFornecedor->setHeaderData("validadeProdutos", "Validade Produtos");

  sdFornecedor->ui->lineEditBusca->setPlaceholderText("Razão Social/Nome Fantasia/CNPJ/CPF do Contato");

  return sdFornecedor;
}

SearchDialog *SearchDialog::loja(QWidget *parent) {
  auto *sdLoja = new SearchDialog("Buscar Loja", "loja", "idLoja", {"nomeFantasia"}, "descricao, nomeFantasia, razaoSocial", "desativado = FALSE", "descricao", false, parent);

  sdLoja->ui->table->setItemDelegateForColumn("porcentagemFrete", new PorcentagemDelegate(false, parent));
  sdLoja->ui->table->setItemDelegateForColumn("valorMinimoFrete", new ReaisDelegate(parent));

  sdLoja->hideColumns({"idLoja", "codUF", "desativado", "certificadoSerie", "certificadoSenha", "porcentagemPIS", "porcentagemCOFINS", "custoTransporteTon", "custoTransporte1", "custoTransporte2",
                       "custoFuncionario", "ultimoNSU", "maximoNSU"});

  sdLoja->setHeaderData("descricao", "Descrição");
  sdLoja->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdLoja->setHeaderData("razaoSocial", "Razão Social");
  sdLoja->setHeaderData("tel", "Tel.");
  sdLoja->setHeaderData("tel2", "Tel. 2");
  sdLoja->setHeaderData("inscEstadual", "Insc. Est.");
  sdLoja->setHeaderData("sigla", "Sigla");
  sdLoja->setHeaderData("cnpj", "CNPJ");
  sdLoja->setHeaderData("porcentagemFrete", "Frete");
  sdLoja->setHeaderData("valorMinimoFrete", "Mínimo Frete");

  sdLoja->ui->lineEditBusca->setPlaceholderText("Descrição/Nome Fantasia/Razão Social");

  return sdLoja;
}

SearchDialog *SearchDialog::nfe(QWidget *parent) {
  auto *sdNFe = new SearchDialog("Buscar NFe", "view_nfe_inutilizada", "idNFe", {"chaveAcesso"}, "numeroNFe, xml, chaveAcesso", "", "", true, parent);

  sdNFe->ui->lineEditBusca->show();

  sdNFe->ui->table->setAutoResize(false);

  sdNFe->hideColumns({"idNFe", "infCpl", "xml"});
  sdNFe->ui->table->showColumn("created");

  sdNFe->setHeaderData("numeroNFe", "NFe");
  sdNFe->setHeaderData("status", "Status");
  sdNFe->setHeaderData("chaveAcesso", "Chave Acesso");
  sdNFe->setHeaderData("infCpl", "Inf. Comp.");
  sdNFe->setHeaderData("emitente", "Emitente");
  sdNFe->setHeaderData("created", "Data");

  sdNFe->ui->treeView->show();

  sdNFe->ui->lineEditBusca->setPlaceholderText("Número NFe/Chave Acesso/XML");

  sdNFe->resize(1172, 783);

  return sdNFe;
}

SearchDialog *SearchDialog::produto(const bool permitirDescontinuados, const bool silent, const bool showAllProdutos, const bool compraAvulsa, QWidget *parent) {
  // TODO: 3nao mostrar promocao vencida no descontinuado
  auto *sdProd = new SearchDialog("Buscar Produto", "view_produto", "idProduto", {"descricao"}, "fornecedor, descricao, colecao, codcomercial", "idProduto = 0", "", true, parent);

  sdProd->permitirDescontinuados = permitirDescontinuados;
  sdProd->silent = silent;
  sdProd->showAllProdutos = showAllProdutos;
  sdProd->compraAvulsa = compraAvulsa;

  if (compraAvulsa) { sdProd->hideColumns({"statusEstoque", "estoqueRestante", "lote"}); }

  sdProd->hideColumns({"desativado", "descontinuado", "estoque", "promocao", "idProduto", "representacao"});

  for (int column = 0, columnCount = sdProd->model.columnCount(); column < columnCount; ++column) {
    if (sdProd->model.record().fieldName(column).endsWith("Upd")) { sdProd->ui->table->setColumnHidden(column, true); }
  }

  sdProd->setHeaderData("fornecedor", "Fornecedor");
  sdProd->setHeaderData("statusEstoque", "Estoque");
  sdProd->setHeaderData("descricao", "Descrição");
  sdProd->setHeaderData("estoqueRestante", "Estoque Disp.");
  sdProd->setHeaderData("estoqueCaixa", "Estoque Cx.");
  sdProd->setHeaderData("lote", "Lote");
  sdProd->setHeaderData("un", "Un.");
  sdProd->setHeaderData("un2", "Un.2");
  sdProd->setHeaderData("colecao", "Coleção");
  sdProd->setHeaderData("tipo", "Tipo");
  sdProd->setHeaderData("minimo", "Mínimo");
  sdProd->setHeaderData("multiplo", "Múltiplo");
  sdProd->setHeaderData("m2cx", "M/Cx.");
  sdProd->setHeaderData("pccx", "Pç./Cx.");
  sdProd->setHeaderData("kgcx", "Kg./Cx.");
  sdProd->setHeaderData("formComercial", "Form. Com.");
  sdProd->setHeaderData("codComercial", "Cód. Com.");
  sdProd->setHeaderData("precoVenda", "R$");
  sdProd->setHeaderData("validade", "Validade");
  sdProd->setHeaderData("ui", "UI");

  sdProd->ui->legendaEstoque->show();
  sdProd->ui->legendaPromocao->show();
  sdProd->ui->legendaStaccatoOFF->show();
  sdProd->ui->pushButtonModelo3d->show();
  sdProd->ui->radioButtonProdAtivos->show();
  sdProd->ui->radioButtonProdDesc->show();

  sdProd->ui->radioButtonProdAtivos->setChecked(true);

  sdProd->ui->lineEditBusca->setPlaceholderText("Fornecedor/Descrição/Coleção/Código Comercial");

  return sdProd;
}

SearchDialog *SearchDialog::profissional(const bool mostrarNaoHa, QWidget *parent) {
  const QString filtroNaoHa = (mostrarNaoHa) ? "" : " AND idProfissional NOT IN (1)";

  auto *sdProfissional =
      new SearchDialog("Buscar Profissional", "profissional", "idProfissional", {"nome_razao"}, "nome_razao, tipoProf", "desativado = FALSE" + filtroNaoHa, "nome_razao", true, parent);

  sdProfissional->hideColumns({"idLoja", "idUsuarioRel", "idProfissional", "inscEstadual", "contatoNome", "contatoCPF", "contatoApelido", "contatoRG", "aniversario", "banco", "agencia", "cc", "nomeBanco",
                               "cpfBanco", "desativado", "comissao", "poupanca", "cnpjBanco"});

  sdProfissional->setHeaderData("pfpj", "Tipo");
  sdProfissional->setHeaderData("nome_razao", "Profissional");
  sdProfissional->setHeaderData("nomeFantasia", "Fantasia/Apelido");
  sdProfissional->setHeaderData("cpf", "CPF");
  sdProfissional->setHeaderData("cnpj", "CNPJ");
  sdProfissional->setHeaderData("tel", "Tel.");
  sdProfissional->setHeaderData("telCel", "Tel. Cel.");
  sdProfissional->setHeaderData("telCom", "Tel. Com.");
  sdProfissional->setHeaderData("idNextel", "id Nextel");
  sdProfissional->setHeaderData("nextel", "Nextel");
  sdProfissional->setHeaderData("email", "E-mail");
  sdProfissional->setHeaderData("tipoProf", "Profissão");

  sdProfissional->ui->lineEditBusca->setPlaceholderText("Profissional/Profissão");

  return sdProfissional;
}

SearchDialog *SearchDialog::transportadora(QWidget *parent) {
  auto *sdTransportadora =
      new SearchDialog("Buscar Transportadora", "transportadora", "idTransportadora", {"nomeFantasia"}, "razaoSocial, nomeFantasia", "desativado = FALSE", "razaoSocial", false, parent);

  sdTransportadora->hideColumns({"idTransportadora", "desativado"});

  sdTransportadora->setHeaderData("razaoSocial", "Razão Social");
  sdTransportadora->setHeaderData("nomeFantasia", "Nome Fantasia");
  sdTransportadora->setHeaderData("cnpj", "CNPJ");
  sdTransportadora->setHeaderData("inscEstadual", "Insc. Est.");
  sdTransportadora->setHeaderData("antt", "ANTT");
  sdTransportadora->setHeaderData("tel", "Tel.");

  sdTransportadora->ui->lineEditBusca->setPlaceholderText("Razão Social/Nome Fantasia");

  return sdTransportadora;
}

SearchDialog *SearchDialog::usuario(QWidget *parent) {
  auto *sdUsuario = new SearchDialog("Buscar Usuário", "view_usuario", "idUsuario", {"nome"}, "nome, tipo", "desativado = FALSE", "nome", false, parent);

  sdUsuario->hideColumns({"idLoja", "idUsuario", "user", "passwd", "especialidade", "desativado"});

  sdUsuario->setHeaderData("descricao", "Loja");
  sdUsuario->setHeaderData("tipo", "Função");
  sdUsuario->setHeaderData("nome", "Nome");
  sdUsuario->setHeaderData("email", "E-mail");

  sdUsuario->ui->lineEditBusca->setPlaceholderText("Função/Nome");

  return sdUsuario;
}

SearchDialog *SearchDialog::veiculo(QWidget *parent) {
  auto *sdVeiculo = new SearchDialog("Buscar Veículo", "view_busca_veiculo", "idVeiculo", {"razaoSocial", "modelo", "placa"}, "modelo, placa", "desativado = FALSE", "razaoSocial", false, parent);

  sdVeiculo->hideColumns({"idVeiculo", "desativado"});

  sdVeiculo->setHeaderData("razaoSocial", "Transportadora");
  sdVeiculo->setHeaderData("modelo", "Modelo");
  sdVeiculo->setHeaderData("capacidade", "Carga");
  sdVeiculo->setHeaderData("placa", "Placa");

  sdVeiculo->ui->lineEditBusca->setPlaceholderText("Transportadora/Modelo/Placa");

  return sdVeiculo;
}

SearchDialog *SearchDialog::vendedor(QWidget *parent) {
  const QString filtro = "tipo IN ('VENDEDOR', 'VENDEDOR ESPECIAL') AND desativado = FALSE";
  const QString filtroLoja = (User::idLoja == "1" or User::isEspecial()) ? "" : " AND idLoja = " + User::idLoja;
  const QString filtroAdmin = (User::isAdministrativo()) ? "" : " AND nome != 'REPOSIÇÂO'";

  auto *sdVendedor = new SearchDialog("Buscar Vendedor", "usuario", "idUsuario", {"nome"}, "nome, tipo", filtro + filtroLoja + filtroAdmin, "nome", false, parent);

  sdVendedor->hideColumns({"idUsuario", "idLoja", "user", "passwd", "password", "especialidade", "regime", "banco", "agencia", "cc", "poupanca", "nomeBanco", "cpfBanco", "cnpjBanco", "desativado"});

  sdVendedor->setHeaderData("tipo", "Função");
  sdVendedor->setHeaderData("nome", "Nome");
  sdVendedor->setHeaderData("email", "E-mail");

  sdVendedor->ui->lineEditBusca->setPlaceholderText("Função/Nome");

  return sdVendedor;
}

SearchDialog *SearchDialog::getCacheConta() {
  if (not cacheConta) {
    cacheConta = conta(nullptr);
    cacheConta->caching = true;
  }

  return cacheConta;
}

void SearchDialog::clearCache() { cacheSearchDialog.clear(); }

SearchDialog *SearchDialog::getCacheLoja() {
  if (not cacheLoja) {
    cacheLoja = loja(nullptr);
    cacheLoja->caching = true;
  }

  return cacheLoja;
}

void SearchDialog::setFornecedorRep(const QString &newFornecedorRep) { fornecedorRep = newFornecedorRep.isEmpty() ? "" : " AND fornecedor = '" + newFornecedorRep + "'"; }

QString SearchDialog::getFilter() const { return filter; }

void SearchDialog::setRepresentacao(const bool isRepresentacao) {
  this->isRepresentacao = isRepresentacao;
  on_lineEditBusca_textChanged();
}

void SearchDialog::on_radioButtonProdAtivos_toggled() { on_lineEditBusca_textChanged(); }

void SearchDialog::on_radioButtonProdDesc_toggled() { on_lineEditBusca_textChanged(); }

void SearchDialog::on_pushButtonModelo3d_clicked() {
  const auto selection = ui->table->selectionModel()->selectedRows();

  if (selection.isEmpty()) { throw RuntimeError("Nenhuma linha selecionada!"); }

  const int row = selection.first().row();

  const QString ip = qApp->getWebDavIp();
  const QString fornecedor = model.data(row, "fornecedor").toString();
  const QString codComercial = model.data(row, "codComercial").toString();

  const QString url = "https://" + ip + "/webdav/METAIS_VIVIANE/MODELOS 3D/" + fornecedor + "/" + codComercial + ".skp";

  auto *manager = new QNetworkAccessManager(this);
  manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);

  connect(manager, &QNetworkAccessManager::authenticationRequired, this, [&](QNetworkReply *reply, QAuthenticator *authenticator) {
    Q_UNUSED(reply)

    authenticator->setUser(User::usuario);
    authenticator->setPassword(User::senha);
  });

  auto *reply = manager->get(QNetworkRequest(QUrl(url)));

  connect(reply, &QNetworkReply::finished, this, [=] {
    if (reply->error() != QNetworkReply::NoError) {
      if (reply->error() == QNetworkReply::ContentNotFoundError) { throw RuntimeError("Produto não possui modelo 3D!"); }

      throw RuntimeException("Erro ao baixar arquivo: " + reply->errorString(), this);
    }

    const QString filename = QDir::currentPath() + "/arquivos/" + url.split("/").last();

    File file(filename);

    if (not file.open(QFile::WriteOnly)) { throw RuntimeException("Erro abrindo arquivo para escrita: " + file.errorString(), this); }

    file.write(reply->readAll());

    file.close();

    if (not QDesktopServices::openUrl(QUrl::fromLocalFile(filename))) { throw RuntimeException("Não foi possível abrir o arquivo 3D!"); }
  });
}
