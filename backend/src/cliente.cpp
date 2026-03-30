#include "../headers/cliente.hpp"
#include <algorithm>
#include <cctype>

using namespace std;

static string toLowerStr(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c){ return tolower(c); });
    return s;
}

Cliente::Cliente()
    : id(0), torceFlamengo(false), assisteOnePiece(false) {}

Cliente::Cliente(int id, string nome, string cpf,
                 string telefone, string email, string sexo)
    : id(id), nome(nome), cpf(cpf), telefone(telefone),
      email(email), sexo(sexo),
      torceFlamengo(false), assisteOnePiece(false), cidade("") {}

Cliente::Cliente(int id, string nome, string cpf,
                 string telefone, string email, string sexo,
                 bool torceFlamengo, bool assisteOnePiece, string cidade)
    : id(id), nome(nome), cpf(cpf), telefone(telefone),
      email(email), sexo(sexo),
      torceFlamengo(torceFlamengo), assisteOnePiece(assisteOnePiece),
      cidade(cidade) {}

// Getters
int     Cliente::getId()              const { return id; }
string  Cliente::getNome()            const { return nome; }
string  Cliente::getCpf()             const { return cpf; }
string  Cliente::getTelefone()        const { return telefone; }
string  Cliente::getEmail()           const { return email; }
string  Cliente::getSexo()            const { return sexo; }
bool    Cliente::getTorceFlamengo()   const { return torceFlamengo; }
bool    Cliente::getAssisteOnePiece() const { return assisteOnePiece; }
string  Cliente::getCidade()          const { return cidade; }

// Setters
void Cliente::setNome(string nome)                       { this->nome = nome; }
void Cliente::setCpf(string cpf)                         { this->cpf = cpf; }
void Cliente::setTelefone(string telefone)               { this->telefone = telefone; }
void Cliente::setEmail(string email)                     { this->email = email; }
void Cliente::setSexo(string sexo)                       { this->sexo = sexo; }
void Cliente::setTorceFlamengo(bool v)                   { this->torceFlamengo = v; }
void Cliente::setAssisteOnePiece(bool v)                 { this->assisteOnePiece = v; }
void Cliente::setCidade(string cidade)                   { this->cidade = cidade; }

bool Cliente::possuiDesconto() const {
    return torceFlamengo || assisteOnePiece || toLowerStr(cidade) == "sousa";
}

void Cliente::exibir() const {
    cout << "ID: "              << id                                        << endl;
    cout << "Nome: "            << nome                                      << endl;
    cout << "CPF: "             << cpf                                       << endl;
    cout << "Telefone: "        << telefone                                  << endl;
    cout << "Email: "           << email                                     << endl;
    cout << "Sexo: "            << sexo                                      << endl;
    cout << "Cidade: "          << (cidade.empty() ? "-" : cidade)           << endl;
    cout << "Torce Flamengo: "  << (torceFlamengo   ? "Sim" : "Nao")         << endl;
    cout << "Assiste One Piece: " << (assisteOnePiece ? "Sim" : "Nao")       << endl;
    cout << "Desconto: "        << (possuiDesconto() ? "Sim (10%)" : "Nao")  << endl;
    cout << endl;
}

void Cliente::exibirSimplificado() const {
    cout << "[" << id << "] " << nome << " | CPF: " << cpf
         << (possuiDesconto() ? " | DESCONTO" : "") << endl;
}
