#ifndef FUNCIONARIO_HPP
#define FUNCIONARIO_HPP

#include <iostream>
#include <string>

class Funcionario {
private:
    int id;
    std::string nome;
    std::string cpf;
    std::string telefone;
    std::string email;
    std::string cargo;

public:
    Funcionario();
    Funcionario(int id, std::string nome, std::string cpf,
                std::string telefone, std::string email, std::string cargo);

    // Getters
    int         getId()       const;
    std::string getNome()     const;
    std::string getCpf()      const;
    std::string getTelefone() const;
    std::string getEmail()    const;
    std::string getCargo()    const;

    // Setters
    void setNome(std::string nome);
    void setCpf(std::string cpf);
    void setTelefone(std::string telefone);
    void setEmail(std::string email);
    void setCargo(std::string cargo);

    void exibir() const;
    void exibirSimplificado() const;
};

#endif
