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

    void menu(PGconn* conn) {
        int opcao = -1;
        while (opcao != 0) {
            cout << "\n===== MENU DE GERENCIAMENTO =====" << endl;
            cout << "1. Inserir Instrumento" << endl;
            cout << "2. Alterar Instrumento" << endl;
            cout << "3. Pesquisar Instrumento por Nome" << endl;
            cout << "4. Remover Instrumento" << endl;
            cout << "5. Listar Todos os Instrumentos" << endl;
            cout << "6. Exibir um Instrumento (por ID)" << endl;
            cout << "7. Relatorio de Estoque" << endl;
            cout << "0. Sair" << endl;
            cout << "Escolha uma opcao: ";
            cin >> opcao;

            int idBusca, qtd;
            string nome, tipo, marca;
            double preco;

            switch (opcao) {
                case 1:
                    cout << "Nome: "; cin.ignore(); getline(cin, nome);
                    cout << "Tipo (guitarra/violao/baixo): "; getline(cin, tipo);
                    cout << "Marca: "; getline(cin, marca);
                    cout << "Preco: "; cin >> preco;
                    cout << "Quantidade: "; cin >> qtd;
                    inserirInstrumento(conn, Instrumento(0, nome, tipo, marca, preco, qtd));
                    break;
                case 2:
                    cout << "ID do Instrumento a alterar: "; cin >> idBusca;
                    cout << "Novo Nome: "; cin.ignore(); getline(cin, nome);
                    cout << "Novo Tipo (guitarra/violao/baixo): "; getline(cin, tipo);
                    cout << "Nova Marca: "; getline(cin, marca);
                    cout << "Novo Preco: "; cin >> preco;
                    cout << "Nova Quantidade: "; cin >> qtd;
                    alterarInstrumento(conn, Instrumento(idBusca, nome, tipo, marca, preco, qtd));
                    break;
                case 3:
                    cout << "Digite o nome para buscar: "; cin.ignore(); getline(cin, nome);
                    pesquisarInstrumentoPorNome(conn, nome);
                    break;
                case 4:
                    cout << "ID do Instrumento a remover: "; cin >> idBusca;
                    removerInstrumento(conn, idBusca);
                    break;
                case 5:
                    listarInstrumentos(conn);
                    break;
                case 6:
                    cout << "ID do Instrumento: "; cin >> idBusca;
                    exibirInstrumento(conn, idBusca);
                    break;
                case 7:
                    relatorioEstoque(conn);
                    break;
                case 0:
                    cout << "Encerrando o sistema..." << endl;
                    break;
                default:
                    cout << "Opcao invalida!" << endl;
            }
        }
    }
};

// ===================== FUNÇÃO PRINCIPAL =====================
int main() {
    // Conecta ao banco
    PGconn* conn = conectar();
    if (conn == nullptr) {
        return 1;
    }

    Loja loja("Loja Musical", "00.000.000/0001-00", 
              "Rua das Guitarras, 123", "(11) 99999-9999");

    cout << "Sistema da Loja iniciado com sucesso!" << endl;
    loja.exibir();

    // Chamar o menu interativo passando a conexão
    loja.menu(conn);

    // Fechar a conexão ao sair do programa
    PQfinish(conn);
    return 0;
}