#include "../headers/gerenciaInstrumentos.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

// ===================== AUXILIARES ESTÁTICAS =====================

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

// ===================== PRIVADO =====================

void GerenciaInstrumentos::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK)
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
}

void GerenciaInstrumentos::imprimirResultados(PGconn* conn, PGresult* res) {
    int rows = PQntuples(res);
    if (rows == 0) { cout << "Nenhum instrumento encontrado." << endl; return; }
    for (int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5)),
                    PQgetvalue(res, i, 6),
                    string(PQgetvalue(res, i, 7)) == "t").exibir();
    }
}

// ===================== CRUD =====================

void GerenciaInstrumentos::inserir(PGconn* conn, Instrumento inst) {
    string precoStr = to_string(inst.getPreco());
    string qtdStr   = to_string(inst.getQuantidade());
    string mariStr  = inst.getFabricadoEmMari() ? "true" : "false";
    const char* p[7] = {
        inst.getNome().c_str(), inst.getTipo().c_str(), inst.getMarca().c_str(),
        precoStr.c_str(), qtdStr.c_str(), inst.getCategoria().c_str(), mariStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7)",
        7, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir instrumento");
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
        cout << "Instrumento inserido com sucesso!" << endl;
    PQclear(res);
}

void GerenciaInstrumentos::alterar(PGconn* conn, Instrumento inst) {
    string idStr    = to_string(inst.getId());
    string precoStr = to_string(inst.getPreco());
    string qtdStr   = to_string(inst.getQuantidade());
    string mariStr  = inst.getFabricadoEmMari() ? "true" : "false";
    const char* p[8] = {
        inst.getNome().c_str(), inst.getTipo().c_str(), inst.getMarca().c_str(),
        precoStr.c_str(), qtdStr.c_str(), inst.getCategoria().c_str(),
        mariStr.c_str(), idStr.c_str()
    };
    PGresult* res = PQexecParams(conn,
        "UPDATE instrumentos SET nome=$1, tipo=$2, marca=$3, preco=$4, "
        "quantidade=$5, categoria=$6, fabricado_em_mari=$7 WHERE id=$8",
        8, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar instrumento");
    PQclear(res);
}

void GerenciaInstrumentos::pesquisar(PGconn* conn, string nomeBusca) {
    string termo = "%" + nomeBusca + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE nome ILIKE $1 OR tipo ILIKE $1 OR marca ILIKE $1 OR categoria ILIKE $1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar instrumento");
    imprimirResultados(conn, res);
    PQclear(res);
}

void GerenciaInstrumentos::remover(PGconn* conn, int id) {
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

void GerenciaInstrumentos::listar(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos ORDER BY id;");
    checarErro(conn, res, "listar instrumentos");
    imprimirResultados(conn, res);
    PQclear(res);
}

void GerenciaInstrumentos::listarSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos ORDER BY id;");
    checarErro(conn, res, "listar instrumentos");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1),
                    PQgetvalue(res, i, 2), PQgetvalue(res, i, 3),
                    atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5)),
                    PQgetvalue(res, i, 6),
                    string(PQgetvalue(res, i, 7)) == "t").exibirSimplificado();
    }
    PQclear(res);
}

void GerenciaInstrumentos::exibir(PGconn* conn, int id) {
    string idStr = to_string(id);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir instrumento");
    if (PQntuples(res) > 0) {
        Instrumento(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1),
                    PQgetvalue(res, 0, 2), PQgetvalue(res, 0, 3),
                    atof(PQgetvalue(res, 0, 4)), atoi(PQgetvalue(res, 0, 5)),
                    PQgetvalue(res, 0, 6),
                    string(PQgetvalue(res, 0, 7)) == "t").exibir();
    } else {
        cout << "Instrumento nao encontrado." << endl;
    }
    PQclear(res);
}

