#ifndef LOJAGERENCIA_HPP
#define LOJAGERENCIA_HPP

#include "instrumento.hpp"
#include "cliente.hpp"
#include "venda.hpp"      // já inclui itemVenda.hpp
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

    // ===== CRUD INSTRUMENTO =====
    void inserirInstrumento(PGconn* conn, Instrumento inst);
    void alterarInstrumento(PGconn* conn, Instrumento inst);
    void pesquisarInstrumento(PGconn* conn, std::string nomeBusca);
    void removerInstrumento(PGconn* conn, int id);
    void listarInstrumentos(PGconn* conn);
    void listarInstrumentosSimplificado(PGconn* conn);
    void exibirInstrumento(PGconn* conn, int id);
    bool possuiInstrumentosCadastrados(PGconn* conn);

    // ===== CRUD CLIENTE =====
    void inserirCliente(PGconn* conn, Cliente cli);
    void alterarCliente(PGconn* conn, Cliente cli);
    void pesquisarCliente(PGconn* conn, std::string nomeBusca);
    void removerCliente(PGconn* conn, int id);
    void listarClientes(PGconn* conn);
    void listarClientesSimplificado(PGconn* conn);
    void exibirCliente(PGconn* conn, int id);
    bool possuiClientesCadastrados(PGconn* conn);

    // ===== CRUD VENDA (cabeçalho) =====
    // inserirVenda: persiste o cabeçalho e retorna o ID gerado pelo banco
    int  inserirVenda(PGconn* conn, Venda venda);
    void alterarVenda(PGconn* conn, Venda venda);
    void pesquisarVenda(PGconn* conn, std::string nomeCliente);
    void removerVenda(PGconn* conn, int id);  // remove cabeçalho e itens em cascata
    void listarVendas(PGconn* conn);
    void listarVendasSimplificado(PGconn* conn);
    void exibirVenda(PGconn* conn, int id);   // exibe cabeçalho + itens
    bool possuiVendasCadastradas(PGconn* conn);

    // ===== CRUD ITEM VENDA =====
    void inserirItemVenda(PGconn* conn, ItemVenda item);
    void removerItensVenda(PGconn* conn, int vendaId); // remove todos os itens de uma venda
    void listarItensVenda(PGconn* conn, int vendaId);  // carrega itens de uma venda específica

    // ===== RELATÓRIOS =====
    void relatorioEstoque(PGconn* conn);
    void relatorioClientes(PGconn* conn);
    void relatorioVendas(PGconn* conn);

    // ===== MENUS =====
    void exibir();
    void menu(PGconn* conn);
    void menuInstrumentos(PGconn* conn);
    void menuClientes(PGconn* conn);
    void menuVendas(PGconn* conn);
};

#endif