#ifndef VENDA_HPP
#define VENDA_HPP

#include "itemVenda.hpp"
#include <string>
#include <vector>

class Venda {
private:
    int id;
    int clienteId;
    std::string data;
    double valorTotal;               // calculado a partir dos itens
    std::vector<ItemVenda> itens;    // itens que compõem esta venda

public:
    Venda();
    Venda(int id, int clienteId, std::string data, double valorTotal);

    // Getters
    int getId() const;
    int getClienteId() const;
    std::string getData() const;
    double getValorTotal() const;
    const std::vector<ItemVenda>& getItens() const;

    // Setters (exceto ID e clienteId)
    void setData(std::string data);
    void setValorTotal(double valorTotal);

    // Gerenciamento de itens em memória
    void adicionarItem(const ItemVenda& item);
    void limparItens();
    void recalcularTotal(); // soma os subtotais de cada ItemVenda

    void exibir() const;
    void exibirSimplificado() const;
};

#endif