#include "../headers/venda.hpp"
#include <iostream>

using namespace std;

Venda::Venda() : id(0), clienteId(0), valorTotal(0.0) {}

Venda::Venda(int id, int clienteId, string data, double valorTotal) {
    this->id         = id;
    this->clienteId  = clienteId;
    this->data       = data;
    this->valorTotal = valorTotal;
}

// Getters
int    Venda::getId()         const { return id; }
int    Venda::getClienteId()  const { return clienteId; }
string Venda::getData()       const { return data; }
double Venda::getValorTotal() const { return valorTotal; }

const vector<ItemVenda>& Venda::getItens() const { return itens; }

// Setters
void Venda::setData(string data)       { this->data       = data; }
void Venda::setValorTotal(double val)  { this->valorTotal = val; }

// Gerenciamento de itens
void Venda::adicionarItem(const ItemVenda& item) {
    itens.push_back(item);
}

void Venda::limparItens() {
    itens.clear();
}

void Venda::recalcularTotal() {
    valorTotal = 0.0;
    for (const ItemVenda& item : itens)
        valorTotal += item.getSubtotal();
}

void Venda::exibir() const {
    cout << "ID Venda: "      << id         << endl;
    cout << "Cliente ID: "    << clienteId  << endl;
    cout << "Data: "          << data       << endl;
    cout << "Valor Total: R$ "<< valorTotal << endl;

    if (!itens.empty()) {
        cout << "Itens:" << endl;
        for (const ItemVenda& item : itens)
            item.exibir();
    }
    cout << endl;
}

void Venda::exibirSimplificado() const {
    cout << "ID: "   << id   << endl;
    cout << "Data: " << data << endl << endl;
}