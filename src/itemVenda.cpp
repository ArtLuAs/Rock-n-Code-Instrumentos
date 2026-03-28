#include "../headers/itemVenda.hpp"
#include <iostream>

using namespace std;

ItemVenda::ItemVenda() {}

ItemVenda::ItemVenda(int id, int vendaId, int instrumentoId,
                     int quantidade, double precoUnitario) {
    this->id             = id;
    this->vendaId        = vendaId;
    this->instrumentoId  = instrumentoId;
    this->quantidade     = quantidade;
    this->precoUnitario  = precoUnitario;
}

// Getters
int    ItemVenda::getId()             const { return id; }
int    ItemVenda::getVendaId()        const { return vendaId; }
int    ItemVenda::getInstrumentoId()  const { return instrumentoId; }
int    ItemVenda::getQuantidade()     const { return quantidade; }
double ItemVenda::getPrecoUnitario()  const { return precoUnitario; }
double ItemVenda::getSubtotal()       const { return quantidade * precoUnitario; }

// Setters
void ItemVenda::setQuantidade(int quantidade)         { this->quantidade    = quantidade; }
void ItemVenda::setPrecoUnitario(double precoUnit)    { this->precoUnitario = precoUnit; }

void ItemVenda::exibir() const {
    cout << "  Item ID: "         << id             << endl;
    cout << "  Instrumento ID: "  << instrumentoId  << endl;
    cout << "  Quantidade: "      << quantidade     << endl;
    cout << "  Preco Unitario: R$ "<< precoUnitario << endl;
    cout << "  Subtotal: R$ "     << getSubtotal()  << endl;
}