bool GerenciaInstrumentos::possuiCadastrados(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM instrumentos;");
    checarErro(conn, res, "contar instrumentos");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== FILTROS =====================

void GerenciaInstrumentos::filtrarPorPreco(PGconn* conn, double precoMin, double precoMax) {
    string minStr = to_string(precoMin);
    string maxStr = to_string(precoMax);
    const char* p[2] = {minStr.c_str(), maxStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE preco BETWEEN $1 AND $2 ORDER BY preco",
        2, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "filtrar por preco");
    cout << "\n--- Instrumentos entre R$ " << precoMin << " e R$ " << precoMax << " ---" << endl;
    imprimirResultados(conn, res);
    PQclear(res);
}

void GerenciaInstrumentos::filtrarPorCategoria(PGconn* conn, string categoria) {
    string termo = "%" + categoria + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE categoria ILIKE $1 ORDER BY nome",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "filtrar por categoria");
    cout << "\n--- Instrumentos da categoria '" << categoria << "' ---" << endl;
    imprimirResultados(conn, res);
    PQclear(res);
}

void GerenciaInstrumentos::filtrarFabricadosEmMari(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE fabricado_em_mari = TRUE ORDER BY nome;");
    checarErro(conn, res, "filtrar fabricados em Mari");
    cout << "\n--- Instrumentos fabricados em Mari ---" << endl;
    imprimirResultados(conn, res);
    PQclear(res);
}

void GerenciaInstrumentos::filtrarEstoqueBaixo(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
        "FROM instrumentos WHERE quantidade < 5 ORDER BY quantidade;");
    checarErro(conn, res, "filtrar estoque baixo");
    cout << "\n--- Instrumentos com estoque critico (< 5 unidades) ---" << endl;
    imprimirResultados(conn, res);
    PQclear(res);
}

// ===================== RELATÓRIO =====================

void GerenciaInstrumentos::relatorioEstoque(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT COALESCE(SUM(quantidade), 0), COALESCE(SUM(preco * quantidade), 0) FROM instrumentos;");
    checarErro(conn, res, "gerar relatorio de estoque");
    PGresult* res2 = PQexec(conn,
        "SELECT COUNT(*) FROM instrumentos WHERE quantidade = 0;");
    checarErro(conn, res2, "gerar relatorio de estoque (sem estoque)");
    PGresult* res3 = PQexec(conn,
        "SELECT COUNT(*) FROM instrumentos WHERE fabricado_em_mari = TRUE;");
    checarErro(conn, res3, "gerar relatorio de estoque (mari)");
    PGresult* res4 = PQexec(conn,
        "SELECT categoria, COUNT(*), SUM(quantidade) FROM instrumentos GROUP BY categoria ORDER BY categoria;");
    checarErro(conn, res4, "relatorio por categoria");

    if (PQntuples(res) > 0) {
        cout << "\n--- RELATORIO DE ESTOQUE ---" << endl;
        cout << "Quantidade Total em Estoque: " << PQgetvalue(res,  0, 0) << endl;
        cout << "Valor Total em Estoque: R$ "   << PQgetvalue(res,  0, 1) << endl;
        cout << "Instrumentos sem Estoque: "    << PQgetvalue(res2, 0, 0) << endl;
        cout << "Fabricados em Mari: "          << PQgetvalue(res3, 0, 0) << endl;
        cout << "\n-- Por Categoria --" << endl;
        for (int i = 0; i < PQntuples(res4); i++) {
            cout << PQgetvalue(res4, i, 0)
                 << " | Modelos: " << PQgetvalue(res4, i, 1)
                 << " | Unidades: " << PQgetvalue(res4, i, 2) << endl;
        }
        cout << "----------------------------\n" << endl;
    }
    PQclear(res4);
    PQclear(res3);
    PQclear(res2);
    PQclear(res);
}

// ===================== MENU FILTROS =====================

void GerenciaInstrumentos::menuFiltros(PGconn* conn, bool isFuncionario) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== FILTROS DE PRODUTOS =====" << endl;
        cout << "1. Por faixa de preco"           << endl;
        cout << "2. Por categoria"                << endl;
        cout << "3. Fabricados em Mari"           << endl;
        if (isFuncionario)
            cout << "4. Estoque critico (< 5 un.)" << endl;
        cout << "0. Voltar"                       << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: {
            double pMin, pMax;
            cout << "Preco minimo: R$ "; cin >> pMin;
            cout << "Preco maximo: R$ "; cin >> pMax;
            filtrarPorPreco(conn, pMin, pMax);
            break;
        }
        case 2: {
            string cat;
            cout << "Categoria: ";
            cin.ignore();
            getline(cin, cat);
            filtrarPorCategoria(conn, cat);
            break;
        }
        case 3:
            filtrarFabricadosEmMari(conn);
            break;
        case 4:
            if (isFuncionario)
                filtrarEstoqueBaixo(conn);
            else
                cout << "Opcao invalida!" << endl;
            break;
        case 0:
            cout << "Voltando..." << endl;
            break;
        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU =====================

