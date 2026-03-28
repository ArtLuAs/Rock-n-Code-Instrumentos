#include "../headers/gerenciaFuncionarios.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

// ===================== PRIVADO =====================

void GerenciaFuncionarios::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK)
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
}

// ===================== CRUD =====================

void GerenciaFuncionarios::inserir(PGconn* conn, Funcionario func) {
    const char* p[5] = {
        func.getNome().c_str(), func.getCpf().c_str(),
        func.getTelefone().c_str(), func.getEmail().c_str(),
        func.getCargo().c_str()
    };
    PGresult* res = PQexecParams(conn,
        "INSERT INTO funcionarios (nome, cpf, telefone, email, cargo) "
        "VALUES ($1,$2,$3,$4,$5)",
        5, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir funcionario");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Funcionario inserido com sucesso!" << endl;
    PQclear(res);
}

void GerenciaFuncionarios::alterar(PGconn* conn, Funcionario func) {
    string idStr = to_string(func.getId());
    const char* p[6] = {
        func.getNome().c_str(), func.getCpf().c_str(),
        func.getTelefone().c_str(), func.getEmail().c_str(),
        func.getCargo().c_str(), idStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "UPDATE funcionarios SET nome=$1, cpf=$2, telefone=$3, email=$4, cargo=$5 WHERE id=$6",
        6, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar funcionario");
    PQclear(res);
}

void GerenciaFuncionarios::pesquisar(PGconn* conn, string nomeBusca) {
    string termo = "%" + nomeBusca + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, cpf, telefone, email, cargo "
        "FROM funcionarios WHERE nome ILIKE $1 OR cpf ILIKE $1 OR cargo ILIKE $1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar funcionario");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhum funcionario encontrado." << endl;
    for (int i = 0; i < rows; i++)
        Funcionario(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibir();
    PQclear(res);
}

void GerenciaFuncionarios::remover(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM funcionarios WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover funcionario");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Funcionario removido com sucesso!" << endl;
    PQclear(res);
}

void GerenciaFuncionarios::listar(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, cpf, telefone, email, cargo FROM funcionarios ORDER BY id;");
    checarErro(conn, res, "listar funcionarios");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhum funcionario cadastrado." << endl;
    for (int i = 0; i < rows; i++)
        Funcionario(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibir();
    PQclear(res);
}

void GerenciaFuncionarios::listarSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, cpf, telefone, email, cargo FROM funcionarios ORDER BY id;");
    checarErro(conn, res, "listar funcionarios");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhum funcionario cadastrado." << endl;
    for (int i = 0; i < rows; i++)
        Funcionario(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    PQgetvalue(res, i, 4), PQgetvalue(res, i, 5)).exibirSimplificado();
    PQclear(res);
}

void GerenciaFuncionarios::exibir(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, cpf, telefone, email, cargo FROM funcionarios WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir funcionario");
    if (PQntuples(res) > 0)
        Funcionario(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1),
                    PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3),
                    PQgetvalue(res, 0, 4), PQgetvalue(res, 0, 5)).exibir();
    else
        cout << "Funcionario nao encontrado." << endl;
    PQclear(res);
}

