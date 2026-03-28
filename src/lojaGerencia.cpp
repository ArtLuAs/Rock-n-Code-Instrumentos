#include "../headers/lojaGerencia.hpp"
#include <iostream>

using namespace std;

// ===================== CONEXÃO =====================

PGconn* conectar() {
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=loja_musical user=lojamusical_user password=SenhaSegura123");
    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Erro na conexao: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

// ===================== LOJA =====================

Loja::Loja(string nome, string cnpj, string endereco, string telefone) {
    this->nome     = nome;
    this->cnpj     = cnpj;
    this->endereco = endereco;
    this->telefone = telefone;
}

void Loja::exibir() {
    cout << "\n===== DADOS DA LOJA =====" << endl;
    cout << "Nome: "     << nome     << endl;
    cout << "CNPJ: "     << cnpj     << endl;
    cout << "Endereco: " << endereco << endl;
    cout << "Telefone: " << telefone << endl;
    cout << "=========================\n" << endl;
}

void Loja::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== MENU PRINCIPAL =====" << endl;
        cout << "1. Gerenciar Instrumentos" << endl;
        cout << "2. Gerenciar Clientes" << endl;
        cout << "3. Gerenciar Vendas" << endl;
        cout << "0. Sair" << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: GerenciaInstrumentos::menu(conn); break;
        case 2: GerenciaClientes::menu(conn);     break;
        case 3: GerenciaVendas::menu(conn);       break;
        case 0: cout << "Encerrando o sistema. Ate logo!" << endl; break;
        default: cout << "Opcao invalida!" << endl;
        }
    }
}
