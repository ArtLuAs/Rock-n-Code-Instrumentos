#include "../headers/gerenciaVendas.hpp"
#include "../headers/gerenciaInstrumentos.hpp"
#include "../headers/gerenciaClientes.hpp"
#include "../headers/gerenciaFuncionarios.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sstream>
#include <vector>

using namespace std;

// ===================== PRIVADO =====================

void GerenciaVendas::checarErro(PGconn* conn, PGresult* res, const string& operacao) {
    if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK)
        cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
}

bool GerenciaVendas::possuiPedidos(PGconn* conn) {
    PGresult* res = PQexec(conn, "SELECT COUNT(*) FROM pedidos;");
    checarErro(conn, res, "contar pedidos");
    int total = (PQntuples(res) > 0) ? atoi(PQgetvalue(res, 0, 0)) : 0;
    PQclear(res);
    return total > 0;
}

// ===================== NOVA VENDA =====================

void GerenciaVendas::novaVenda(PGconn* conn, int funcionarioId) {
    if (!GerenciaClientes::possuiCadastrados(conn)) {
        cout << "Nenhum cliente cadastrado. Cadastre um cliente antes de registrar uma venda." << endl;
        return;
    }
    if (!GerenciaInstrumentos::possuiCadastrados(conn)) {
        cout << "Nenhum instrumento cadastrado. Cadastre um instrumento antes de registrar uma venda." << endl;
        return;
    }

    // --- Selecionar cliente ---
    cout << "\n--- Clientes disponiveis ---" << endl;
    GerenciaClientes::listarSimplificado(conn);
    int clienteId;
    cout << "ID do Cliente: ";
    cin >> clienteId;

    // Verificar se cliente tem desconto
    {
        string idStr = to_string(clienteId);
        const char* p[1] = {idStr.c_str()};
        PGresult* res = PQexecParams(conn,
            "SELECT nome, torce_flamengo, assiste_one_piece, LOWER(COALESCE(cidade,'')) "
            "FROM clientes WHERE id=$1",
            1, nullptr, p, nullptr, nullptr, 0);
        checarErro(conn, res, "buscar cliente");
        if (PQntuples(res) == 0) {
            cout << "Cliente nao encontrado." << endl;
            PQclear(res);
            return;
        }
        bool flam    = string(PQgetvalue(res, 0, 1)) == "t";
        bool one     = string(PQgetvalue(res, 0, 2)) == "t";
        bool sousa   = string(PQgetvalue(res, 0, 3)) == "sousa";
        bool desconto = flam || one || sousa;
        cout << "\nCliente: " << PQgetvalue(res, 0, 0);
        if (desconto)
            cout << " | DESCONTO DE 10% APLICADO";
        cout << endl;
        PQclear(res);
    }

    // --- Montar carrinho ---
    vector<int> instIds, qtds;
    char continuar = 's';
    while (continuar == 's' || continuar == 'S') {
        cout << "\n--- Instrumentos disponiveis ---" << endl;
        GerenciaInstrumentos::listarSimplificado(conn);
        int instId, qtd;
        cout << "ID do Instrumento: "; cin >> instId;
        cout << "Quantidade: ";        cin >> qtd;
        instIds.push_back(instId);
        qtds.push_back(qtd);
        cout << "Adicionar outro instrumento? (s/n): "; cin >> continuar;
    }

    // --- Forma de pagamento ---
    cout << "\nForma de pagamento:" << endl;
    cout << "1. Dinheiro"          << endl;
    cout << "2. Cartao de Credito" << endl;
    cout << "3. Cartao de Debito"  << endl;
    cout << "4. Pix"               << endl;
    cout << "5. Boleto"            << endl;
    cout << "6. Berries"           << endl;
    cout << "Escolha: ";
    int formaopc; cin >> formaopc;
    string forma;
    switch (formaopc) {
        case 1: forma = "dinheiro";       break;
        case 2: forma = "cartao_credito"; break;
        case 3: forma = "cartao_debito";  break;
        case 4: forma = "pix";            break;
        case 5: forma = "boleto";         break;
        case 6: forma = "berries";        break;
        default: forma = "dinheiro";
    }

    // --- Montar arrays PostgreSQL ---
    ostringstream arrInst, arrQtd;
    arrInst << "{";
    arrQtd  << "{";
    for (size_t i = 0; i < instIds.size(); i++) {
        if (i > 0) { arrInst << ","; arrQtd << ","; }
        arrInst << instIds[i];
        arrQtd  << qtds[i];
    }
    arrInst << "}";
    arrQtd  << "}";

    string cliStr  = to_string(clienteId);
    string funcStr = to_string(funcionarioId);
    string instArr = arrInst.str();
    string qtdArr  = arrQtd.str();

    // Ordem da procedure: (clienteId, funcionarioId, instrumentos[], quantidades[], forma_pgto, OUT pedidoId)
    const char* p[5] = {
        cliStr.c_str(), funcStr.c_str(),
        instArr.c_str(), qtdArr.c_str(), forma.c_str()
    };

    // --- Chamar stored procedure ---
    PGresult* res = PQexecParams(conn,
        "CALL efetuar_compra($1::integer, $2::integer, "
        "$3::integer[], $4::integer[], $5::forma_pgto, NULL)",
        5, nullptr, p, nullptr, nullptr, 0);

    if (PQresultStatus(res) == PGRES_COMMAND_OK || PQresultStatus(res) == PGRES_TUPLES_OK) {
        cout << "\nPedido registrado com sucesso! Status: PENDENTE." << endl;
        cout << "Use a opcao 'Confirmar/Recusar Pagamento' para finalizar." << endl;
    } else {
        cout << "\nErro ao registrar pedido: " << PQerrorMessage(conn) << endl;
    }
    PQclear(res);
}

