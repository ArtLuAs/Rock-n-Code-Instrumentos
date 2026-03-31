#include "headers/lojaGerencia.hpp"
#include "headers/gerenciaInstrumentos.hpp"
#include "headers/gerenciaClientes.hpp"
#include "headers/gerenciaFuncionarios.hpp"
#include "headers/gerenciaVendas.hpp"

#include "vendor/httplib.h"
#include "vendor/json.hpp"

#include <iostream>
#include <string>

using namespace std;
using json = nlohmann::json;

// ===================== CORS =====================

static void setCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

static void jsonResp(httplib::Response& res, const json& body, int status = 200) {
    setCors(res);
    res.set_content(body.dump(), "application/json");
    res.status = status;
}

static void errResp(httplib::Response& res, const string& msg, int status = 400) {
    setCors(res);
    res.set_content(json{{"error", msg}}.dump(), "application/json");
    res.status = status;
}

// Aceita campo como string ou number, retorna sempre string
static string strVal(const json& b, const string& key, const string& def = "") {
    if (!b.contains(key) || b[key].is_null()) return def;
    if (b[key].is_string()) return b[key].get<string>();
    return b[key].dump();
}

// Aceita campo como string ou number, retorna sempre int
static int intVal(const json& b, const string& key, int def = 0) {
    if (!b.contains(key) || b[key].is_null()) return def;
    if (b[key].is_number()) return b[key].get<int>();
    if (b[key].is_string()) return atoi(b[key].get<string>().c_str());
    return def;
}

// ===================== MAIN =====================

