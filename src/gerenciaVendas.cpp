#include "../headers/gerenciaVendas.hpp"
#include "../headers/gerenciaInstrumentos.hpp"
#include "../headers/gerenciaClientes.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace std;

// ===================== PRIVADO =====================

void GerenciaVendas::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK)
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
}

// ===================== CRUD VENDA =====================

int GerenciaVendas::inserir(PGconn* conn, Venda venda) {
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
    return novoId;
}

void GerenciaVendas::alterar(PGconn* conn, Venda venda) {
    string idStr  = to_string(venda.getId());
    string valStr = to_string(venda.getValorTotal());
    const char* p[3] = {valStr.c_str(), venda.getData().c_str(), idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "UPDATE vendas SET valor_total=$1, data=$2 WHERE id=$3",
        3, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "alterar venda");
    PQclear(res);
}

void GerenciaVendas::pesquisar(PGconn* conn, string nomeCliente) {
    string termo = "%" + nomeCliente + "%";
    const char* p[1] = {termo.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT v.id, v.cliente_id, v.valor_total, v.data "
        "FROM vendas v JOIN clientes c ON v.cliente_id = c.id "
        "WHERE c.nome ILIKE $1 ORDER BY v.id",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "pesquisar venda");
    int rows = PQntuples(res);
    if (rows == 0) cout << "Nenhuma venda encontrada." << endl;
    for (int i = 0; i < rows; i++) {
        int vendaId = atoi(PQgetvalue(res, i, 0));
        Venda v(vendaId, atoi(PQgetvalue(res, i, 1)),
                PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 2)));
        listarItens(conn, vendaId);
        v.exibir();
    }
    PQclear(res);
}

void GerenciaVendas::remover(PGconn* conn, int id) {
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

void GerenciaVendas::listar(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT id, cliente_id, valor_total, data FROM vendas ORDER BY id;");
    checarErro(conn, res, "listar vendas");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        int vendaId = atoi(PQgetvalue(res, i, 0));
        Venda v(vendaId, atoi(PQgetvalue(res, i, 1)),
                PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 2)));
        listarItens(conn, vendaId);
        v.exibir();
    }
    PQclear(res);
}

void GerenciaVendas::listarSimplificado(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT id, data FROM vendas ORDER BY id;");
    checarErro(conn, res, "listar vendas");
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        Venda(atoi(PQgetvalue(res, i, 0)), 0,
              PQgetvalue(res, i, 1), 0.0).exibirSimplificado();
    }
    PQclear(res);
}

void GerenciaVendas::exibirVenda(PGconn* conn, int id) {
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
        listarItens(conn, id);
        v.exibir();
    } else {
        cout << "Venda nao encontrada." << endl;
        PQclear(res);
    }
}