// ===================== CONFIRMAR / RECUSAR =====================

void GerenciaVendas::atualizarStatus(PGconn* conn) {
    if (!possuiPedidos(conn)) {
        cout << "Nenhum pedido cadastrado." << endl;
        return;
    }

    PGresult* res = PQexec(conn,
        "SELECT p.id, c.nome, p.total, p.desconto, p.forma_pagamento, p.data "
        "FROM pedidos p JOIN clientes c ON c.id = p.cliente_id "
        "WHERE p.status_pagamento = 'pendente' ORDER BY p.id;");
    checarErro(conn, res, "listar pedidos pendentes");
    int rows = PQntuples(res);
    if (rows == 0) {
        cout << "Nenhum pedido pendente." << endl;
        PQclear(res);
        return;
    }
    cout << "\n--- Pedidos Pendentes ---" << endl;
    for (int i = 0; i < rows; i++) {
        cout << "[" << PQgetvalue(res, i, 0) << "] "
             << PQgetvalue(res, i, 1)
             << " | R$ " << PQgetvalue(res, i, 2)
             << " | Desconto: " << PQgetvalue(res, i, 3) << "%"
             << " | " << PQgetvalue(res, i, 4)
             << " | " << PQgetvalue(res, i, 5) << endl;
    }
    PQclear(res);

    int pedidoId;
    cout << "\nID do pedido: "; cin >> pedidoId;
    cout << "\n1. Confirmar pagamento" << endl;
    cout << "2. Recusar pagamento"    << endl;
    cout << "0. Cancelar"             << endl;
    cout << "Escolha: ";
    int opcao; cin >> opcao;

    string novoStatus;
    if      (opcao == 1) novoStatus = "confirmado";
    else if (opcao == 2) novoStatus = "recusado";
    else { cout << "Operacao cancelada." << endl; return; }

    string idStr = to_string(pedidoId);
    const char* p[2] = {novoStatus.c_str(), idStr.c_str()};
    PGresult* res2 = PQexecParams(conn,
        "UPDATE pedidos SET status_pagamento=$1::status_pgto WHERE id=$2",
        2, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res2, "atualizar status do pedido");
    if (PQresultStatus(res2) == PGRES_COMMAND_OK)
        cout << "Status atualizado para: " << novoStatus << endl;
    PQclear(res2);
}

