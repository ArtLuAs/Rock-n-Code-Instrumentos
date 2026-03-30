#include "../headers/instrumento.hpp"

using namespace std;

Instrumento::Instrumento()
    : id(0), preco(0.0), quantidade(0), categoria("outros"), fabricadoEmMari(false) {}

Instrumento::Instrumento(int id, string nome, string tipo,
                         string marca, double preco, int quantidade)
    : id(id), nome(nome), tipo(tipo), marca(marca),
      preco(preco), quantidade(quantidade),
      categoria("outros"), fabricadoEmMari(false) {}

Instrumento::Instrumento(int id, string nome, string tipo,
                         string marca, double preco, int quantidade,
                         string categoria, bool fabricadoEmMari)
    : id(id), nome(nome), tipo(tipo), marca(marca),
      preco(preco), quantidade(quantidade),
      categoria(categoria), fabricadoEmMari(fabricadoEmMari) {}

// Getters
int         Instrumento::getId()              const { return id; }
string      Instrumento::getNome()            const { return nome; }
string      Instrumento::getTipo()            const { return tipo; }
string      Instrumento::getMarca()           const { return marca; }
double      Instrumento::getPreco()           const { return preco; }
int         Instrumento::getQuantidade()      const { return quantidade; }
string      Instrumento::getCategoria()       const { return categoria; }
bool        Instrumento::getFabricadoEmMari() const { return fabricadoEmMari; }

// Setters
void Instrumento::setNome(string nome)                       { this->nome = nome; }
void Instrumento::setTipo(string tipo)                       { this->tipo = tipo; }
void Instrumento::setMarca(string marca)                     { this->marca = marca; }
void Instrumento::setPreco(double preco)                     { this->preco = preco; }
void Instrumento::setQuantidade(int quantidade)               { this->quantidade = quantidade; }
void Instrumento::setCategoria(string categoria)             { this->categoria = categoria; }
void Instrumento::setFabricadoEmMari(bool fabricadoEmMari)   { this->fabricadoEmMari = fabricadoEmMari; }

void Instrumento::exibir() const {
    cout << "ID: "          << id                                      << endl;
    cout << "Nome: "        << nome                                    << endl;
    cout << "Tipo: "        << tipo                                    << endl;
    cout << "Marca: "       << marca                                   << endl;
    cout << "Preco: R$ "    << preco                                   << endl;
    cout << "Quantidade: "  << quantidade                              << endl;
    cout << "Categoria: "   << categoria                               << endl;
    cout << "Fabricado em Mari: " << (fabricadoEmMari ? "Sim" : "Nao") << endl;
    cout << endl;
}

void Instrumento::exibirSimplificado() const {
    cout << "[" << id << "] " << nome << " | " << tipo << " | R$ " << preco << endl;
}
