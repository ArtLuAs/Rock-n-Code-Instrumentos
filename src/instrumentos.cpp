#include "../headers/instrumento.hpp"

using namespace std;

Instrumento::Instrumento() {
}

Instrumento::Instrumento(int id, string nome, string tipo,
                         string marca, double preco, int quantidade) {
        this->id = id;
        this->nome = nome;
        this->tipo = tipo;
        this->marca = marca;
        this->preco = preco;
        this->quantidade = quantidade;
}

// Getters
int Instrumento::getId() const {
        return id;
}
string Instrumento::getNome() const {
        return nome;
}
string Instrumento::getTipo() const {
        return tipo;
}
string Instrumento::getMarca() const {
        return marca;
}
double Instrumento::getPreco() const {
        return preco;
}
int Instrumento::getQuantidade() const {
        return quantidade;
}

// Setters
void Instrumento::setNome(string nome) {
        this->nome = nome;
}
void Instrumento::setTipo(string tipo) {
        this->tipo = tipo;
}
void Instrumento::setMarca(string marca) {
        this->marca = marca;
}
void Instrumento::setPreco(double preco) {
        this->preco = preco;
}
void Instrumento::setQuantidade(int quantidade) {
        this->quantidade = quantidade;
}

void Instrumento::exibir() const {
        cout << "ID: " << id << endl;
        cout << "Nome: " << nome << endl;
        cout << "Tipo: " << tipo << endl;
        cout << "Marca: " << marca << endl;
        cout << "Preco: " << preco << endl;
        cout << "Quantidade: " << quantidade << endl;
}