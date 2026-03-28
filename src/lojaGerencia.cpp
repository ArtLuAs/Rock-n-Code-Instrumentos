#include "../headers/lojaGerencia.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

// ===================== CONEXÃO =====================

PGconn* conectar() {
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=loja_musical user=lojamusical_user password=SenhaSegura123");

    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Erro na conexao: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

// ===================== LOJA =====================

Loja::Loja(string nome, string cnpj, string endereco, string telefone) {
    this->nome = nome;
    this->cnpj = cnpj;
    this->endereco = endereco;
    this->telefone = telefone;
}

void Loja::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
    }
}

// ===================== FUNÇÕES AUXILIARES ESTÁTICAS =====================

static string normalizarTipo(string tipo) {
    size_t inicio = tipo.find_first_not_of(" \t\n\r");
    if (inicio == string::npos) return "";
    size_t fim = tipo.find_last_not_of(" \t\n\r");
    tipo = tipo.substr(inicio, fim - inicio + 1);
    transform(tipo.begin(), tipo.end(), tipo.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return tipo;
}

static bool tipoValido(const string& tipo) {
    return tipo == "guitarra" || tipo == "violao" || tipo == "baixo";
}

static string solicitarTipoValido() {
    string tipo;
    while (true) {
        cout << "Tipo (guitarra/violao/baixo): ";
        getline(cin, tipo);
        tipo = normalizarTipo(tipo);
        if (tipoValido(tipo)) return tipo;
        cout << "Tipo invalido. Digite apenas: guitarra, violao ou baixo." << endl;
    }
}

static string normalizarSexo(string sexo) {
    size_t inicio = sexo.find_first_not_of(" \t\n\r");
    if (inicio == string::npos) return "";
    size_t fim = sexo.find_last_not_of(" \t\n\r");
    sexo = sexo.substr(inicio, fim - inicio + 1);
    // Capitaliza a primeira letra
    if (!sexo.empty()) sexo[0] = toupper(sexo[0]);
    return sexo;
}

static bool sexoValido(const string& sexo) {
    return sexo == "Masculino" || sexo == "Feminino" || sexo == "Outro";
}

static string solicitarSexoValido() {
    string sexo;
    while (true) {
        cout << "Sexo (Masculino/Feminino/Outro): ";
        getline(cin, sexo);
        sexo = normalizarSexo(sexo);
        if (sexoValido(sexo)) return sexo;
        cout << "Sexo invalido. Digite: Masculino, Feminino ou Outro." << endl;
    }
}

// ===================== CRUD INSTRUMENTO =====================

void Loja::inserirInstrumento(PGconn* conn, Instrumento inst) {
    string precoStr = to_string(inst.getPreco());
    string qtdStr   = to_string(inst.getQuantidade());
    const char* p[5] = {
        inst.getNome().c_str(), inst.getTipo().c_str(),
        inst.getMarca().c_str(), precoStr.c_str(), qtdStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade) VALUES ($1,$2,$3,$4,$5)",
        5, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir instrumento");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Instrumento inserido com sucesso!" << endl;
    PQclear(res);
}

void Loja::alterarInstrumento(PGconn* conn, Instrumento inst) {
    string idStr    = to_string(inst.getId());
    string precoStr = to_string(inst.getPreco());
    string qtdStr   = to_string(inst.getQuantidade());
    const char* p[6] = {
        inst.getNome().c_str(), inst.getTipo().c_str(), inst.getMarca().c_str(),
        precoStr.c_str(), qtdStr.c_str(), idStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "UPDATE instrumentos SET nome=$1, tipo=$2, marca=$3, preco=$4, quantidade=$5 WHERE id=$6",
        6, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar instrumento");
    PQclear(res);
}

void Loja::pesquisarInstrumento(PGconn* conn, string nomeBusca) {
    string termo = "%" + nomeBusca + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos "
        "WHERE nome ILIKE $1 OR tipo ILIKE $1 OR marca ILIKE $1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar instrumento");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhum instrumento encontrado." << endl;
    for (int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5))).exibir();
    }
    PQclear(res);
}

void Loja::removerInstrumento(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM instrumentos WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover instrumento");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Instrumento removido com sucesso!" << endl;
    PQclear(res);
}

void Loja::listarInstrumentos(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos ORDER BY id;");
    checarErro(conn, res, "listar instrumentos");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5))).exibir();
    }
    PQclear(res);
}

