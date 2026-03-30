#ifndef GERENCIA_INSTRUMENTOS_HPP
#define GERENCIA_INSTRUMENTOS_HPP

#include "instrumento.hpp"
#include <libpq-fe.h>
#include <string>

class GerenciaInstrumentos {
public:
    static void inserir(PGconn* conn, Instrumento inst);
    static void alterar(PGconn* conn, Instrumento inst);
    static void pesquisar(PGconn* conn, std::string nomeBusca);
    static void remover(PGconn* conn, int id);
    static void listar(PGconn* conn);
    static void listarSimplificado(PGconn* conn);
    static void exibir(PGconn* conn, int id);
    static bool possuiCadastrados(PGconn* conn);
    static void relatorioEstoque(PGconn* conn);

    // Filtros
    static void filtrarPorPreco(PGconn* conn, double precoMin, double precoMax);
    static void filtrarPorCategoria(PGconn* conn, std::string categoria);
    static void filtrarFabricadosEmMari(PGconn* conn);
    static void filtrarEstoqueBaixo(PGconn* conn); // estoque < 5, exclusivo funcionarios

    static void menu(PGconn* conn);
    static void menuFiltros(PGconn* conn, bool isFuncionario = false);

private:
    static void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);
    static void imprimirResultados(PGconn* conn, PGresult* res);
};

#endif