// ===================== LISTAR =====================

void GerenciaVendas::listar(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT p.id, c.nome, f.nome, p.data, p.forma_pagamento, "
        "p.status_pagamento, p.desconto, p.total "
        "FROM pedidos p "
        "JOIN clientes c     ON c.id = p.cliente_id "
        "JOIN funcionarios f ON f.id = p.funcionario_id "
        "ORDER BY p.id;");
    checarErro(conn, res, "listar pedidos");
    int rows = PQntuples(res);
    if (rows == 0) { cout << "Nenhum pedido registrado." << endl; PQclear(res); return; }
    for (int i = 0; i < rows; i++) {
        cout << "------------------------------------------" << endl;
        cout << "Pedido #"    << PQgetvalue(res, i, 0) << endl;
        cout << "Cliente: "   << PQgetvalue(res, i, 1) << endl;
        cout << "Vendedor: "  << PQgetvalue(res, i, 2) << endl;
        cout << "Data: "      << PQgetvalue(res, i, 3) << endl;
        cout << "Pagamento: " << PQgetvalue(res, i, 4)
             << " | Status: " << PQgetvalue(res, i, 5) << endl;
        cout << "Desconto: "  << PQgetvalue(res, i, 6) << "%" << endl;
        cout << "Total: R$ "  << PQgetvalue(res, i, 7) << endl;
    }
    cout << "------------------------------------------" << endl;
    PQclear(res);
}

void GerenciaVendas::exibirPedido(PGconn* conn, int pedidoId) {
    string idStr = to_string(pedidoId);
    const char* p[1] = {idStr.c_str()};

    PGresult* res = PQexecParams(conn,
        "SELECT p.id, c.nome, f.nome, p.data, p.forma_pagamento, "
        "p.status_pagamento, p.desconto, p.total "
        "FROM pedidos p "
        "JOIN clientes c     ON c.id = p.cliente_id "
        "JOIN funcionarios f ON f.id = p.funcionario_id "
        "WHERE p.id=$1",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "exibir pedido");
    if (PQntuples(res) == 0) {
        cout << "Pedido nao encontrado." << endl;
        PQclear(res);
        return;
    }
    cout << "\n========== PEDIDO #" << PQgetvalue(res, 0, 0) << " ==========" << endl;
    cout << "Cliente: "   << PQgetvalue(res, 0, 1) << endl;
    cout << "Vendedor: "  << PQgetvalue(res, 0, 2) << endl;
    cout << "Data: "      << PQgetvalue(res, 0, 3) << endl;
    cout << "Pagamento: " << PQgetvalue(res, 0, 4)
         << " | Status: " << PQgetvalue(res, 0, 5) << endl;
    cout << "Desconto: "  << PQgetvalue(res, 0, 6) << "%" << endl;
    cout << "Total: R$ "  << PQgetvalue(res, 0, 7) << endl;
    PQclear(res);

    PGresult* res2 = PQexecParams(conn,
        "SELECT i.nome, ip.quantidade, ip.preco_unitario, "
        "(ip.quantidade * ip.preco_unitario) AS subtotal "
        "FROM itens_pedido ip "
        "JOIN instrumentos i ON i.id = ip.instrumento_id "
        "WHERE ip.pedido_id=$1 ORDER BY ip.id",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res2, "listar itens do pedido");
    int rows = PQntuples(res2);
    cout << "\n--- Itens ---" << endl;
    for (int i = 0; i < rows; i++) {
        cout << PQgetvalue(res2, i, 0)
             << " x" << PQgetvalue(res2, i, 1)
             << " | R$ " << PQgetvalue(res2, i, 2)
             << " | Subtotal: R$ " << PQgetvalue(res2, i, 3) << endl;
    }
    cout << "====================================\n" << endl;
    PQclear(res2);
}

// ===================== HISTORICO CLIENTE =====================

