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

// ===================== MENU FUNCIONÁRIO =====================

void Loja::menuFuncionario(PGconn* conn, int funcionarioId) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== MENU FUNCIONARIO =====" << endl;
        cout << "1. Gerenciar Instrumentos"  << endl;
        cout << "2. Gerenciar Clientes"      << endl;
        cout << "3. Gerenciar Funcionarios"  << endl;
        cout << "4. Gerenciar Vendas"        << endl;
        cout << "0. Sair (logout)"           << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: GerenciaInstrumentos::menu(conn);  break;
        case 2: GerenciaClientes::menu(conn);      break;
        case 3: GerenciaFuncionarios::menu(conn);  break;
        case 4: GerenciaVendas::menu(conn);        break;
        case 0: cout << "Saindo do modo funcionario..." << endl; break;
        default: cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU CLIENTE =====================

void Loja::menuCliente(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== MENU CLIENTE =====" << endl;
        cout << "1. Ver Instrumentos"        << endl;
        cout << "2. Pesquisar Instrumento"   << endl;
        cout << "3. Realizar Compra"         << endl;
        cout << "4. Ver meu historico"       << endl;
        cout << "0. Voltar"                  << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: GerenciaInstrumentos::listar(conn); break;
        case 2: {
            string busca;
            cout << "Buscar por [nome/tipo/marca/categoria]: ";
            cin.ignore();
            getline(cin, busca);
            GerenciaInstrumentos::pesquisar(conn, busca);
            break;
        }
        case 3: GerenciaVendas::menu(conn);  break;
        case 4: GerenciaVendas::historicoCliente(conn); break;
        case 0: cout << "Voltando ao menu inicial..." << endl; break;
        default: cout << "Opcao invalida!" << endl;
        }
    }
}

// ===================== MENU INICIAL =====================

void Loja::menu(PGconn* conn) {
    int opcao = -1;
    while (opcao != 0) {
        cout << "\n===== BEM-VINDO A ROCK N CODE =====" << endl;
        cout << "1. Entrar como Funcionario" << endl;
        cout << "2. Continuar como Cliente"  << endl;
        cout << "0. Sair"                    << endl;
        cout << "\nEscolha uma opcao: ";
        cin >> opcao;
        cout << endl;

        switch (opcao) {
        case 1: {
            int funcionarioId = -1;
            if (GerenciaFuncionarios::autenticar(conn, funcionarioId))
                menuFuncionario(conn, funcionarioId);
            break;
        }
        case 2:
            menuCliente(conn);
            break;
        case 0:
            cout << "Encerrando o sistema. Ate logo!" << endl;
            break;
        default:
            cout << "Opcao invalida!" << endl;
        }
    }
}
