#include "../headers/lojaGerencia.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

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

// ===== IMPLEMENTAÇÃO DO CRUD =====

void Loja::inserirInstrumento(PGconn* conn, Instrumento inst) {
        string precoStr = to_string(inst.getPreco());
        string qtdStr = to_string(inst.getQuantidade());
        const char* params[5] = {
            inst.getNome().c_str(), inst.getTipo().c_str(),
            inst.getMarca().c_str(), precoStr.c_str(), qtdStr.c_str()};
        PGresult* res = PQexecParams(conn,
                                     "INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade) VALUES ($1,$2,$3,$4,$5)",
                                     5, nullptr, params, nullptr, nullptr, 0);
        checarErro(conn, res, "inserir instrumento");
        PQclear(res);
}

void Loja::alterarInstrumento(PGconn* conn, Instrumento inst) {
        string idStr = to_string(inst.getId());
        string precoStr = to_string(inst.getPreco());
        string qtdStr = to_string(inst.getQuantidade());
        const char* p[6] = {
            inst.getNome().c_str(), inst.getTipo().c_str(), inst.getMarca().c_str(),
            precoStr.c_str(), qtdStr.c_str(), idStr.c_str()};
        PGresult* res = PQexecParams(conn,
                                     "UPDATE instrumentos SET nome=$1, tipo=$2, marca=$3, preco=$4, quantidade=$5 WHERE id=$6",
                                     6, nullptr, p, nullptr, nullptr, 0);
        checarErro(conn, res, "alterar instrumento");
        PQclear(res);
}

void Loja::pesquisarInstrumento(PGconn* conn, string nomeBusca) {
        // O ILIKE com wildcards deve ser montado no lado C++,
        // mas o valor é passado como parâmetro — nunca concatenado no SQL.
        string termoBusca = "%" + nomeBusca + "%";
        const char* params[1] = {termoBusca.c_str()};
        PGresult* res = PQexecParams(conn,
                                     "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos "
                                     "WHERE nome ILIKE $1 OR tipo ILIKE $1 OR marca ILIKE $1",
                                     1, nullptr, params, nullptr, nullptr, 0);
        checarErro(conn, res, "pesquisar instrumento");

        int rows = PQntuples(res);
        for (int i = 0; i < rows; i++) {
                Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                            PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                            atof(PQgetvalue(res, i, 4)),
                            atoi(PQgetvalue(res, i, 5)))
                    .exibir();
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
                            atof(PQgetvalue(res, i, 4)),
                            atoi(PQgetvalue(res, i, 5)))
                    .exibir();
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
                            atof(PQgetvalue(res, i, 4)),
                            atoi(PQgetvalue(res, i, 5)))
                    .exibirSimplificado();
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
                            atof(PQgetvalue(res, 0, 4)),
                            atoi(PQgetvalue(res, 0, 5)))
                    .exibir();
        } else {
                cout << "Instrumento nao encontrado." << endl;
        }
        PQclear(res);
}

bool Loja::possuiInstrumentosCadastrados(PGconn* conn) {
        PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM instrumentos;");
        checarErro(conn, res, "contar instrumentos cadastrados");

        int totalInstrumentos = 0;
        if (PQntuples(res) > 0) {
                totalInstrumentos = atoi(PQgetvalue(res, 0, 0));
        }
        PQclear(res);

        return totalInstrumentos > 0;
}

void Loja::relatorioEstoque(PGconn* conn) {
        PGresult* res = PQexec(conn,
                               "SELECT COALESCE(SUM(quantidade), 0), COALESCE(SUM(preco * quantidade), 0) FROM instrumentos;");
        checarErro(conn, res, "gerar relatorio de estoque");
        PGresult* res2 = PQexec(conn,
                                "SELECT COUNT(*) FROM instrumentos WHERE quantidade = 0;");
        checarErro(conn, res2, "gerar relatorio de estoque 2");

        if (PQntuples(res) > 0 && PQntuples(res2) > 0) {
                cout << "\n--- RELATORIO DE ESTOQUE ---\n"
                     << endl;
                cout << "Quantidade Total: " << PQgetvalue(res, 0, 0) << endl;
                cout << "Valor Total: R$ " << PQgetvalue(res, 0, 1) << endl;
                cout << "Numero de Instrumentos sem Estoque: " << PQgetvalue(res2, 0, 0) << endl;
                cout << "----------------------------\n"
                     << endl;
        }
        PQclear(res2);
        PQclear(res);
}