void GerenciaVendas::historicoCliente(PGconn* conn) {
    if (!GerenciaClientes::possuiCadastrados(conn)) {
        cout << "Nenhum cliente cadastrado." << endl;
        return;
    }
    GerenciaClientes::listarSimplificado(conn);
    int clienteId;
    cout << "ID do Cliente: "; cin >> clienteId;

    string idStr = to_string(clienteId);
    const char* p[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn,
        "SELECT p.id, p.data, p.forma_pagamento, p.status_pagamento, p.desconto, p.total "
        "FROM pedidos p WHERE p.cliente_id=$1 ORDER BY p.data DESC",
        1, nullptr, p, nullptr, nullptr, 0);
    checarErro(conn, res, "historico do cliente");
    int rows = PQntuples(res);
    if (rows == 0) {
        cout << "Este cliente nao possui pedidos." << endl;
        PQclear(res);
        return;
    }
    cout << "\n--- Historico de Pedidos ---" << endl;
    for (int i = 0; i < rows; i++) {
        cout << "[" << PQgetvalue(res, i, 0) << "] "
             << PQgetvalue(res, i, 1)
             << " | " << PQgetvalue(res, i, 2)
             << " | " << PQgetvalue(res, i, 3)
             << " | Desconto: " << PQgetvalue(res, i, 4) << "%"
             << " | Total: R$ " << PQgetvalue(res, i, 5) << endl;
    }
    PQclear(res);

    char verDetalhe;
    cout << "\nVer detalhe de algum pedido? (s/n): "; cin >> verDetalhe;
    if (verDetalhe == 's' || verDetalhe == 'S') {
        int pedidoId;
        cout << "ID do pedido: "; cin >> pedidoId;
        exibirPedido(conn, pedidoId);
    }
}

// ===================== RELATORIO MENSAL =====================

void GerenciaVendas::relatorioMensal(PGconn* conn) {
    PGresult* res = PQexec(conn,
        "SELECT vendedor, mes, total_pedidos, total_vendido "
        "FROM vw_vendas_por_vendedor_mes;");
    checarErro(conn, res, "relatorio mensal");
    int rows = PQntuples(res);
    cout << "\n--- RELATORIO MENSAL POR VENDEDOR ---" << endl;
    if (rows == 0) {
        cout << "Nenhuma venda confirmada ainda." << endl;
    } else {
        for (int i = 0; i < rows; i++) {
            cout << PQgetvalue(res, i, 1)          // mes
                 << " | " << PQgetvalue(res, i, 0) // vendedor
                 << " | Pedidos: " << PQgetvalue(res, i, 2)
                 << " | Total: R$ " << PQgetvalue(res, i, 3) << endl;
        }
    }
    cout << "-------------------------------------\n" << endl;
    PQclear(res);
}

// ===================== MENU =====================

void GerenciaVendas::menu(PGconn* conn) {
    int funcionarioId = -1;
    if (!GerenciaFuncionarios::autenticar(conn, funcionarioId)) return;

    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== GERENCIAR VENDAS =====" << endl;
        cout << "1. Nova Venda"                  << endl;
        cout << "2. Confirmar/Recusar Pagamento" << endl;
        cout << "3. Listar Todos os Pedidos"     << endl;
        cout << "4. Exibir Pedido por ID"        << endl;
        cout << "5. Historico de um Cliente"     << endl;
        cout << "6. Relatorio Mensal"            << endl;
        cout << "0. Voltar"                      << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: novaVenda(conn, funcionarioId);  break;
        case 2: atualizarStatus(conn);           break;
        case 3: listar(conn);                    break;
        case 4: {
            if (!possuiPedidos(conn)) { cout << "Nenhum pedido cadastrado." << endl; break; }
            listar(conn);
            int id; cout << "ID do pedido: "; cin >> id;
            exibirPedido(conn, id);
            break;
        }
        case 5: historicoCliente(conn);          break;
        case 6: relatorioMensal(conn);           break;
        case 0: cout << "Voltando..." << endl;   break;
        default: cout << "Opcao invalida!" << endl;
        }
    }
}