bool GerenciaVendas::possuiCadastradas(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM vendas;");
    checarErro(conn, res, "contar vendas");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== CRUD ITENS =====================

void GerenciaVendas::inserirItem(PGconn* conn, ItemVenda item) {
    string vendaIdStr = to_string(item.getVendaId());
    string instIdStr  = to_string(item.getInstrumentoId());
    string qtdStr     = to_string(item.getQuantidade());
    string precoStr   = to_string(item.getPrecoUnitario());
    const char* p[4]  = {vendaIdStr.c_str(), instIdStr.c_str(), qtdStr.c_str(), precoStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "INSERT INTO itens_venda (venda_id, instrumento_id, quantidade, preco_unitario) "
        "VALUES ($1,$2,$3,$4)",
        4, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "inserir item de venda");
    PQclear(res);
}

void GerenciaVendas::removerItens(PGconn* conn, int vendaId) {
    string idStr = to_string(vendaId);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "DELETE FROM itens_venda WHERE venda_id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "remover itens da venda");
    PQclear(res);
}

void GerenciaVendas::listarItens(PGconn* conn, int vendaId) {
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

// ===================== RELATÓRIO =====================

void GerenciaVendas::relatorioVendas(PGconn* conn) {
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

// ===================== MENU =====================

void GerenciaVendas::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR VENDAS =====" << endl;
        cout << "1. Inserir Venda" << endl;
        cout << "2. Alterar Venda (data/itens)" << endl;
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

        case 1: {
            if (!GerenciaClientes::possuiCadastrados(conn)) {
                cout << "Nenhum cliente cadastrado. Cadastre um cliente antes de registrar uma venda." << endl;
                break;
            }
            if (!GerenciaInstrumentos::possuiCadastrados(conn)) {
                cout << "Nenhum instrumento cadastrado. Cadastre um instrumento antes de registrar uma venda." << endl;
                break;
            }
            cout << "--- Clientes disponíveis ---" << endl;
            GerenciaClientes::listarSimplificado(conn);
            cout << "ID do Cliente: ";
            cin >> clienteId;
            cout << "Data da Venda (AAAA-MM-DD): ";
            cin.ignore();
            getline(cin, data);

            Venda novaVenda(0, clienteId, data, 0.0);

            char continuar = 's';
            while (continuar == 's' || continuar == 'S') {
                cout << "\n--- Instrumentos disponíveis ---" << endl;
                GerenciaInstrumentos::listarSimplificado(conn);
                int instId, qtd;
                double precoUnit;
                cout << "ID do Instrumento: ";
                cin >> instId;
                cout << "Quantidade: ";
                cin >> qtd;
                cout << "Preco Unitario: ";
                cin >> precoUnit;
                novaVenda.adicionarItem(ItemVenda(0, 0, instId, qtd, precoUnit));
                cout << "Adicionar outro instrumento? (s/n): ";
                cin >> continuar;
            }

            novaVenda.recalcularTotal();
            int vendaId = inserir(conn, novaVenda);
            if (vendaId > 0) {
                for (const ItemVenda& item : novaVenda.getItens()) {
                    inserirItem(conn, ItemVenda(0, vendaId,
                        item.getInstrumentoId(), item.getQuantidade(), item.getPrecoUnitario()));
                }
                cout << "Venda #" << vendaId << " registrada! Total: R$ " << novaVenda.getValorTotal() << endl;
            }
            break;
        }

        case 2: {
            if (!possuiCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para alterar." << endl;
                break;
            }
            listarSimplificado(conn);
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
                removerItens(conn, idBusca);
                Venda vendaEditada(idBusca, 0, "", 0.0);
                char continuar = 's';
                while (continuar == 's' || continuar == 'S') {
                    cout << "\n--- Instrumentos disponíveis ---" << endl;
                    GerenciaInstrumentos::listarSimplificado(conn);
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
                    inserirItem(conn, item);
                string valStr = to_string(vendaEditada.getValorTotal());
                string idStr  = to_string(idBusca);
                const char* p[2] = {valStr.c_str(), idStr.c_str()};
                PGresult* res = PQexecParams(conn,
                    "UPDATE vendas SET valor_total=$1 WHERE id=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                checarErro(conn, res, "atualizar total da venda");
                PQclear(res);
                cout << "Itens atualizados. Novo total: R$ " << vendaEditada.getValorTotal() << endl;
                break;
            }
            case 0: break;
            default: cout << "Opcao invalida!" << endl;
            }
            break;
        }

        case 3:
            if (!possuiCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para pesquisar." << endl;
                break;
            }
            cout << "Nome do cliente para buscar: ";
            cin.ignore();
            getline(cin, nomeCliente);
            cout << endl;
            pesquisar(conn, nomeCliente);
            break;

        case 4:
            if (!possuiCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada para remover." << endl;
                break;
            }
            listarSimplificado(conn);
            cout << "ID da venda a remover: ";
            cin >> idBusca;
            cout << endl;
            remover(conn, idBusca);
            break;

        case 5:
            if (!possuiCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada." << endl;
                break;
            }
            listar(conn);
            break;

        case 6:
            if (!possuiCadastradas(conn)) {
                cout << "Nenhuma venda cadastrada." << endl;
                break;
            }
            listarSimplificado(conn);
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