int main() {
    PGconn* conn = conectar();
    if (!conn) {
        cerr << "Falha ao conectar ao banco de dados." << endl;
        return 1;
    }
    cout << "Backend iniciado em http://localhost:8080" << endl;

    httplib::Server svr;

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
        res.status = 204;
    });

    // ==================================================
    // POST /api/login
    // ==================================================
    svr.Post("/api/login", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body    = json::parse(req.body);
            string cpf   = strVal(body, "cpf");
            string senha = strVal(body, "senha");
            string tipo  = strVal(body, "tipo", "funcionario");

            string cpfNorm;
            for (char c : cpf)
                if (c != '.' && c != '-') cpfNorm += c;

            if (tipo == "cliente") {
                const char* p[2] = {cpfNorm.c_str(), senha.c_str()};
                PGresult* r = PQexecParams(conn,
                    "SELECT id, nome FROM clientes WHERE cpf=$1 AND senha=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) > 0) {
                    json resp = {
                        {"id",   atoi(PQgetvalue(r, 0, 0))},
                        {"nome", PQgetvalue(r, 0, 1)},
                        {"role", "cliente"}
                    };
                    PQclear(r);
                    jsonResp(res, resp);
                } else {
                    PQclear(r);
                    errResp(res, "CPF ou senha invalidos", 401);
                }
            } else {
                const char* p[2] = {cpfNorm.c_str(), senha.c_str()};
                PGresult* r = PQexecParams(conn,
                    "SELECT id, nome, cargo FROM funcionarios WHERE cpf=$1 AND senha=$2",
                    2, nullptr, p, nullptr, nullptr, 0);
                if (PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) > 0) {
                    json resp = {
                        {"id",    atoi(PQgetvalue(r, 0, 0))},
                        {"nome",  PQgetvalue(r, 0, 1)},
                        {"cargo", PQgetvalue(r, 0, 2)},
                        {"role",  "funcionario"}
                    };
                    PQclear(r);
                    jsonResp(res, resp);
                } else {
                    PQclear(r);
                    errResp(res, "CPF ou senha invalidos", 401);
                }
            }
        } catch (const exception& e) {
            errResp(res, string("Erro: ") + e.what(), 400);
        }
    });

    // ==================================================
    // POST /api/registro/cliente
    // ==================================================
    svr.Post("/api/registro/cliente", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string nome  = strVal(b, "nome");
            string cpf   = strVal(b, "cpf");
            string senha = strVal(b, "senha");
            string tel   = strVal(b, "telefone");
            string email = strVal(b, "email");
            string sexo  = strVal(b, "sexo");
            string cid   = strVal(b, "cidade");
            string flam  = (b.contains("torceFlamengo")   && b["torceFlamengo"]   == true) ? "true" : "false";
            string one   = (b.contains("assisteOnePiece") && b["assisteOnePiece"] == true) ? "true" : "false";

            if (nome.empty() || cpf.empty() || senha.empty()) {
                errResp(res, "Nome, CPF e senha sao obrigatorios"); return;
            }
            const char* chk[1] = {cpf.c_str()};
            PGresult* exist = PQexecParams(conn, "SELECT id FROM clientes WHERE cpf=$1",
                1, nullptr, chk, nullptr, nullptr, 0);
            if (PQntuples(exist) > 0) { PQclear(exist); errResp(res, "CPF ja cadastrado", 409); return; }
            PQclear(exist);

            const char* p[9] = {nome.c_str(), cpf.c_str(), tel.c_str(), email.c_str(),
                                sexo.c_str(), flam.c_str(), one.c_str(), cid.c_str(), senha.c_str()};
            PGresult* r = PQexecParams(conn,
                "INSERT INTO clientes (nome,cpf,telefone,email,sexo,torce_flamengo,assiste_one_piece,cidade,senha) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9) RETURNING id",
                9, nullptr, p, nullptr, nullptr, 0);
            if (PQresultStatus(r) == PGRES_TUPLES_OK) {
                json resp = {{"id", atoi(PQgetvalue(r, 0, 0))}, {"success", true}};
                PQclear(r); jsonResp(res, resp, 201);
            } else {
                string err = PQerrorMessage(conn); PQclear(r); errResp(res, err);
            }
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // POST /api/registro/funcionario
    // ==================================================
    svr.Post("/api/registro/funcionario", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string nome  = strVal(b, "nome");
            string cpf   = strVal(b, "cpf");
            string cargo = strVal(b, "cargo", "vendedor");
            string senha = strVal(b, "senha");

            if (nome.empty() || cpf.empty() || senha.empty()) {
                errResp(res, "Nome, CPF e senha sao obrigatorios"); return;
            }
            const char* chk[1] = {cpf.c_str()};
            PGresult* exist = PQexecParams(conn, "SELECT id FROM funcionarios WHERE cpf=$1",
                1, nullptr, chk, nullptr, nullptr, 0);
            if (PQntuples(exist) > 0) { PQclear(exist); errResp(res, "CPF ja cadastrado", 409); return; }
            PQclear(exist);

            const char* p[4] = {nome.c_str(), cpf.c_str(), cargo.c_str(), senha.c_str()};
            PGresult* r = PQexecParams(conn,
                "INSERT INTO funcionarios (nome, cpf, cargo, senha) VALUES ($1,$2,$3,$4) RETURNING id",
                4, nullptr, p, nullptr, nullptr, 0);
            if (PQresultStatus(r) == PGRES_TUPLES_OK) {
                json resp = {{"id", atoi(PQgetvalue(r, 0, 0))}, {"success", true}};
                PQclear(r); jsonResp(res, resp, 201);
            } else {
                string err = PQerrorMessage(conn); PQclear(r); errResp(res, err);
            }
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // GET /api/produtos
    // ==================================================
    svr.Get("/api/produtos", [conn](const httplib::Request&, httplib::Response& res) {
        PGresult* result = PQexec(conn,
            "SELECT id, nome, tipo, marca, preco, quantidade, categoria, fabricado_em_mari "
            "FROM instrumentos ORDER BY id;");
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",              atoi(PQgetvalue(result, i, 0))},
                {"nome",            PQgetvalue(result, i, 1)},
                {"tipo",            PQgetvalue(result, i, 2)},
                {"marca",           PQgetvalue(result, i, 3)},
                {"preco",           atof(PQgetvalue(result, i, 4))},
                {"quantidade",      atoi(PQgetvalue(result, i, 5))},
                {"categoria",       PQgetvalue(result, i, 6)},
                {"fabricadoEmMari", string(PQgetvalue(result, i, 7)) == "t"}
            });
        }
        PQclear(result);
        jsonResp(res, arr);
    });

    // ==================================================
    // POST /api/produtos
    // ==================================================
    svr.Post("/api/produtos", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string nome  = strVal(b, "nome");
            string tipo  = strVal(b, "tipo");
            string marca = strVal(b, "marca");
            string preco = strVal(b, "preco", "0");
            string qtd   = strVal(b, "quantidade", "0");
            string cat   = strVal(b, "categoria", "outros");
            string mari  = (b.contains("fabricadoEmMari") && b["fabricadoEmMari"] == true) ? "true" : "false";
            const char* p[7] = {nome.c_str(), tipo.c_str(), marca.c_str(),
                                preco.c_str(), qtd.c_str(), cat.c_str(), mari.c_str()};
            PGresult* result = PQexecParams(conn,
                "INSERT INTO instrumentos (nome,tipo,marca,preco,quantidade,categoria,fabricado_em_mari) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7) RETURNING id",
                7, nullptr, p, nullptr, nullptr, 0);
            if (PQresultStatus(result) == PGRES_TUPLES_OK) {
                json resp = {{"id", atoi(PQgetvalue(result, 0, 0))}, {"success", true}};
                PQclear(result); jsonResp(res, resp, 201);
            } else {
                string err = PQerrorMessage(conn); PQclear(result); errResp(res, err);
            }
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // PUT /api/produtos/:id
    // ==================================================
    svr.Put("/api/produtos/(\\d+)", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string id    = req.matches[1];
            string nome  = strVal(b, "nome");
            string tipo  = strVal(b, "tipo");
            string marca = strVal(b, "marca");
            string preco = strVal(b, "preco", "0");
            string qtd   = strVal(b, "quantidade", "0");
            string cat   = strVal(b, "categoria", "outros");
            string mari  = (b.contains("fabricadoEmMari") && b["fabricadoEmMari"] == true) ? "true" : "false";
            const char* p[8] = {nome.c_str(), tipo.c_str(), marca.c_str(),
                                preco.c_str(), qtd.c_str(), cat.c_str(), mari.c_str(), id.c_str()};
            PGresult* result = PQexecParams(conn,
                "UPDATE instrumentos SET nome=$1,tipo=$2,marca=$3,preco=$4,"
                "quantidade=$5,categoria=$6,fabricado_em_mari=$7 WHERE id=$8",
                8, nullptr, p, nullptr, nullptr, 0);
            json resp = {{"success", PQresultStatus(result) == PGRES_COMMAND_OK}};
            PQclear(result); jsonResp(res, resp);
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // DELETE /api/produtos/:id
    // ==================================================
    svr.Delete("/api/produtos/(\\d+)", [conn](const httplib::Request& req, httplib::Response& res) {
        string id = req.matches[1];
        const char* p[1] = {id.c_str()};
        PGresult* result = PQexecParams(conn,
            "DELETE FROM instrumentos WHERE id=$1", 1, nullptr, p, nullptr, nullptr, 0);
        json resp = {{"success", PQresultStatus(result) == PGRES_COMMAND_OK}};
        PQclear(result); jsonResp(res, resp);
    });

    // ==================================================
    // GET /api/clientes
    // ==================================================
    svr.Get("/api/clientes", [conn](const httplib::Request&, httplib::Response& res) {
        PGresult* result = PQexec(conn,
            "SELECT id, nome, cpf, telefone, email, sexo, "
            "torce_flamengo, assiste_one_piece, cidade FROM clientes ORDER BY id;");
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",              atoi(PQgetvalue(result, i, 0))},
                {"nome",            PQgetvalue(result, i, 1)},
                {"cpf",             PQgetvalue(result, i, 2)},
                {"telefone",        PQgetvalue(result, i, 3)},
                {"email",           PQgetvalue(result, i, 4)},
                {"sexo",            PQgetvalue(result, i, 5)},
                {"torceFlamengo",   string(PQgetvalue(result, i, 6)) == "t"},
                {"assisteOnePiece", string(PQgetvalue(result, i, 7)) == "t"},
                {"cidade",          PQgetvalue(result, i, 8)}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // POST /api/clientes
    // ==================================================
    svr.Post("/api/clientes", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string nome  = strVal(b, "nome");
            string cpf   = strVal(b, "cpf");
            string tel   = strVal(b, "telefone");
            string email = strVal(b, "email");
            string sexo  = strVal(b, "sexo");
            string flam  = (b.contains("torceFlamengo")   && b["torceFlamengo"]   == true) ? "true" : "false";
            string one   = (b.contains("assisteOnePiece") && b["assisteOnePiece"] == true) ? "true" : "false";
            string cid   = strVal(b, "cidade");
            const char* p[8] = {nome.c_str(), cpf.c_str(), tel.c_str(), email.c_str(),
                                sexo.c_str(), flam.c_str(), one.c_str(), cid.c_str()};
            PGresult* result = PQexecParams(conn,
                "INSERT INTO clientes (nome,cpf,telefone,email,sexo,torce_flamengo,assiste_one_piece,cidade) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8) RETURNING id",
                8, nullptr, p, nullptr, nullptr, 0);
            if (PQresultStatus(result) == PGRES_TUPLES_OK) {
                json resp = {{"id", atoi(PQgetvalue(result, 0, 0))}, {"success", true}};
                PQclear(result); jsonResp(res, resp, 201);
            } else {
                string err = PQerrorMessage(conn); PQclear(result); errResp(res, err);
            }
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // PUT /api/clientes/:id
    // ==================================================
    svr.Put("/api/clientes/(\\d+)", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b       = json::parse(req.body);
            string id    = req.matches[1];
            string nome  = strVal(b, "nome");
            string cpf   = strVal(b, "cpf");
            string tel   = strVal(b, "telefone");
            string email = strVal(b, "email");
            string sexo  = strVal(b, "sexo");
            string flam  = (b.contains("torceFlamengo")   && b["torceFlamengo"]   == true) ? "true" : "false";
            string one   = (b.contains("assisteOnePiece") && b["assisteOnePiece"] == true) ? "true" : "false";
            string cid   = strVal(b, "cidade");
            const char* p[9] = {nome.c_str(), cpf.c_str(), tel.c_str(), email.c_str(),
                                sexo.c_str(), flam.c_str(), one.c_str(), cid.c_str(), id.c_str()};
            PGresult* result = PQexecParams(conn,
                "UPDATE clientes SET nome=$1,cpf=$2,telefone=$3,email=$4,sexo=$5,"
                "torce_flamengo=$6,assiste_one_piece=$7,cidade=$8 WHERE id=$9",
                9, nullptr, p, nullptr, nullptr, 0);
            json resp = {{"success", PQresultStatus(result) == PGRES_COMMAND_OK}};
            PQclear(result); jsonResp(res, resp);
        } catch (const exception& e) { errResp(res, string("Erro: ") + e.what()); }
    });

    // ==================================================
    // DELETE /api/clientes/:id
    // ==================================================
    svr.Delete("/api/clientes/(\\d+)", [conn](const httplib::Request& req, httplib::Response& res) {
        string id = req.matches[1];
        const char* p[1] = {id.c_str()};
        PGresult* result = PQexecParams(conn,
            "DELETE FROM clientes WHERE id=$1", 1, nullptr, p, nullptr, nullptr, 0);
        json resp = {{"success", PQresultStatus(result) == PGRES_COMMAND_OK}};
        PQclear(result); jsonResp(res, resp);
    });

    // ==================================================
    // GET /api/funcionarios
    // ==================================================
    svr.Get("/api/funcionarios", [conn](const httplib::Request&, httplib::Response& res) {
        PGresult* result = PQexec(conn,
            "SELECT id, nome, cargo FROM funcionarios ORDER BY id;");
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",    atoi(PQgetvalue(result, i, 0))},
                {"nome",  PQgetvalue(result, i, 1)},
                {"cargo", PQgetvalue(result, i, 2)}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // GET /api/vendas/me?clienteId=X  (pedidos do cliente logado)
    // IMPORTANTE: deve ser registrado ANTES de GET /api/vendas
    // ==================================================
    svr.Get("/api/vendas/me", [conn](const httplib::Request& req, httplib::Response& res) {
        string clienteIdStr = req.get_param_value("clienteId");
        if (clienteIdStr.empty()) {
            errResp(res, "clienteId e obrigatorio"); return;
        }
        const char* p[1] = {clienteIdStr.c_str()};
        PGresult* result = PQexecParams(conn,
            "SELECT p.id, p.data, p.desconto, p.total "
            "FROM pedidos p "
            "WHERE p.cliente_id = $1::integer "
            "ORDER BY p.data DESC, p.id DESC;",
            1, nullptr, p, nullptr, nullptr, 0);
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",       atoi(PQgetvalue(result, i, 0))},
                {"data",     PQgetvalue(result, i, 1)},
                {"desconto", atof(PQgetvalue(result, i, 2))},
                {"total",    atof(PQgetvalue(result, i, 3))}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // GET /api/vendas  (uso do funcionario/admin)
    // ==================================================
    svr.Get("/api/vendas", [conn](const httplib::Request&, httplib::Response& res) {
        PGresult* result = PQexec(conn,
            "SELECT p.id, c.nome AS cliente, c.id AS cliente_id, f.nome AS vendedor, "
            "p.data, p.desconto, p.total "
            "FROM pedidos p "
            "JOIN clientes c     ON c.id = p.cliente_id "
            "JOIN funcionarios f ON f.id = p.funcionario_id "
            "ORDER BY p.id;");
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",        atoi(PQgetvalue(result, i, 0))},
                {"cliente",   PQgetvalue(result, i, 1)},
                {"clienteId", atoi(PQgetvalue(result, i, 2))},
                {"vendedor",  PQgetvalue(result, i, 3)},
                {"data",      PQgetvalue(result, i, 4)},
                {"desconto",  atof(PQgetvalue(result, i, 5))},
                {"total",     atof(PQgetvalue(result, i, 6))}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // POST /api/vendas
    // Body: { clienteId, funcionarioId,
    //         instrumentos: [id,...], quantidades: [qtd,...] }
    // ==================================================
    svr.Post("/api/vendas", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b = json::parse(req.body);

            int clienteId     = intVal(b, "clienteId");
            int funcionarioId = intVal(b, "funcionarioId");

            if (clienteId == 0 || funcionarioId == 0) {
                errResp(res, "clienteId e funcionarioId sao obrigatorios"); return;
            }

            auto instJson = b.at("instrumentos");
            auto qtdJson  = b.at("quantidades");

            string pgInst = "{", pgQtd = "{";
            for (size_t i = 0; i < instJson.size(); i++) {
                if (i > 0) { pgInst += ","; pgQtd += ","; }
                int instId = instJson[i].is_string()
                    ? atoi(instJson[i].get<string>().c_str())
                    : instJson[i].get<int>();
                int qtd = qtdJson[i].is_string()
                    ? atoi(qtdJson[i].get<string>().c_str())
                    : qtdJson[i].get<int>();
                pgInst += to_string(instId);
                pgQtd  += to_string(qtd);
            }
            pgInst += "}"; pgQtd += "}";

            string cliStr  = to_string(clienteId);
            string funcStr = to_string(funcionarioId);
            const char* p[4] = {cliStr.c_str(), funcStr.c_str(),
                                pgInst.c_str(), pgQtd.c_str()};
            PGresult* result = PQexecParams(conn,
                "CALL efetuar_compra($1::integer,$2::integer,"
                "$3::integer[],$4::integer[],NULL)",
                4, nullptr, p, nullptr, nullptr, 0);

            if (PQresultStatus(result) == PGRES_COMMAND_OK || PQresultStatus(result) == PGRES_TUPLES_OK) {
                PQclear(result);
                jsonResp(res, {{"success", true}}, 201);
            } else {
                string err = PQerrorMessage(conn);
                PQclear(result);
                errResp(res, err);
            }
        } catch (const exception& e) {
            errResp(res, string("Erro: ") + e.what());
        }
    });

    cout << "Endpoints ativos em http://localhost:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    PQfinish(conn);
    return 0;
}
