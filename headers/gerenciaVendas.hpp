#ifndef GERENCIA_VENDAS_HPP
#define GERENCIA_VENDAS_HPP

#include <libpq-fe.h>
#include <string>

class GerenciaVendas {
public:
    // Fluxo principal de nova venda (chama efetuar_compra)
    static void novaVenda(PGconn* conn, int funcionarioId);

    // Confirmar ou recusar pagamento de um pedido
    static void atualizarStatus(PGconn* conn);

    // Listar e exibir pedidos
    static void listar(PGconn* conn);
    static void exibirPedido(PGconn* conn, int pedidoId);

    // Histórico de pedidos de um cliente (usado no menu cliente)
    static void historicoCliente(PGconn* conn);

    // Relatório mensal por vendedor (via view)
    static void relatorioMensal(PGconn* conn);

    // Menu completo (modo funcionário)
    static void menu(PGconn* conn);

private:
    static void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);
    static bool possuiPedidos(PGconn* conn);
};

#endif