void Loja::listarInstrumentosSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos ORDER BY id;");
    checarErro(conn, res, "listar instrumentos");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5))).exibirSimplificado();
    }
    PQclear(res);
}

void Loja::exibirInstrumento(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir instrumento");
    if (PQntuples(res) > 0) {
        Instrumento(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1),
                    PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3),
                    atof(PQgetvalue(res, 0, 4)), atoi(PQgetvalue(res, 0, 5))).exibir();
    } else {
        cout << "Instrumento nao encontrado." << endl;
    }
    PQclear(res);
}

bool Loja::possuiInstrumentosCadastrados(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM instrumentos;");
    checarErro(conn, res, "contar instrumentos");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== CRUD CLIENTE =====================

void Loja::inserirCliente(PGconn* conn, Cliente cli) {
    const char* p[5] = {
        cli.getNome().c_str(), cli.getCpf().c_str(), cli.getTelefone().c_str(),
        cli.getEmail().c_str(), cli.getSexo().c_str()
    };
    PGresult* res = PQexecParams(conn,
        "INSERT INTO clientes (nome, cpf, telefone, email, sexo) VALUES ($1,$2,$3,$4,$5)",
        5, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir cliente");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Cliente inserido com sucesso!" << endl;
    PQclear(res);
}

void Loja::alterarCliente(PGconn* conn, Cliente cli) {
    string idStr = to_string(cli.getId());
    const char* p[6] = {
        cli.getNome().c_str(), cli.getCpf().c_str(), cli.getTelefone().c_str(),
        cli.getEmail().c_str(), cli.getSexo().c_str(), idStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "UPDATE clientes SET nome=$1, cpf=$2, telefone=$3, email=$4, sexo=$5 WHERE id=$6",
        6, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar cliente");
    PQclear(res);
}

void Loja::pesquisarCliente(PGconn* conn, string nomeBusca) {
    string termo = "%" + nomeBusca + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, cpf, telefone, email, sexo FROM clientes "
        "WHERE nome ILIKE $1 OR cpf ILIKE $1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar cliente");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhum cliente encontrado." << endl;
    for (int i = 0; i < rows; i++) {
        Cliente(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibir();
    }
    PQclear(res);
}

void Loja::removerCliente(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM clientes WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover cliente");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Cliente removido com sucesso!" << endl;
    PQclear(res);
}

void Loja::listarClientes(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, cpf, telefone, email, sexo FROM clientes ORDER BY id;");
    checarErro(conn, res, "listar clientes");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Cliente(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibir();
    }
    PQclear(res);
}

void Loja::listarClientesSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, cpf, telefone, email, sexo FROM clientes ORDER BY id;");
    checarErro(conn, res, "listar clientes");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Cliente(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibirSimplificado();
    }
    PQclear(res);
}

void Loja::exibirCliente(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, cpf, telefone, email, sexo FROM clientes WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir cliente");
    if (PQntuples(res) > 0) {
        Cliente(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1),
                PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3),
                PQgetvalue(res, 0, 4), PQgetvalue(res, 0, 5)).exibir();
    } else {
        cout << "Cliente nao encontrado." << endl;
    }
    PQclear(res);
}

bool Loja::possuiClientesCadastrados(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM clientes;");
    checarErro(conn, res, "contar clientes");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== CRUD VENDA (cabeçalho) =====================

// Persiste apenas o cabeçalho e retorna o ID gerado pelo banco.
// Os itens devem ser inseridos depois via inserirItemVenda().
int Loja::inserirVenda(PGconn* conn, Venda venda) {
    string cliStr = to_string(venda.getClienteId());
    string valStr = to_string(venda.getValorTotal());
    const char* p[3] = {cliStr.c_str(), valStr.c_str(), venda.getData().c_str()};
    PGresult* res = PQexecParams(conn,
        "INSERT INTO vendas (cliente_id, valor_total, data) VALUES ($1,$2,$3) RETURNING id",
        3, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir venda");

    int novoId = 0;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0)
        novoId = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return novoId; // retorna o ID para que o chamador possa inserir os itens
}

// Atualiza apenas os campos do cabeçalho (data e valor_total).
// Para alterar itens, use removerItensVenda() + inserirItemVenda().
void Loja::alterarVenda(PGconn* conn, Venda venda) {
    string idStr  = to_string(venda.getId());
    string valStr = to_string(venda.getValorTotal());
    const char* p[3] = {valStr.c_str(), venda.getData().c_str(), idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "UPDATE vendas SET valor_total=$1, data=$2 WHERE id=$3",
        3, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar venda");
    PQclear(res);
}

void Loja::pesquisarVenda(PGconn* conn, string nomeCliente) {
    string termo = "%" + nomeCliente + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT v.id, v.cliente_id, v.valor_total, v.data "
        "FROM vendas v JOIN clientes c ON v.cliente_id = c.id "
        "WHERE c.nome ILIKE $1 ORDER BY v.id",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar venda");
    int rows = PQntuples(res);
    if (rows == 0) { cout << "Nenhuma venda encontrada." << endl; }
    for (int i = 0; i < rows; i++) {
        int vendaId = atoi(PQgetvalue(res, i, 0));
        Venda v(vendaId, atoi(PQgetvalue(res, i, 1)),
                PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 2)));
        listarItensVenda(conn, vendaId); // carrega itens na exibição
        v.exibir();
    }
    PQclear(res);
}

