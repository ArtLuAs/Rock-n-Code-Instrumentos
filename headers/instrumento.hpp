#ifndef INSTRUMENTO_HPP
#define INSTRUMENTO_HPP

#include <iostream>
#include <string>

class Instrumento {
private:
    int id;
    std::string nome;
    std::string tipo;
    std::string marca;
    double preco;
    int quantidade;

public:
    Instrumento();
    Instrumento(int id, std::string nome, std::string tipo, 
                std::string marca, double preco, int quantidade);

    // Getters
    int getId() const;
    std::string getNome() const;
    std::string getTipo() const;
    std::string getMarca() const;
    double getPreco() const;
    int getQuantidade() const;

    // Setters (exceto ID)
    void setNome(std::string nome);
    void setTipo(std::string tipo);
    void setMarca(std::string marca);
    void setPreco(double preco);
    void setQuantidade(int quantidade);

    void exibir() const;
};

#endif