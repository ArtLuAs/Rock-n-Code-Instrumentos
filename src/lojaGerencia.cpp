#include "../headers/lojaGerencia.hpp"
#include <iostream>

using namespace std;

PGconn* conectar() {
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=loja_musical user=lojamusical_user password=SenhaSegura123"
    );

    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Erro na conexão: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

Loja::Loja(string nome, string cnpj, string endereco, string telefone) {
    this->nome = nome;
    this->cnpj = cnpj;
    this->endereco = endereco;
    this->telefone = telefone;
}

void Loja::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
        cout << "Erro ao " << operacao << ": "<< PQerrorMessage(conn) << endl;
    }
}

// ===== IMPLEMENTAÇÃO DO CRUD =====

void Loja::inserirInstrumento(PGconn* conn, Instrumento inst) {
    string query = "INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade) VALUES ('" + inst.getNome() + "', '" + inst.getTipo() + "', '" + inst.getMarca() + "', " +
                   to_string(inst.getPreco()) + ", " + to_string(inst.getQuantidade()) + ");";
    PGresult* res = PQexec(conn, query.c_str());
    checarErro(conn, res, "inserir instrumento");
    PQclear(res);
}

void Loja::alterarInstrumento(PGconn* conn, Instrumento inst) {
    string query = "UPDATE instrumentos SET nome = '" + inst.getNome() +
                   "', tipo = '" + inst.getTipo() +
                   "', marca = '" + inst.getMarca() +
                   "', preco = " + to_string(inst.getPreco()) +
                   ", quantidade = " + to_string(inst.getQuantidade()) +
                   " WHERE id = " + to_string(inst.getId()) + ";";
    PGresult* res = PQexec(conn, query.c_str());
    checarErro(conn, res, "alterar instrumento");
    PQclear(res);
}

void Loja::pesquisarInstrumentoPorNome(PGconn* conn, string nomeBusca) {
    string query = "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos WHERE nome ILIKE '%" + nomeBusca + "%';";
    PGresult* res = PQexec(conn, query.c_str());
    checarErro(conn, res, "pesquisar instrumento");

    int rows = PQntuples(res);
    for(int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)),
                    atoi(PQgetvalue(res, i, 5))).exibir();
    }
    PQclear(res);
}

void Loja::removerInstrumento(PGconn* conn, int id) {
    string query = "DELETE FROM instrumentos WHERE id = " + to_string(id) + ";";
    PGresult* res = PQexec(conn, query.c_str());
    checarErro(conn, res, "remover instrumento");
    PQclear(res);
}

void Loja::listarInstrumentos(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos ORDER BY id;");
    checarErro(conn, res, "listar instrumentos");

    int rows = PQntuples(res);
    for(int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)),
                    atoi(PQgetvalue(res, i, 5))).exibir();
    }
    PQclear(res);
}

void Loja::exibirInstrumento(PGconn* conn, int id) {
    string query = "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos WHERE id = " + to_string(id) + ";";
    PGresult* res = PQexec(conn, query.c_str());
    checarErro(conn, res, "exibir instrumento");

    if (PQntuples(res) > 0) {
        Instrumento(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1),
                    PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3),
                    atof(PQgetvalue(res, 0, 4)),
                    atoi(PQgetvalue(res, 0, 5))).exibir();
    } else {
        cout << "Instrumento não encontrado." << endl;
    }
    PQclear(res);
}

void Loja::relatorioEstoque(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT SUM(quantidade), SUM(preco * quantidade) FROM instrumentos;");
    checarErro(conn, res, "gerar relatório de estoque");

    if (PQntuples(res) > 0) {
        cout << "\n--- RELATÓRIO DE ESTOQUE ---\n" << endl;
        cout << "Quantidade Total: " << PQgetvalue(res, 0, 0) << endl;
        cout << "Valor Total: R$ " << PQgetvalue(res, 0, 1) << endl;
        cout << "----------------------------\n" << endl;
    }
    PQclear(res);
}

void Loja::exibir() {
    cout << "\n===== DADOS DA LOJA =====" << endl;
    cout << "Nome: " << nome << endl;
    cout << "CNPJ: " << cnpj << endl;
    cout << "Endereço: " << endereco << endl;
    cout << "Telefone: " << telefone << endl;
    cout << "=========================\n" << endl;
}

void Loja::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== MENU DE GERENCIAMENTO =====" << endl;
        cout << "1. Inserir Instrumento" << endl;
        cout << "2. Alterar Instrumento" << endl;
        cout << "3. Pesquisar Instrumento por Nome" << endl;
        cout << "4. Remover Instrumento" << endl;
        cout << "5. Listar Todos os Instrumentos" << endl;
        cout << "6. Exibir um Instrumento (por ID)" << endl;
        cout << "7. Relatorio de Estoque" << endl;
        cout << "0. Sair" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;

        int idBusca, qtd;
        string nome, tipo, marca;
        double preco;

        switch (opcao) {
            case 1:
                cout << "Nome: "; cin.ignore(); getline(cin, nome);
                cout << "Tipo (guitarra/violao/baixo): "; getline(cin, tipo);
                cout << "Marca: "; getline(cin, marca);
                cout << "Preco: "; cin >> preco;
                cout << "Quantidade: "; cin >> qtd;
                inserirInstrumento(conn, Instrumento(0, nome, tipo, marca, preco, qtd));
                break;
            case 2:
                cout << "ID do Instrumento a alterar: "; cin >> idBusca;
                cout << "Novo Nome: "; cin.ignore(); getline(cin, nome);
                cout << "Novo Tipo (guitarra/violao/baixo): "; getline(cin, tipo);
                cout << "Nova Marca: "; getline(cin, marca);
                cout << "Novo Preco: "; cin >> preco;
                cout << "Nova Quantidade: "; cin >> qtd;
                alterarInstrumento(conn, Instrumento(idBusca, nome, tipo, marca, preco, qtd));
                break;
            case 3:
                cout << "Digite o nome para buscar: "; cin.ignore(); getline(cin, nome);
                pesquisarInstrumentoPorNome(conn, nome);
                break;
            case 4:
                cout << "ID do Instrumento a remover: "; cin >> idBusca;
                removerInstrumento(conn, idBusca);
                break;
            case 5:
                listarInstrumentos(conn);
                break;
            case 6:
                cout << "ID do Instrumento: "; cin >> idBusca;
                exibirInstrumento(conn, idBusca);
                break;
            case 7:
                relatorioEstoque(conn);
                break;
            case 0:
                cout << "Encerrando o sistema..." << endl;
                break;
            default:
                cout << "Opcao invalida!" << endl;
        }
    }
}
