#include "../headers/cliente.hpp"

using namespace std;

Cliente::Cliente() {}

Cliente::Cliente(int id, string nome, string cpf,
                 string telefone, string email, string sexo) {
    this->id = id;
    this->nome = nome;
    this->cpf = cpf;
    this->telefone = telefone;
    this->email = email;
    this->sexo = sexo;
}

// Getters
int Cliente::getId() const { return id; }
string Cliente::getNome() const { return nome; }
string Cliente::getCpf() const { return cpf; }
string Cliente::getTelefone() const { return telefone; }
string Cliente::getEmail() const { return email; }
string Cliente::getSexo() const { return sexo; }

// Setters
void Cliente::setNome(string nome) { this->nome = nome; }
void Cliente::setCpf(string cpf) { this->cpf = cpf; }
void Cliente::setTelefone(string telefone) { this->telefone = telefone; }
void Cliente::setEmail(string email) { this->email = email; }
void Cliente::setSexo(string sexo) { this->sexo = sexo; }

void Cliente::exibir() const {
    cout << "ID: "       << id       << endl;
    cout << "Nome: "     << nome     << endl;
    cout << "CPF: "      << cpf      << endl;
    cout << "Telefone: " << telefone << endl;
    cout << "Email: "    << email    << endl;
    cout << "Sexo: "     << sexo     << endl << endl;
}

void Cliente::exibirSimplificado() const {
    cout << "ID: "   << id   << endl;
    cout << "Nome: " << nome << endl << endl;
}