// Remove o cabeçalho; os itens são removidos em cascata pelo banco (ON DELETE CASCADE).
void Loja::removerVenda(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM vendas WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover venda");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Venda removida com sucesso!" << endl;
    PQclear(res);
}

void Loja::listarVendas(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, cliente_id, valor_total, data FROM vendas ORDER BY id;");
    checarErro(conn, res, "listar vendas");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        int vendaId = atoi(PQgetvalue(res, i, 0));
        Venda v(vendaId, atoi(PQgetvalue(res, i, 1)),
                PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 2)));
        listarItensVenda(conn, vendaId); // popula os itens antes de exibir
        v.exibir();
    }
    PQclear(res);
}

void Loja::listarVendasSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT id, data FROM vendas ORDER BY id;");
    checarErro(conn, res, "listar vendas");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Venda(atoi(PQgetvalue(res, i, 0)), 0,
              PQgetvalue(res, i, 1), 0.0).exibirSimplificado();
    }
    PQclear(res);
}

// Exibe cabeçalho + todos os itens da venda.
void Loja::exibirVenda(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, cliente_id, valor_total, data FROM vendas WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir venda");
    if (PQntuples(res) > 0) {
        Venda v(atoi(PQgetvalue(res, 0, 0)), atoi(PQgetvalue(res, 0, 1)),
                PQgetvalue(res, 0, 3), atof(PQgetvalue(res, 0, 2)));
        PQclear(res);
        listarItensVenda(conn, id); // carrega itens no objeto (via cout direto)
        v.exibir();
    } else {
        cout << "Venda nao encontrada." << endl;
        PQclear(res);
    }
}

bool Loja::possuiVendasCadastradas(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM vendas;");
    checarErro(conn, res, "contar vendas");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== CRUD ITEM VENDA =====================

void Loja::inserirItemVenda(PGconn* conn, ItemVenda item) {
    string vendaIdStr  = to_string(item.getVendaId());
    string instIdStr   = to_string(item.getInstrumentoId());
    string qtdStr      = to_string(item.getQuantidade());
    string precoStr    = to_string(item.getPrecoUnitario());
    const char* p[4]   = {vendaIdStr.c_str(), instIdStr.c_str(), qtdStr.c_str(), precoStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "INSERT INTO itens_venda (venda_id, instrumento_id, quantidade, preco_unitario) "
        "VALUES ($1,$2,$3,$4)",
        4, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir item de venda");
    PQclear(res);
}

void Loja::removerItensVenda(PGconn* conn, int vendaId) {
    string idStr = to_string(vendaId);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM itens_venda WHERE venda_id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover itens da venda");
    PQclear(res);
}

// Busca os itens do banco e os exibe diretamente (não armazena em Venda
// para evitar acoplamento com o banco dentro da classe de domínio).
void Loja::listarItensVenda(PGconn* conn, int vendaId) {
    string idStr = to_string(vendaId);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, venda_id, instrumento_id, quantidade, preco_unitario "
        "FROM itens_venda WHERE venda_id=$1 ORDER BY id",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "listar itens da venda");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        ItemVenda(atoi(PQgetvalue(res, i, 0)), atoi(PQgetvalue(res, i, 1)),
                  atoi(PQgetvalue(res, i, 2)), atoi(PQgetvalue(res, i, 3)),
                  atof(PQgetvalue(res, i, 4))).exibir();
    }
    PQclear(res);
}