void GerenciaInstrumentos::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR INSTRUMENTOS =====" << endl;
        cout << "1. Inserir Instrumento"   << endl;
        cout << "2. Alterar Instrumento"   << endl;
        cout << "3. Pesquisar Instrumento" << endl;
        cout << "4. Remover Instrumento"   << endl;
        cout << "5. Listar Todos"          << endl;
        cout << "6. Exibir por ID"         << endl;
        cout << "7. Filtros de Produtos"   << endl;
        cout << "8. Relatorio de Estoque"  << endl;
        cout << "0. Voltar"                << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        int idBusca, qtd, opcaoAlteracao;
        string nome, tipo, marca, categoria;
        double preco;
        char mariChar;
        bool mari;

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
            cin.ignore();
            cout << "Categoria: ";
            getline(cin, categoria);
            cout << "Fabricado em Mari? (s/n): ";
            cin >> mariChar;
            mari = (mariChar == 's' || mariChar == 'S');
            inserir(conn, Instrumento(0, nome, tipo, marca, preco, qtd, categoria, mari));
            break;

        case 2:
            if (!possuiCadastrados(conn)) { cout << "Nenhum instrumento cadastrado." << endl; break; }
            listarSimplificado(conn);
            cout << "ID do instrumento a alterar: ";
            cin >> idBusca;
            cout << endl;
            exibir(conn, idBusca);

            cout << "===== MENU DE EDICAO =====" << endl;
            cout << "1. Alterar Nome"             << endl;
            cout << "2. Alterar Tipo"             << endl;
            cout << "3. Alterar Marca"            << endl;
            cout << "4. Alterar Preco"            << endl;
            cout << "5. Alterar Quantidade"       << endl;
            cout << "6. Alterar Categoria"        << endl;
            cout << "7. Alterar Fabricado em Mari" << endl;
            cout << "0. Voltar"                   << endl;
            cout << "\nEscolha uma opcao: ";
            cin >> opcaoAlteracao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << endl;

            switch (opcaoAlteracao) {
            case 1: {
                cout << "Novo Nome: "; getline(cin, nome);
                string idStr = to_string(idBusca);
                const char* p[2] = {nome.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET nome=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar nome"); PQclear(res); break;
            }
            case 2: {
                tipo = solicitarTipoValido();
                string idStr = to_string(idBusca);
                const char* p[2] = {tipo.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET tipo=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar tipo"); PQclear(res); break;
            }
            case 3: {
                cout << "Nova Marca: "; getline(cin, marca);
                string idStr = to_string(idBusca);
                const char* p[2] = {marca.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET marca=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar marca"); PQclear(res); break;
            }
            case 4: {
                cout << "Novo Preco: "; cin >> preco;
                string precoStr = to_string(preco), idStr = to_string(idBusca);
                const char* p[2] = {precoStr.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET preco=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar preco"); PQclear(res); break;
            }
            case 5: {
                cout << "Nova Quantidade: "; cin >> qtd;
                string qtdStr = to_string(qtd), idStr = to_string(idBusca);
                const char* p[2] = {qtdStr.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET quantidade=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar quantidade"); PQclear(res); break;
            }
            case 6: {
                cout << "Nova Categoria: "; getline(cin, categoria);
                string idStr = to_string(idBusca);
                const char* p[2] = {categoria.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET categoria=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar categoria"); PQclear(res); break;
            }
            case 7: {
                cout << "Fabricado em Mari? (s/n): "; cin >> mariChar;
                string mariStr = (mariChar == 's' || mariChar == 'S') ? "true" : "false";
                string idStr   = to_string(idBusca);
                const char* p[2] = {mariStr.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn, "UPDATE instrumentos SET fabricado_em_mari=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "alterar fabricado em mari"); PQclear(res); break;
            }
            case 0: break;
            default: cout << "Opcao invalida!" << endl;
            }
            break;

        case 3:
            if (!possuiCadastrados(conn)) { cout << "Nenhum instrumento cadastrado." << endl; break; }
            cout << "Buscar por [nome/marca/tipo/categoria]: ";
            cin.ignore();
            getline(cin, nome);
            pesquisar(conn, nome);
            break;

        case 4:
            if (!possuiCadastrados(conn)) { cout << "Nenhum instrumento cadastrado." << endl; break; }
            listarSimplificado(conn);
            cout << "ID do instrumento a remover: "; cin >> idBusca;
            remover(conn, idBusca);
            break;

        case 5:
            if (!possuiCadastrados(conn)) { cout << "Nenhum instrumento cadastrado." << endl; break; }
            listar(conn);
            break;

        case 6:
            if (!possuiCadastrados(conn)) { cout << "Nenhum instrumento cadastrado." << endl; break; }
            listarSimplificado(conn);
            cout << "ID do instrumento: "; cin >> idBusca;
            exibir(conn, idBusca);
            break;

        case 7:
            menuFiltros(conn, true); // true = modo funcionario (mostra estoque critico)
            break;

        case 8:
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
