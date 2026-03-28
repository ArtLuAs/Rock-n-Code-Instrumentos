#ifndef ITEMVENDA_HPP
#define ITEMVENDA_HPP

class ItemVenda {
private:
    int id;
    int vendaId;
    int instrumentoId;
    int quantidade;
    double precoUnitario;

public:
    ItemVenda();
    ItemVenda(int id, int vendaId, int instrumentoId,
              int quantidade, double precoUnitario);

    // Getters
    int getId() const;
    int getVendaId() const;
    int getInstrumentoId() const;
    int getQuantidade() const;
    double getPrecoUnitario() const;
    double getSubtotal() const; // quantidade * precoUnitario

    // Setters (exceto ID e vendaId)
    void setQuantidade(int quantidade);
    void setPrecoUnitario(double precoUnitario);

    void exibir() const;
};

#endif