// ===================== RELATÓRIOS =====================

void Loja::relatorioEstoque(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT COALESCE(SUM(quantidade), 0), COALESCE(SUM(preco * quantidade), 0) FROM instrumentos;");
    checarErro(conn, res, "gerar relatorio de estoque");
    PGresult* res2 = PQexec(conn,
        "SELECT COUNT(*) FROM instrumentos WHERE quantidade = 0;");
    checarErro(conn, res2, "gerar relatorio de estoque (sem estoque)");

    if (PQntuples(res) > 0 && PQntuples(res2) > 0) {
        cout << "\n--- RELATORIO DE ESTOQUE ---" << endl;
        cout << "Quantidade Total em Estoque: " << PQgetvalue(res, 0, 0) << endl;
        cout << "Valor Total em Estoque: R$ "   << PQgetvalue(res, 0, 1) << endl;
        cout << "Instrumentos sem Estoque: "    << PQgetvalue(res2, 0, 0) << endl;
        cout << "----------------------------\n" << endl;
    }
    PQclear(res2);
    PQclear(res);
}

void Loja::relatorioClientes(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM clientes;");
    checarErro(conn, res, "gerar relatorio de clientes");
    if (PQntuples(res) > 0) {
        cout << "\n--- RELATORIO DE CLIENTES ---" << endl;
        cout << "Total de Clientes Cadastrados: " << PQgetvalue(res, 0, 0) << endl;
        cout << "-----------------------------\n" << endl;
    }
    PQclear(res);
}

void Loja::relatorioVendas(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*), COALESCE(SUM(valor_total), 0) FROM vendas;");
    checarErro(conn, res, "gerar relatorio de vendas");
    if (PQntuples(res) > 0) {
        cout << "\n--- RELATORIO DE VENDAS ---" << endl;
        cout << "Total de Vendas Realizadas: " << PQgetvalue(res, 0, 0) << endl;
        cout << "Valor Total Vendido: R$ "     << PQgetvalue(res, 0, 1) << endl;
        cout << "---------------------------\n" << endl;
    }
    PQclear(res);
}

// ===================== EXIBIR LOJA =====================

void Loja::exibir() {
    cout << "\n===== DADOS DA LOJA =====" << endl;
    cout << "Nome: "     << nome     << endl;
    cout << "CNPJ: "     << cnpj     << endl;
    cout << "Endereco: " << endereco << endl;
    cout << "Telefone: " << telefone << endl;
    cout << "=========================\n" << endl;
}

// ===================== MENU INSTRUMENTOS =====================