void Loja::exibir() {
        cout << "\n===== DADOS DA LOJA =====" << endl;
        cout << "Nome: " << nome << endl;
        cout << "CNPJ: " << cnpj << endl;
        cout << "Endereco: " << endereco << endl;
        cout << "Telefone: " << telefone << endl;
        cout << "=========================\n"
             << endl;
}

static string normalizarTipo(string tipo) {
        size_t inicio = tipo.find_first_not_of(" \t\n\r");
        if (inicio == string::npos) {
                return "";
        }

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
                if (tipoValido(tipo)) {
                        return tipo;
                }

                cout << "Tipo invalido. Digite apenas: guitarra, violao ou baixo." << endl;
        }
}

void Loja::menu(PGconn* conn) {
        int opcao = -1;
        while (opcao != 0) {
                cout << endl
                     << "===== MENU DE GERENCIAMENTO =====" << endl;
                cout << "1. Inserir Instrumento" << endl;
                cout << "2. Alterar Instrumento" << endl;
                cout << "3. Pesquisar Instrumento" << endl;
                cout << "4. Remover Instrumento" << endl;
                cout << "5. Listar Todos os Instrumentos" << endl;
                cout << "6. Exibir um Instrumento (por ID)" << endl;
                cout << "7. Relatorio de Estoque" << endl;
                cout << "0. Sair" << endl;
                cout << endl
                     << "Escolha uma opcao: ";
                cin >> opcao;
                cout << endl;

                int idBusca, qtd;
                string nome, tipo, marca;
                double preco;

                int opcaoAlteracao = -1;
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
                        cout << "ID do Instrumento a alterar: ";
                        cin >> idBusca;
                        cout << endl;

                        exibirInstrumento(conn, idBusca);
                        cout << "\n===== MENU DE EDICAO =====" << endl;
                        cout << "1. Alterar Nome" << endl;
                        cout << "2. Alterar Tipo" << endl;
                        cout << "3. Alterar Marca" << endl;
                        cout << "4. Alterar Preco" << endl;
                        cout << "5. Alterar Quantidade" << endl;
                        cout << "0. Sair" << endl;
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
                                        string precoStr = to_string(preco); // string com vida útil garantida
                                        string idStr = to_string(idBusca);
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
                                        string qtdStr = to_string(qtd); // idem
                                        string idStr = to_string(idBusca);
                                        const char* p[2] = {qtdStr.c_str(), idStr.c_str()};
                                        PGresult* res = PQexecParams(conn,
                                                                     "UPDATE instrumentos SET quantidade=$1 WHERE id=$2",
                                                                     2, nullptr, p, nullptr, nullptr, 0);
                                        checarErro(conn, res, "alterar quantidade do instrumento");
                                        PQclear(res);
                                }
                                break;
                        case 0:
                                cout << "Saindo do menu de edicao..." << endl;
                                break;
                        default:
                                cout << "Opcao invalida!" << endl;
                                break;
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
                        cout << "ID do Instrumento a remover: ";
                        cin >> idBusca;
                        cout << endl;
                        removerInstrumento(conn, idBusca);
                        break;
                case 5:
                        if (!possuiInstrumentosCadastrados(conn)) {
                                cout << "Nenhum instrumento cadastrado para listar." << endl;
                                break;
                        }
                        listarInstrumentos(conn);
                        break;
                case 6:
                        if (!possuiInstrumentosCadastrados(conn)) {
                                cout << "Nenhum instrumento cadastrado para listar." << endl;
                                break;
                        }
                        listarInstrumentosSimplificado(conn);
                        cout << "ID do Instrumento: ";
                        cin >> idBusca;
                        cout << endl;
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
