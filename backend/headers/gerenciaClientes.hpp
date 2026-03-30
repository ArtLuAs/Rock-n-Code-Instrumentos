#ifndef GERENCIA_CLIENTES_HPP
#define GERENCIA_CLIENTES_HPP

#include "cliente.hpp"
#include <libpq-fe.h>
#include <string>

class GerenciaClientes {
public:
    static void inserir(PGconn* conn, Cliente cli);
    static void alterar(PGconn* conn, Cliente cli);
    static void pesquisar(PGconn* conn, std::string nomeBusca);
    static void remover(PGconn* conn, int id);
    static void listar(PGconn* conn);
    static void listarSimplificado(PGconn* conn);
    static void exibir(PGconn* conn, int id);
    static bool possuiCadastrados(PGconn* conn);
    static void relatorioClientes(PGconn* conn);
    static void menu(PGconn* conn);

private:
    static void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);
};

#endif