void Loja::menuInstrumentos(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR INSTRUMENTOS =====" << endl;
        cout << "1. Inserir Instrumento" << endl;
        cout << "2. Alterar Instrumento" << endl;
        cout << "3. Pesquisar Instrumento" << endl;
        cout << "4. Remover Instrumento" << endl;
        cout << "5. Listar Todos os Instrumentos" << endl;
        cout << "6. Exibir Instrumento por ID" << endl;
        cout << "7. Relatorio de Estoque" << endl;
        cout << "0. Voltar" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        int idBusca, qtd, opcaoAlteracao;
        string nome, tipo, marca;
        double preco;

        switch (opcao) {
        case 1:
            cout << "Nome: ";
            cin.ignore();
            getline(cin, nome);
            tipo = solicitarTipoValido();
            cout << "Marca: ";
            getline(cin, marca);
            cout << "Preco: ";
            cin >> preco;
            cout << "Quantidade: ";
            cin >> qtd;
            inserirInstrumento(conn, Instrumento(0, nome, tipo, marca, preco, qtd));
            break;

        case 2:
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado para alterar." << endl;
                break;
            }
            listarInstrumentosSimplificado(conn);
            cout << "ID do instrumento a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibirInstrumento(conn, idBusca);

            cout << "===== MENU DE EDICAO =====" << endl;
            cout << "1. Alterar Nome" << endl;
            cout << "2. Alterar Tipo" << endl;
            cout << "3. Alterar Marca" << endl;
            cout << "4. Alterar Preco" << endl;
            cout << "5. Alterar Quantidade" << endl;
            cout << "0. Voltar" << endl;
            cout << "\nEscolha uma opcao: ";
            cin >> opcaoAlteracao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl;

            switch (opcaoAlteracao) {
            case 1:
                cout << "Novo Nome: ";
                getline(cin, nome);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {nome.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE instrumentos SET nome=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar nome do instrumento");
                    PQclear(res);
                }
                break;
            case 2:
                tipo = solicitarTipoValido();
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {tipo.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE instrumentos SET tipo=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar tipo do instrumento");
                    PQclear(res);
                }
                break;
            case 3:
                cout << "Nova Marca: ";
                getline(cin, marca);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {marca.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE instrumentos SET marca=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar marca do instrumento");
                    PQclear(res);
                }
                break;
            case 4:
                cout << "Novo Preco: ";
                cin >> preco;
                {
                    string precoStr = to_string(preco);
                    string idStr    = to_string(idBusca);
                    const char* p[2] = {precoStr.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE instrumentos SET preco=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar preco do instrumento");
                    PQclear(res);
                }
                break;
            case 5:
                cout << "Nova Quantidade: ";
                cin >> qtd;
                {
                    string qtdStr = to_string(qtd);
                    string idStr  = to_string(idBusca);
                    const char* p[2] = {qtdStr.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE instrumentos SET quantidade=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar quantidade do instrumento");
                    PQclear(res);
                }
                break;
            case 0:
                break;
            default:
                cout << "Opcao invalida!" << endl;
            }
            break;

        case 3:
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado para pesquisar." << endl;
                break;
            }
            cout << "Digite [nome/marca/tipo] para buscar: ";
            cin.ignore();
            getline(cin, nome);
            cout << endl;
            pesquisarInstrumento(conn, nome);
            break;

        case 4:
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado para remover." << endl;
                break;
            }
            listarInstrumentosSimplificado(conn);
            cout << "ID do instrumento a remover: ";
            cin >> idBusca;
            cout << endl;
            removerInstrumento(conn, idBusca);
            break;

        case 5:
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado." << endl;
                break;
            }
            listarInstrumentos(conn);
            break;

        case 6:
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado." << endl;
                break;
            }
            listarInstrumentosSimplificado(conn);
            cout << "ID do instrumento: ";
            cin >> idBusca;
            cout << endl;
            exibirInstrumento(conn, idBusca);
            break;

        case 7:
            relatorioEstoque(conn);
            break;

        case 0:
            cout << "Voltando ao menu principal..." << endl;
            break;

        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU CLIENTES =====================

void Loja::menuClientes(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR CLIENTES =====" << endl;
        cout << "1. Inserir Cliente" << endl;
        cout << "2. Alterar Cliente" << endl;
        cout << "3. Pesquisar Cliente" << endl;
        cout << "4. Remover Cliente" << endl;
        cout << "5. Listar Todos os Clientes" << endl;
        cout << "6. Exibir Cliente por ID" << endl;
        cout << "7. Relatorio de Clientes" << endl;
        cout << "0. Voltar" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        int idBusca, opcaoAlteracao;
        string nome, cpf, telefone, email, sexo;

        switch (opcao) {
        case 1:
            cin.ignore();
            cout << "Nome: ";
            getline(cin, nome);
            cout << "CPF: ";
            getline(cin, cpf);
            cout << "Telefone: ";
            getline(cin, telefone);
            cout << "Email: ";
            getline(cin, email);
            sexo = solicitarSexoValido();
            inserirCliente(conn, Cliente(0, nome, cpf, telefone, email, sexo));
            break;

        case 2:
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para alterar." << endl;
                break;
            }
            listarClientesSimplificado(conn);
            cout << "ID do cliente a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibirCliente(conn, idBusca);

            cout << "===== MENU DE EDICAO =====" << endl;
            cout << "1. Alterar Nome" << endl;
            cout << "2. Alterar CPF" << endl;
            cout << "3. Alterar Telefone" << endl;
            cout << "4. Alterar Email" << endl;
            cout << "5. Alterar Sexo" << endl;
            cout << "0. Voltar" << endl;
            cout << "\nEscolha uma opcao: ";
            cin >> opcaoAlteracao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl;

            switch (opcaoAlteracao) {
            case 1:
                cout << "Novo Nome: ";
                getline(cin, nome);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {nome.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE clientes SET nome=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar nome do cliente");
                    PQclear(res);
                }
                break;
            case 2:
                cout << "Novo CPF: ";
                getline(cin, cpf);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {cpf.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE clientes SET cpf=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar cpf do cliente");
                    PQclear(res);
                }
                break;
            case 3:
                cout << "Novo Telefone: ";
                getline(cin, telefone);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {telefone.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE clientes SET telefone=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar telefone do cliente");
                    PQclear(res);
                }
                break;
            case 4:
                cout << "Novo Email: ";
                getline(cin, email);
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {email.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE clientes SET email=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar email do cliente");
                    PQclear(res);
                }
                break;
            case 5:
                sexo = solicitarSexoValido();
                {
                    string idStr = to_string(idBusca);
                    const char* p[2] = {sexo.c_str(), idStr.c_str()};
                    PGresult* res = PQexecParams(conn,
                        "UPDATE clientes SET sexo=$1 WHERE id=$2",
                        2, nullptr, p, nullptr, nullptr, 0);
                    checarErro(conn, res, "alterar sexo do cliente");
                    PQclear(res);
                }
                break;
            case 0:
                break;
            default:
                cout << "Opcao invalida!" << endl;
            }
            break;

        case 3:
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para pesquisar." << endl;
                break;
            }
            cout << "Digite [nome/CPF] para buscar: ";
            cin.ignore();
            getline(cin, nome);
            cout << endl;
            pesquisarCliente(conn, nome);
            break;

        case 4:
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para remover." << endl;
                break;
            }
            listarClientesSimplificado(conn);
            cout << "ID do cliente a remover: ";
            cin >> idBusca;
            cout << endl;
            removerCliente(conn, idBusca);
            break;

        case 5:
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado." << endl;
                break;
            }
            listarClientes(conn);
            break;

        case 6:
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado." << endl;
                break;
            }
            listarClientesSimplificado(conn);
            cout << "ID do cliente: ";
            cin >> idBusca;
            cout << endl;
            exibirCliente(conn, idBusca);
            break;

        case 7:
            relatorioClientes(conn);
            break;

        case 0:
            cout << "Voltando ao menu principal..." << endl;
            break;

        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU VENDAS =====================

