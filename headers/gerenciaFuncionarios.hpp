#ifndef GERENCIA_FUNCIONARIOS_HPP
#define GERENCIA_FUNCIONARIOS_HPP

#include "funcionario.hpp"
#include <libpq-fe.h>
#include <string>

class GerenciaFuncionarios {
public:
    static void inserir(PGconn* conn, Funcionario func);
    static void alterar(PGconn* conn, Funcionario func);
    static void pesquisar(PGconn* conn, std::string nomeBusca);
    static void remover(PGconn* conn, int id);
    static void listar(PGconn* conn);
    static void listarSimplificado(PGconn* conn);
    static void exibir(PGconn* conn, int id);
    static bool possuiCadastrados(PGconn* conn);
    static bool autenticar(PGconn* conn, int& idOut); // login por CPF ou ID
    static void relatorioFuncionarios(PGconn* conn);
    static void menu(PGconn* conn);

private:
    static void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);
};

#endif
