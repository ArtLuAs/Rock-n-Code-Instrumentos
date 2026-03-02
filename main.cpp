#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ===================== CLASSE INSTRUMENTO =====================
class Instrumento {
private:
    int id;
    string nome;
    string tipo; // guitarra, violao ou baixo
    string marca;
    double preco;
    int quantidade;

public:
    Instrumento() {}
    Instrumento(int id, string nome, string tipo, string marca, double preco, int quantidade) {
        this->id = id;
        this->nome = nome;
        this->tipo = tipo;
        this->marca = marca;
        this->preco = preco;
        this->quantidade = quantidade;
    }

    // Getters e Setters
    int getId() { return id; }
    string getNome() { return nome; }
    string getTipo() { return tipo; }
    string getMarca() { return marca; }
    double getPreco() { return preco; }
    int getQuantidade() { return quantidade; }
    
    // Id atributo imutável 
    void setNome(string nome) { this->nome = nome; }
    void setTipo(string tipo) { this->tipo = tipo; }
    void setMarca(string marca) { this->marca = marca; }
    void setPreco(double preco) { this->preco = preco; }
    void setQuantidade(int quantidade) { this->quantidade = quantidade; }

    void exibir() {
        cout << "ID: " << id << endl;
        cout << "Nome: " << nome << endl;
        cout << "Tipo: " << tipo << endl;
        cout << "Marca: " << marca << endl;
        cout << "Preço: " << preco << endl;
        cout << "Quantidade: " << quantidade << endl;
    }
};

// ===================== CLASSE CLIENTE =====================
class Cliente {
private:
    int id;
    string nome;
    string cpf;
    string telefone;
    string email;

public:
    Cliente() {}
    Cliente(int id, string nome, string cpf, string telefone, string email) {
        this->id = id;
        this->nome = nome;
        this->cpf = cpf;
        this->telefone = telefone;
        this->email = email;
    }

    int getId() { return id; }
    string getNome() { return nome; }
    string getCpf() { return cpf; }
    string getTelefone() { return telefone; }
    string getEmail() { return email; }

    // Id atributo imutável 
    void setNome(string nome) { this->nome = nome; }
    void setCpf(string cpf) { this->cpf = cpf; }
    void setTelefone(string telefone) { this->telefone = telefone; }
    void setEmail(string email) { this->email = email; }

    void exibir() {
        cout << "ID: " << id << endl;
        cout << "Nome: " << nome << endl;
        cout << "CPF: " << cpf << endl;
        cout << "Telefone: " << telefone << endl;
        cout << "Email: " << email << endl;
    }
};

// ===================== CLASSE VENDA =====================
class Venda {
private:
    int id;
    int clienteId;
    int instrumentoId;
    int quantidade;
    double valorTotal;
    string data;

public:
    Venda() {}
    Venda(int id, int clienteId, int instrumentoId, int quantidade, double valorTotal, string data) {
        this->id = id;
        this->clienteId = clienteId;
        this->instrumentoId = instrumentoId;
        this->quantidade = quantidade;
        this->valorTotal = valorTotal;
        this->data = data;
    }

    int getId() { return id; }
    int getClienteId() { return clienteId; }
    int getInstrumentoId() { return instrumentoId; }
    int getQuantidade() { return quantidade; }
    double getValorTotal() { return valorTotal; }
    string getData() { return data; }

    // Id atributo imutável, chave estrangeira imutável
    void setQuantidade(int quantidade) { this->quantidade = quantidade; }
    void setValorTotal(double valorTotal) { this->valorTotal = valorTotal; }

    void calcularTotal(double precoUnitario) {
        valorTotal = precoUnitario * quantidade;
    }

    void exibir() {
        cout << "ID Venda: " << id << endl;
        cout << "Cliente ID: " << clienteId << endl;
        cout << "Instrumento ID: " << instrumentoId << endl;
        cout << "Quantidade: " << quantidade << endl;
        cout << "Valor Total: " << valorTotal << endl;
        cout << "Data: " << data << endl;
    }
};

// ===================== CLASSE LOJA (GERENCIADOR CRUD) =====================
class Loja {
private:
    string nome;
    string cnpj;
    string endereco;
    string telefone;

public:
    Loja(string nome, string cnpj, string endereco, string telefone) {
        this->nome = nome;
        this->cnpj = cnpj;
        this->endereco = endereco;
        this->telefone = telefone;
    }

    // ================= CRUD INSTRUMENTO =================
    void inserirInstrumento(Instrumento instrumento) {
        // Implementar inserção no banco
    }

    void alterarInstrumento(int id) {
        // Implementar alteração no banco
    }

    void pesquisarInstrumentoPorNome(string nome) {
        // Implementar busca no banco
    }

    void removerInstrumento(int id) {
        // Implementar remoção no banco
    }

    void listarInstrumentos() {
        // Implementar listagem no banco
    }

    void exibirInstrumento(int id) {
        // Implementar exibição de um registro
    }

    // ================= CRUD CLIENTE =================
    void inserirCliente(Cliente cliente) {
        // Implementar inserção no banco
    }

    void alterarCliente(int id) {
        // Implementar alteração no banco
    }

    void pesquisarClientePorNome(string nome) {
        // Implementar busca no banco
    }

    void removerCliente(int id) {
        // Implementar remoção
    }

    void listarClientes() {
        // Implementar listagem
    }

    void exibirCliente(int id) {
        // Implementar exibição de um
    }

    // ================= CRUD VENDA =================
    void inserirVenda(Venda venda) {
        // Implementar inserção
    }

    void alterarVenda(int id) {
        // Implementar alteração
    }

    void pesquisarVendaPorNome(string nomeCliente) {
        // Implementar busca
    }

    void removerVenda(int id) {
        // Implementar remoção
    }

    void listarVendas() {
        // Implementar listagem
    }

    void exibirVenda(int id) {
        // Implementar exibição
    }

    // ================= RELATÓRIOS =================
    void relatorioEstoque() {
        // Exibir quantidade total de instrumentos
        // Exibir valor total em estoque
    }

    void relatorioClientes() {
        // Exibir total de clientes cadastrados
    }

    void relatorioVendas() {
        // Exibir total de vendas realizadas
        // Exibir valor total vendido
    }
    
    void exibir() {
    cout << "===== DADOS DA LOJA =====" << endl;
    cout << "Nome: " << nome << endl;
    cout << "CNPJ: " << cnpj << endl;
    cout << "Endereço: " << endereco << endl;
    cout << "Telefone: " << telefone << endl;
}
};

// ===================== FUNÇÃO PRINCIPAL =====================
int main() {

    Loja loja("Loja Musical", "00.000.000/0001-00", 
              "Rua das Guitarras, 123", "(11) 99999-9999");

    cout << "Sistema da Loja iniciado com sucesso!" << endl;
    
    loja.exibir();

    return 0;
}