void Loja::menuVendas(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR VENDAS =====" << endl;
        cout << "1. Inserir Venda" << endl;
        cout << "2. Alterar Venda (data)" << endl;
        cout << "3. Pesquisar Venda por Nome do Cliente" << endl;
        cout << "4. Remover Venda" << endl;
        cout << "5. Listar Todas as Vendas" << endl;
        cout << "6. Exibir Venda por ID" << endl;
        cout << "7. Relatorio de Vendas" << endl;
        cout << "0. Voltar" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        int idBusca, clienteId;
        string data, nomeCliente;

        switch (opcao) {

        // ----- INSERIR VENDA -----
        // Fluxo: informa cliente e data → abre loop de itens →
        //        calcula total automaticamente → persiste tudo.
        case 1: {
            if (!possuiClientesCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado. Cadastre um cliente antes de registrar uma venda." << endl;
                break;
            }
            if (!possuiInstrumentosCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado. Cadastre um instrumento antes de registrar uma venda." << endl;
                break;
            }

            // Cabeçalho
            cout << "--- Clientes disponíveis ---" << endl;
            listarClientesSimplificado(conn);
            cout << "ID do Cliente: ";
            cin >> clienteId;
            cout << "Data da Venda (AAAA-MM-DD): ";
            cin.ignore();
            getline(cin, data);

            Venda novaVenda(0, clienteId, data, 0.0);

            // Loop de itens
            char continuar = 's';
            while (continuar == 's' || continuar == 'S') {
                cout << "\n--- Instrumentos disponíveis ---" << endl;
                listarInstrumentosSimplificado(conn);

                int instId, qtd;
                double precoUnit;
                cout << "ID do Instrumento: ";
                cin >> instId;
                cout << "Quantidade: ";
                cin >> qtd;
                cout << "Preco Unitario: ";
                cin >> precoUnit;

                // Adiciona item provisoriamente em memória para calcular o total
                novaVenda.adicionarItem(ItemVenda(0, 0, instId, qtd, precoUnit));

                cout << "Adicionar outro instrumento? (s/n): ";
                cin >> continuar;
            }

            // Calcula o valor total somando os subtotais dos itens
            novaVenda.recalcularTotal();

            // Persiste: primeiro o cabeçalho (obtém o ID), depois cada item
            int vendaId = inserirVenda(conn, novaVenda);
            if (vendaId > 0) {
                for (const ItemVenda& item : novaVenda.getItens()) {
                    inserirItemVenda(conn, ItemVenda(0, vendaId,
                        item.getInstrumentoId(), item.getQuantidade(), item.getPrecoUnitario()));
                }
                cout << "Venda #" << vendaId << " registrada com sucesso! "
                     << "Total: R$ " << novaVenda.getValorTotal() << endl;
            }
            break;
        }

        // ----- ALTERAR VENDA -----
        // Permite alterar a data ou substituir todos os itens da venda.
        case 2: {
            if (!possuiVendasCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para alterar." << endl;
                break;
            }
            listarVendasSimplificado(conn);
            cout << "ID da venda a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibirVenda(conn, idBusca);

            cout << "===== MENU DE EDICAO =====" << endl;
            cout << "1. Alterar Data" << endl;
            cout << "2. Substituir todos os Itens" << endl;
            cout << "0. Voltar" << endl;
            cout << "\nEscolha uma opcao: ";
            int opcaoAlteracao;
            cin >> opcaoAlteracao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl;

            switch (opcaoAlteracao) {
            case 1: {
                cout << "Nova Data (AAAA-MM-DD): ";
                getline(cin, data);
                string idStr = to_string(idBusca);
                const char* p[2] = {data.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE vendas SET data=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar data da venda");
                PQclear(res);
                break;
            }
            case 2: {
                // Remove todos os itens atuais e reinserindo os novos
                removerItensVenda(conn, idBusca);

                Venda vendaEditada(idBusca, 0, "", 0.0);
                char continuar = 's';
                while (continuar == 's' || continuar == 'S') {
                    cout << "\n--- Instrumentos disponíveis ---" << endl;
                    listarInstrumentosSimplificado(conn);

                    int instId, qtd;
                    double precoUnit;
                    cout << "ID do Instrumento: ";
                    cin >> instId;
                    cout << "Quantidade: ";
                    cin >> qtd;
                    cout << "Preco Unitario: ";
                    cin >> precoUnit;

                    vendaEditada.adicionarItem(ItemVenda(0, idBusca, instId, qtd, precoUnit));
                    cout << "Adicionar outro instrumento? (s/n): ";
                    cin >> continuar;
                }

                vendaEditada.recalcularTotal();

                for (const ItemVenda& item : vendaEditada.getItens())
                    inserirItemVenda(conn, item);

                // Atualiza o valor_total no cabeçalho
                string valStr = to_string(vendaEditada.getValorTotal());
                string idStr  = to_string(idBusca);
                const char* p[2] = {valStr.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE vendas SET valor_total=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "atualizar valor total da venda");
                PQclear(res);
                cout << "Itens atualizados. Novo total: R$ " << vendaEditada.getValorTotal() << endl;
                break;
            }
            case 0:
                break;
            default:
                cout << "Opcao invalida!" << endl;
            }
            break;
        }

        case 3:
            if (!possuiVendasCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para pesquisar." << endl;
                break;
            }
            cout << "Nome do cliente para buscar: ";
            cin.ignore();
            getline(cin, nomeCliente);
            cout << endl;
            pesquisarVenda(conn, nomeCliente);
            break;

        case 4:
            if (!possuiVendasCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para remover." << endl;
                break;
            }
            listarVendasSimplificado(conn);
            cout << "ID da venda a remover: ";
            cin >> idBusca;
            cout << endl;
            removerVenda(conn, idBusca);
            break;

        case 5:
            if (!possuiVendasCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada." << endl;
                break;
            }
            listarVendas(conn);
            break;

        case 6:
            if (!possuiVendasCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada." << endl;
                break;
            }
            listarVendasSimplificado(conn);
            cout << "ID da venda: ";
            cin >> idBusca;
            cout << endl;
            exibirVenda(conn, idBusca);
            break;

        case 7:
            relatorioVendas(conn);
            break;

        case 0:
            cout << "Voltando ao menu principal..." << endl;
            break;

        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU PRINCIPAL =====================

void Loja::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== MENU PRINCIPAL =====" << endl;
        cout << "1. Gerenciar Instrumentos" << endl;
        cout << "2. Gerenciar Clientes" << endl;
        cout << "3. Gerenciar Vendas" << endl;
        cout << "0. Sair" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1:
            menuInstrumentos(conn);
            break;
        case 2:
            menuClientes(conn);
            break;
        case 3:
            menuVendas(conn);
            break;
        case 0:
            cout << "Encerrando o sistema. Ate logo!" << endl;
            break;
        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}