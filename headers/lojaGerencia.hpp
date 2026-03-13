#ifndef LOJAGERENCIA_HPP
#define LOJAGERENCIA_HPP

#include "instrumento.hpp"
#include <libpq-fe.h>
#include <string>

PGconn* conectar();

class Loja {
private:
        std::string nome;
        std::string cnpj;
        std::string endereco;
        std::string telefone;

        void checarErro(PGconn* conn, PGresult* res, const std::string& operacao);

public:
        Loja(std::string nome, std::string cnpj,
             std::string endereco, std::string telefone);

        // CRUD
        void inserirInstrumento(PGconn* conn, Instrumento inst);
        void alterarInstrumento(PGconn* conn, Instrumento inst);
        void pesquisarInstrumentoPorNome(PGconn* conn, std::string nomeBusca);
        void removerInstrumento(PGconn* conn, int id);
        void listarInstrumentos(PGconn* conn);
        void exibirInstrumento(PGconn* conn, int id);
        bool possuiInstrumentosCadastrados(PGconn* conn);

        // Relatórios
        void relatorioEstoque(PGconn* conn);

        void exibir();
        void menu(PGconn* conn);
};

#endif