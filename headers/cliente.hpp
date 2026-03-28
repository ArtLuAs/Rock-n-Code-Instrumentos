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
    bool torceFlamengo;
    bool assisteOnePiece;
    std::string cidade;

public:
    Cliente();
    // Construtor legado (6 parâmetros) — mantido para compatibilidade
    Cliente(int id, std::string nome, std::string cpf,
            std::string telefone, std::string email, std::string sexo);
    // Construtor completo (9 parâmetros)
    Cliente(int id, std::string nome, std::string cpf,
            std::string telefone, std::string email, std::string sexo,
            bool torceFlamengo, bool assisteOnePiece, std::string cidade);

    // Getters
    int         getId()              const;
    std::string getNome()            const;
    std::string getCpf()             const;
    std::string getTelefone()        const;
    std::string getEmail()           const;
    std::string getSexo()            const;
    bool        getTorceFlamengo()   const;
    bool        getAssisteOnePiece() const;
    std::string getCidade()          const;

    // Setters
    void setNome(std::string nome);
    void setCpf(std::string cpf);
    void setTelefone(std::string telefone);
    void setEmail(std::string email);
    void setSexo(std::string sexo);
    void setTorceFlamengo(bool torceFlamengo);
    void setAssisteOnePiece(bool assisteOnePiece);
    void setCidade(std::string cidade);

    bool possuiDesconto() const; // true se torce Flamengo, assiste One Piece ou é de Sousa

    void exibir() const;
    void exibirSimplificado() const;
};

#endif
