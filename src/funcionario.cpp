#include "../headers/funcionario.hpp"

using namespace std;

Funcionario::Funcionario()
    : id(0) {}

Funcionario::Funcionario(int id, string nome, string cpf,
                         string telefone, string email, string cargo)
    : id(id), nome(nome), cpf(cpf),
      telefone(telefone), email(email), cargo(cargo) {}

// Getters
int    Funcionario::getId()       const { return id; }
string Funcionario::getNome()     const { return nome; }
string Funcionario::getCpf()      const { return cpf; }
string Funcionario::getTelefone() const { return telefone; }
string Funcionario::getEmail()    const { return email; }
string Funcionario::getCargo()    const { return cargo; }

// Setters
void Funcionario::setNome(string nome)         { this->nome = nome; }
void Funcionario::setCpf(string cpf)           { this->cpf = cpf; }
void Funcionario::setTelefone(string telefone) { this->telefone = telefone; }
void Funcionario::setEmail(string email)       { this->email = email; }
void Funcionario::setCargo(string cargo)       { this->cargo = cargo; }

void Funcionario::exibir() const {
    cout << "ID: "       << id       << endl;
    cout << "Nome: "     << nome     << endl;
    cout << "CPF: "      << cpf      << endl;
    cout << "Telefone: " << telefone << endl;
    cout << "Email: "    << email    << endl;
    cout << "Cargo: "    << cargo    << endl;
    cout << endl;
}

void Funcionario::exibirSimplificado() const {
    cout << "[" << id << "] " << nome << " | " << cargo << endl;
}
