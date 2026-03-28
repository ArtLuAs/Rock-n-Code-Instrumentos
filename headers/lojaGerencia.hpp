#ifndef LOJAGERENCIA_HPP
#define LOJAGERENCIA_HPP

#include "gerenciaInstrumentos.hpp"
#include "gerenciaClientes.hpp"
#include "gerenciaVendas.hpp"
#include <libpq-fe.h>
#include <string>

PGconn* conectar();

class Loja {
private:
    std::string nome;
    std::string cnpj;
    std::string endereco;
    std::string telefone;

public:
    Loja(std::string nome, std::string cnpj,
         std::string endereco, std::string telefone);

    void exibir();
    void menu(PGconn* conn);
};

#endif