bool GerenciaFuncionarios::possuiCadastrados(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM funcionarios;");
    checarErro(conn, res, "contar funcionarios");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== AUTENTICAÇÃO =====================

bool GerenciaFuncionarios::autenticar(PGconn* conn, int& idOut) {
    cout << "\n===== LOGIN DE FUNCIONARIO =====" << endl;
    cout << "Digite seu CPF ou ID: ";
    string entrada;
    cin.ignore();
    getline(cin, entrada);

    PGresult* res;
    // Tenta por ID se for numérico, senão por CPF
    bool isNumerico = !entrada.empty() &&
                      entrada.find_first_not_of("0123456789") == string::npos;
    if (isNumerico) {
        const char* p[1] = {entrada.c_str()};
        res = PQexecParams(conn,
            "SELECT id, nome FROM funcionarios WHERE id=$1",
            1, nullptr, p, nullptr, nullptr, 0);
    } else {
        const char* p[1] = {entrada.c_str()};
        res = PQexecParams(conn,
            "SELECT id, nome FROM funcionarios WHERE cpf=$1",
            1, nullptr, p, nullptr, nullptr, 0);
    }
    checarErro(conn, res, "autenticar funcionario");
    if (PQntuples(res) > 0) {
        idOut = atoi(PQgetvalue(res, 0, 0));
        cout << "Bem-vindo, " << PQgetvalue(res, 0, 1) << "!" << endl;
        PQclear(res);
        return true;
    }
    cout << "Funcionario nao encontrado. Acesso negado." << endl;
    PQclear(res);
    return false;
}

// ===================== RELATÓRIO =====================

void GerenciaFuncionarios::relatorioFuncionarios(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM funcionarios;");
    checarErro(conn, res, "relatorio funcionarios");
    PGresult* res2 = PQexec(conn,
        "SELECT vendedor, mes, total_pedidos, total_vendido "
        "FROM vw_vendas_por_vendedor_mes LIMIT 10;");
    checarErro(conn, res2, "relatorio vendas por vendedor");

    cout << "\n--- RELATORIO DE FUNCIONARIOS ---" << endl;
    cout << "Total cadastrado: " << PQgetvalue(res, 0, 0) << endl;
    cout << "\n-- Top Vendas (pedidos confirmados) --" << endl;
    int rows = PQntuples(res2);
    if (rows == 0) {
        cout << "Nenhuma venda confirmada ainda." << endl;
    } else {
        for (int i = 0; i < rows; i++) {
            cout << PQgetvalue(res2, i, 0)        // vendedor
                 << " | "  << PQgetvalue(res2, i, 1)  // mes
                 << " | Pedidos: " << PQgetvalue(res2, i, 2)
                 << " | Total: R$ " << PQgetvalue(res2, i, 3) << endl;
        }
    }
    cout << "---------------------------------\n" << endl;
    PQclear(res2);
    PQclear(res);
}

// ===================== MENU =====================

void GerenciaFuncionarios::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR FUNCIONARIOS =====" << endl;
        cout << "1. Inserir Funcionario"   << endl;
        cout << "2. Alterar Funcionario"   << endl;
        cout << "3. Pesquisar Funcionario" << endl;
        cout << "4. Remover Funcionario"   << endl;
        cout << "5. Listar Todos"          << endl;
        cout << "6. Exibir por ID"         << endl;
        cout << "7. Relatorio"             << endl;
        cout << "0. Voltar"                << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        int idBusca, opcaoAlteracao;
        string nome, cpf, telefone, email, cargo, busca;

        switch (opcao) {
        case 1:
            cin.ignore();
            cout << "Nome: ";     getline(cin, nome);
            cout << "CPF: ";      getline(cin, cpf);
            cout << "Telefone: "; getline(cin, telefone);
            cout << "Email: ";    getline(cin, email);
            cout << "Cargo: ";    getline(cin, cargo);
            if (cargo.empty()) cargo = "vendedor";
            inserir(conn, Funcionario(0, nome, cpf, telefone, email, cargo));
            break;

        case 2:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum funcionario cadastrado." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do funcionario a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibir(conn, idBusca);

            cout << "===== MENU DE EDICAO =====" << endl;
            cout << "1. Alterar Nome"     << endl;
            cout << "2. Alterar CPF"      << endl;
            cout << "3. Alterar Telefone" << endl;
            cout << "4. Alterar Email"    << endl;
            cout << "5. Alterar Cargo"    << endl;
            cout << "0. Voltar"           << endl;
            cout << "\nEscolha uma opcao: ";
            cin >> opcaoAlteracao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl;

            switch (opcaoAlteracao) {
            case 1: {
                cout << "Novo Nome: "; getline(cin, nome);
                string idStr = to_string(idBusca);
                const char* p[2] = {nome.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE funcionarios SET nome=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar nome"); PQclear(res);
                break;
            }
            case 2: {
                cout << "Novo CPF: "; getline(cin, cpf);
                string idStr = to_string(idBusca);
                const char* p[2] = {cpf.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE funcionarios SET cpf=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar cpf"); PQclear(res);
                break;
            }
            case 3: {
                cout << "Novo Telefone: "; getline(cin, telefone);
                string idStr = to_string(idBusca);
                const char* p[2] = {telefone.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE funcionarios SET telefone=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar telefone"); PQclear(res);
                break;
            }
            case 4: {
                cout << "Novo Email: "; getline(cin, email);
                string idStr = to_string(idBusca);
                const char* p[2] = {email.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE funcionarios SET email=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar email"); PQclear(res);
                break;
            }
            case 5: {
                cout << "Novo Cargo: "; getline(cin, cargo);
                string idStr = to_string(idBusca);
                const char* p[2] = {cargo.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE funcionarios SET cargo=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar cargo"); PQclear(res);
                break;
            }
            case 0: break;
            default: cout << "Opcao invalida!" << endl;
            }
            break;

        case 3:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum funcionario cadastrado." << endl;
                break;
            }
            cout << "Digite [nome/CPF/cargo] para buscar: ";
            cin.ignore();
            getline(cin, busca);
            pesquisar(conn, busca);
            break;

        case 4:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum funcionario cadastrado." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do funcionario a remover: ";
            cin >> idBusca;
            remover(conn, idBusca);
            break;

        case 5:
            listar(conn);
            break;

        case 6:
            if (!possuiCadastrados(conn)) {
                cout << "Nenhum funcionario cadastrado." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID do funcionario: ";
            cin >> idBusca;
            exibir(conn, idBusca);
            break;

        case 7:
            relatorioFuncionarios(conn);
            break;

        case 0:
            cout << "Voltando..." << endl;
            break;

        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}
