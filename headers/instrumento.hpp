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
    std::string categoria;
    bool fabricadoEmMari;

public:
    Instrumento();
    // Construtor legado (6 parâmetros) — mantido para compatibilidade
    Instrumento(int id, std::string nome, std::string tipo,
                std::string marca, double preco, int quantidade);
    // Construtor completo (8 parâmetros)
    Instrumento(int id, std::string nome, std::string tipo,
                std::string marca, double preco, int quantidade,
                std::string categoria, bool fabricadoEmMari);

    // Getters
    int         getId()             const;
    std::string getNome()           const;
    std::string getTipo()           const;
    std::string getMarca()          const;
    double      getPreco()          const;
    int         getQuantidade()     const;
    std::string getCategoria()      const;
    bool        getFabricadoEmMari() const;

    // Setters
    void setNome(std::string nome);
    void setTipo(std::string tipo);
    void setMarca(std::string marca);
    void setPreco(double preco);
    void setQuantidade(int quantidade);
    void setCategoria(std::string categoria);
    void setFabricadoEmMari(bool fabricadoEmMari);

    void exibir() const;
    void exibirSimplificado() const;
};

#endif
