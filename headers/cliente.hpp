#ifndef CLIENTE_HPP
#define CLIENTE_HPP

#include <iostream>
#include <string>

class Cliente {
private:
    int id;
    std::string nome;
    std::string cpf;
    std::string telefone;
    std::string email;
    std::string sexo;

public:
    Cliente();
    Cliente(int id, std::string nome, std::string cpf,
            std::string telefone, std::string email, std::string sexo);

    // Getters
    int getId() const;
    std::string getNome() const;
    std::string getCpf() const;
    std::string getTelefone() const;
    std::string getEmail() const;
    std::string getSexo() const;

    // Setters (exceto ID)
    void setNome(std::string nome);
    void setCpf(std::string cpf);
    void setTelefone(std::string telefone);
    void setEmail(std::string email);
    void setSexo(std::string sexo);

    void exibir() const;
    void exibirSimplificado() const;
};

#endif