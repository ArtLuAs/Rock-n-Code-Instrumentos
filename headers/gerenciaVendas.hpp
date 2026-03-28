#ifndef GERENCIA_VENDAS_HPP
#define GERENCIA_VENDAS_HPP

#include "venda.hpp"
#include "itemVenda.hpp"
#include <libpq-fe.h>
#include <string>

class GerenciaVendas {
public:
    static int  inserir(PGconn* conn, Venda venda);
    static void alterar(PGconn* conn, Venda venda);
    static void pesquisar(PGconn* conn, std::string nomeCliente);
    static void remover(PGconn* conn, int id);
    static void listar(PGconn* conn);
    static void listarSimplificado(PGconn* conn);
    static void exibirVenda(PGconn* conn, int id);
    static bool possuiCadastradas(PGconn* conn);

    static void inserirItem(PGconn* conn, ItemVenda item);
    static void removerItens(PGconn* conn, int vendaId);
    static void listarItens(PGconn* conn, int vendaId);

    static void relatorioVendas(PGconn* conn);
    static void menu(PGconn* conn);

private:
    static void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);
};

#endif
