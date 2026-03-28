#include "../headers/gerenciaClientes.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

// ===================== AUXILIARES ESTÁTICAS =====================

static string normalizarSexo(string sexo) {
    size_t inicio = sexo.find_first_not_of(" \t\n\r");
    if (inicio == string::npos) return "";
    size_t fim = sexo.find_last_not_of(" \t\n\r");
    sexo = sexo.substr(inicio, fim - inicio + 1);
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

// ===================== PRIVADO =====================

void GerenciaClientes::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK)
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
}

// ===================== CRUD =====================

void GerenciaClientes::inserir(PGconn* conn, Cliente cli) {
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

void GerenciaClientes::alterar(PGconn* conn, Cliente cli) {
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

void GerenciaClientes::pesquisar(PGconn* conn, string nomeBusca) {
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

void GerenciaClientes::remover(PGconn* conn, int id) {
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

void GerenciaClientes::listar(PGconn* conn) {
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

void GerenciaClientes::listarSimplificado(PGconn* conn) {
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

void GerenciaClientes::exibir(PGconn* conn, int id) {
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

bool GerenciaClientes::possuiCadastrados(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM clientes;");
    checarErro(conn, res, "contar clientes");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== RELATÓRIO =====================

void GerenciaClientes::relatorioClientes(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM clientes;");
    checarErro(conn, res, "gerar relatorio de clientes");
    if (PQntuples(res) > 0) {
        cout << "\n--- RELATORIO DE CLIENTES ---" << endl;
        cout << "Total de Clientes Cadastrados: " << PQgetvalue(res, 0, 0) << endl;
        cout << "-----------------------------\n" << endl;
    }
    PQclear(res);
}

// ===================== MENU =====================

void GerenciaClientes::menu(PGconn* conn) {
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
            inserir(conn, Cliente(0, nome, cpf, telefone, email, sexo));
            break;

        case 2:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para alterar." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do cliente a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibir(conn, idBusca);

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
            case 1: {
                cout << "Novo Nome: ";
                getline(cin, nome);
                string idStr = to_string(idBusca);
                const char* p[2] = {nome.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE clientes SET nome=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar nome");
                PQclear(res);
                break;
            }
            case 2: {
                cout << "Novo CPF: ";
                getline(cin, cpf);
                string idStr = to_string(idBusca);
                const char* p[2] = {cpf.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE clientes SET cpf=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar cpf");
                PQclear(res);
                break;
            }
            case 3: {
                cout << "Novo Telefone: ";
                getline(cin, telefone);
                string idStr = to_string(idBusca);
                const char* p[2] = {telefone.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE clientes SET telefone=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar telefone");
                PQclear(res);
                break;
            }
            case 4: {
                cout << "Novo Email: ";
                getline(cin, email);
                string idStr = to_string(idBusca);
                const char* p[2] = {email.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE clientes SET email=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar email");
                PQclear(res);
                break;
            }
            case 5: {
                sexo = solicitarSexoValido();
                string idStr = to_string(idBusca);
                const char* p[2] = {sexo.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE clientes SET sexo=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar sexo");
                PQclear(res);
                break;
            }
            case 0: break;
            default: cout << "Opcao invalida!" << endl;
            }
            break;

        case 3:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para pesquisar." << endl;
                break;
            }
            cout << "Digite [nome/CPF] para buscar: ";
            cin.ignore();
            getline(cin, nome);
            cout << endl;
            pesquisar(conn, nome);
            break;

        case 4:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado para remover." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do cliente a remover: ";
            cin >> idBusca;
            cout << endl;
            remover(conn, idBusca);
            break;

        case 5:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado." << endl;
                break;
            }
            listar(conn);
            break;

        case 6:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do cliente: ";
            cin >> idBusca;
            cout << endl;
            exibir(conn, idBusca);
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
