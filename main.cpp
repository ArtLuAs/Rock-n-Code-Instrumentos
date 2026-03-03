#include <iostream>
#include <string>
#include <vector>
#include <libpq-fe.h>

using namespace std;

PGconn* conectar() {
    PGconn* conn = PQconnectdb(
        "host=localhost port=5432 dbname=loja_musical user=lojamusical_user password=SenhaSegura123"
    );

    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Erro na conexão: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

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

// ===================== CLASSE LOJA (GERENCIADOR CRUD) =====================
class Loja {
private:
    string nome;
    string cnpj;
    string endereco;
    string telefone;

    // Função auxiliar para verificar erros de execução do libpq
    void checarErro(PGconn* conn, PGresult* res, const string& operacao) {
        if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res) != PGRES_TUPLES_OK) {
            cout << "Erro ao " << operacao << ": " << PQerrorMessage(conn) << endl;
        }
    }

public:
    Loja(string nome, string cnpj, string endereco, string telefone) {
        this->nome = nome;
        this->cnpj = cnpj;
        this->endereco = endereco;
        this->telefone = telefone;
    }

    // ================= CRUD INSTRUMENTO =================
    void inserirInstrumento(PGconn* conn, Instrumento inst) {
        string query = "INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade) VALUES ('" +
                       inst.getNome() + "', '" + inst.getTipo() + "', '" + inst.getMarca() + "', " +
                       to_string(inst.getPreco()) + ", " + to_string(inst.getQuantidade()) + ");";
        PGresult* res = PQexec(conn, query.c_str());
        checarErro(conn, res, "inserir instrumento");
        PQclear(res);
    }

    void alterarInstrumento(PGconn* conn, Instrumento inst) {
        string query = "UPDATE instrumentos SET nome = '" + inst.getNome() + 
                       "', tipo = '" + inst.getTipo() + 
                       "', marca = '" + inst.getMarca() + 
                       "', preco = " + to_string(inst.getPreco()) + 
                       ", quantidade = " + to_string(inst.getQuantidade()) + 
                       " WHERE id = " + to_string(inst.getId()) + ";";
        PGresult* res = PQexec(conn, query.c_str());
        checarErro(conn, res, "alterar instrumento");
        PQclear(res);
    }

    void pesquisarInstrumentoPorNome(PGconn* conn, string nomeBusca) {
        string query = "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos WHERE nome ILIKE '%" + nomeBusca + "%';";
        PGresult* res = PQexec(conn, query.c_str());
        checarErro(conn, res, "pesquisar instrumento");
        
        int rows = PQntuples(res);
        for(int i = 0; i < rows; i++) {
            Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1), PQgetvalue(res, i, 2),
                        PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5))).exibir();
        }
        PQclear(res);
    }

    void removerInstrumento(PGconn* conn, int id) {
        string query = "DELETE FROM instrumentos WHERE id = " + to_string(id) + ";";
        PGresult* res = PQexec(conn, query.c_str());
        checarErro(conn, res, "remover instrumento");
        PQclear(res);
    }

    void listarInstrumentos(PGconn* conn) {
        PGresult* res = PQexec(conn, "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos ORDER BY id;");
        checarErro(conn, res, "listar instrumentos");
        
        int rows = PQntuples(res);
        for(int i = 0; i < rows; i++) {
            Instrumento(atoi(PQgetvalue(res, i, 0)), PQgetvalue(res, i, 1), PQgetvalue(res, i, 2),
                        PQgetvalue(res, i, 3), atof(PQgetvalue(res, i, 4)), atoi(PQgetvalue(res, i, 5))).exibir();
        }
        PQclear(res);
    }

    void exibirInstrumento(PGconn* conn, int id) {
        string query = "SELECT id, nome, tipo, marca, preco, quantidade FROM instrumentos WHERE id = " + to_string(id) + ";";
        PGresult* res = PQexec(conn, query.c_str());
        checarErro(conn, res, "exibir instrumento");
        
        if (PQntuples(res) > 0) {
            Instrumento(atoi(PQgetvalue(res, 0, 0)), PQgetvalue(res, 0, 1), PQgetvalue(res, 0, 2),
                        PQgetvalue(res, 0, 3), atof(PQgetvalue(res, 0, 4)), atoi(PQgetvalue(res, 0, 5))).exibir();
        } else {
            cout << "Instrumento não encontrado." << endl;
        }
        PQclear(res);
    }

    // ================= RELATÓRIOS =================
    void relatorioEstoque(PGconn* conn) {
        PGresult* res = PQexec(conn, "SELECT SUM(quantidade), SUM(preco * quantidade) FROM instrumentos;");
        checarErro(conn, res, "gerar relatório de estoque");
        
        if (PQntuples(res) > 0) {
            cout << "\n--- RELATÓRIO DE ESTOQUE ---\n" << endl;
            cout << "Quantidade Total de Instrumentos: " << PQgetvalue(res, 0, 0) << endl;
            cout << "Valor Total em Estoque: R$ " << PQgetvalue(res, 0, 1) << endl;
            cout << "\n----------------------------\n" << endl;
        }
        PQclear(res);
    }
    
    void exibir() {
        cout << "\n===== DADOS DA LOJA =====" << endl;
        cout << "Nome: " << nome << endl;
        cout << "CNPJ: " << cnpj << endl;
        cout << "Endereço: " << endereco << endl;
        cout << "Telefone: " << telefone << endl;
        cout << "=========================\n" << endl;
    }
};

// ===================== FUNÇÃO PRINCIPAL =====================
int main() {

    // Conecta no banco
    PGconn* conn = conectar();
    if (!conn) return 1;

    Loja loja("Loja Musical", "00.000.000/0001-00", 
              "Rua das Guitarras, 123", "(11) 99999-9999");

    cout << "Sistema da Loja iniciado com sucesso!" << endl;
    loja.exibir();

    // ===================== INSERIR INSTRUMENTOS =====================
    loja.inserirInstrumento(conn, Instrumento(0, "Stratocaster", "guitarra", "Fender", 7500.00, 5));
    loja.inserirInstrumento(conn, Instrumento(0, "Les Paul", "guitarra", "Gibson", 12000.00, 3));
    loja.inserirInstrumento(conn, Instrumento(0, "Violão Folk", "violao", "Yamaha", 1500.00, 10));
    loja.inserirInstrumento(conn, Instrumento(0, "Baixo Precision", "baixo", "Fender", 4000.00, 4));
    loja.inserirInstrumento(conn, Instrumento(0, "Violão Clássico", "violao", "Cordoba", 1800.00, 7));

    loja.relatorioEstoque(conn);

    PQfinish(conn); // Fecha a conexão
    return 0;
}