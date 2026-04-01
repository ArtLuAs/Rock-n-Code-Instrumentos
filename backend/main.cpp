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

static void setCors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS");
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

static string strVal(const json& b, const string& key, const string& def = "") {
    if (!b.contains(key) || b[key].is_null()) return def;
    if (b[key].is_string()) return b[key].get<string>();
    return b[key].dump();
}

static int intVal(const json& b, const string& key, int def = 0) {
    if (!b.contains(key) || b[key].is_null()) return def;
    if (b[key].is_number()) return b[key].get<int>();
    if (b[key].is_string()) return atoi(b[key].get<string>().c_str());
    return def;
}

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
                    PQclear(r); jsonResp(res, resp);
                } else {
                    PQclear(r); errResp(res, "CPF ou senha invalidos", 401);
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
                    PQclear(r); jsonResp(res, resp);
                } else {
                    PQclear(r); errResp(res, "CPF ou senha invalidos", 401);
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
        PQclear(result); jsonResp(res, arr);
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
    // GET /api/clientes/:id
    // ==================================================
    svr.Get("/api/clientes/(\\d+)", [conn](const httplib::Request& req, httplib::Response& res) {
        string id = req.matches[1];
        const char* p[1] = {id.c_str()};
        PGresult* result = PQexecParams(conn,
            "SELECT id, nome, cpf, telefone, email, sexo, "
            "torce_flamengo, assiste_one_piece, cidade "
            "FROM clientes WHERE id=$1",
            1, nullptr, p, nullptr, nullptr, 0);
        if (PQresultStatus(result) != PGRES_TUPLES_OK || PQntuples(result) == 0) {
            PQclear(result); errResp(res, "Cliente nao encontrado", 404); return;
        }
        json resp = {
            {"id",              atoi(PQgetvalue(result, 0, 0))},
            {"nome",            PQgetvalue(result, 0, 1)},
            {"cpf",             PQgetvalue(result, 0, 2)},
            {"telefone",        PQgetvalue(result, 0, 3)},
            {"email",           PQgetvalue(result, 0, 4)},
            {"sexo",            PQgetvalue(result, 0, 5)},
            {"torceFlamengo",   string(PQgetvalue(result, 0, 6)) == "t"},
            {"assisteOnePiece", string(PQgetvalue(result, 0, 7)) == "t"},
            {"cidade",          PQgetvalue(result, 0, 8)}
        };
        PQclear(result); jsonResp(res, resp);
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
            PGresult* r = PQexecParams(conn,
                "INSERT INTO clientes (nome,cpf,telefone,email,sexo,torce_flamengo,assiste_one_piece,cidade) "
                "VALUES ($1,$2,$3,$4,$5,$6,$7,$8) RETURNING id",
                8, nullptr, p, nullptr, nullptr, 0);
            if (PQresultStatus(r) == PGRES_TUPLES_OK) {
                json resp = {{"id", atoi(PQgetvalue(r, 0, 0))}, {"success", true}};
                PQclear(r); jsonResp(res, resp, 201);
            } else {
                string err = PQerrorMessage(conn); PQclear(r); errResp(res, err);
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
    // GET /api/vendas/me?clienteId=X
    // IMPORTANTE: registrado ANTES de GET /api/vendas
    // ==================================================
    svr.Get("/api/vendas/me", [conn](const httplib::Request& req, httplib::Response& res) {
        string clienteIdStr = req.get_param_value("clienteId");
        if (clienteIdStr.empty()) { errResp(res, "clienteId e obrigatorio"); return; }
        const char* p[1] = {clienteIdStr.c_str()};
        PGresult* result = PQexecParams(conn,
            "SELECT p.id, p.data, p.forma_pagamento::text, p.status_pagamento::text, p.desconto, p.total "
            "FROM pedidos p "
            "WHERE p.cliente_id = $1::integer "
            "ORDER BY p.data DESC, p.id DESC;",
            1, nullptr, p, nullptr, nullptr, 0);
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",              atoi(PQgetvalue(result, i, 0))},
                {"data",            PQgetvalue(result, i, 1)},
                {"formaPagamento",  PQgetvalue(result, i, 2)},
                {"statusPagamento", PQgetvalue(result, i, 3)},
                {"desconto",        atof(PQgetvalue(result, i, 4))},
                {"total",           atof(PQgetvalue(result, i, 5))}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // GET /api/vendas
    // ==================================================
    svr.Get("/api/vendas", [conn](const httplib::Request&, httplib::Response& res) {
        PGresult* result = PQexec(conn,
            "SELECT p.id, c.nome AS cliente, c.id AS cliente_id, "
            "COALESCE(f.nome, '—') AS vendedor, "
            "p.data, p.forma_pagamento::text, p.status_pagamento::text, p.desconto, p.total "
            "FROM pedidos p "
            "JOIN clientes c ON c.id = p.cliente_id "
            "LEFT JOIN funcionarios f ON f.id = p.funcionario_id "
            "ORDER BY p.id;");
        json arr = json::array();
        int rows = PQntuples(result);
        for (int i = 0; i < rows; i++) {
            arr.push_back({
                {"id",              atoi(PQgetvalue(result, i, 0))},
                {"cliente",         PQgetvalue(result, i, 1)},
                {"clienteId",       atoi(PQgetvalue(result, i, 2))},
                {"vendedor",        PQgetvalue(result, i, 3)},
                {"data",            PQgetvalue(result, i, 4)},
                {"formaPagamento",  PQgetvalue(result, i, 5)},
                {"statusPagamento", PQgetvalue(result, i, 6)},
                {"desconto",        atof(PQgetvalue(result, i, 7))},
                {"total",           atof(PQgetvalue(result, i, 8))}
            });
        }
        PQclear(result); jsonResp(res, arr);
    });

    // ==================================================
    // PATCH /api/vendas/:id/status
    // Body: { status: "confirmado"|"recusado"|"pendente", funcionarioId: N }
    // ==================================================
    svr.Patch("/api/vendas/(\\d+)/status", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            string id     = req.matches[1];
            auto b        = json::parse(req.body);
            string status = strVal(b, "status");

            if (status != "pendente" && status != "confirmado" && status != "recusado") {
                errResp(res, "Status invalido. Use: pendente, confirmado ou recusado"); return;
            }

            PGresult* result;
            bool temFuncionario = b.contains("funcionarioId") && !b["funcionarioId"].is_null();

            if (temFuncionario && status != "pendente") {
                string funcId = to_string(intVal(b, "funcionarioId"));
                const char* p[3] = {status.c_str(), funcId.c_str(), id.c_str()};
                result = PQexecParams(conn,
                    "UPDATE pedidos "
                    "SET status_pagamento = $1::status_pgto, funcionario_id = $2::integer "
                    "WHERE id = $3::integer",
                    3, nullptr, p, nullptr, nullptr, 0);
            } else {
                const char* p[2] = {status.c_str(), id.c_str()};
                result = PQexecParams(conn,
                    "UPDATE pedidos "
                    "SET status_pagamento = $1::status_pgto, funcionario_id = NULL "
                    "WHERE id = $2::integer",
                    2, nullptr, p, nullptr, nullptr, 0);
            }

            if (PQresultStatus(result) == PGRES_COMMAND_OK) {
                PQclear(result);
                jsonResp(res, {{"success", true}, {"status", status}});
            } else {
                string err = PQerrorMessage(conn);
                PQclear(result);
                errResp(res, err);
            }
        } catch (const exception& e) {
            errResp(res, string("Erro: ") + e.what());
        }
    });

    // ==================================================
    // POST /api/vendas
    // ==================================================
    svr.Post("/api/vendas", [conn](const httplib::Request& req, httplib::Response& res) {
        try {
            auto b = json::parse(req.body);

            int clienteId = intVal(b, "clienteId");
            if (clienteId == 0) {
                errResp(res, "clienteId e obrigatorio"); return;
            }

            bool temFunc = b.contains("funcionarioId") && !b["funcionarioId"].is_null()
                           && intVal(b, "funcionarioId") != 0;
            string funcStr   = temFunc ? to_string(intVal(b, "funcionarioId")) : "";
            string formaPgto = strVal(b, "formaPagamento", "dinheiro");

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

            string cliStr = to_string(clienteId);
            PGresult* result;

            if (temFunc) {
                const char* p[5] = {cliStr.c_str(), funcStr.c_str(),
                                    pgInst.c_str(), pgQtd.c_str(), formaPgto.c_str()};
                result = PQexecParams(conn,
                    "CALL efetuar_compra($1::integer,$2::integer,"
                    "$3::integer[],$4::integer[],NULL,$5::varchar)",
                    5, nullptr, p, nullptr, nullptr, 0);
            } else {
                const char* p[4] = {cliStr.c_str(),
                                    pgInst.c_str(), pgQtd.c_str(), formaPgto.c_str()};
                result = PQexecParams(conn,
                    "CALL efetuar_compra($1::integer,NULL,"
                    "$2::integer[],$3::integer[],NULL,$4::varchar)",
                    4, nullptr, p, nullptr, nullptr, 0);
            }

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

    // ==================================================
    // GET /api/relatorio/mensal?ano=YYYY&mes=MM
    // Retorna: totalVendas, receitaConfirmada, receitaPendente,
    //          ticketMedio, produtoMaisVendido, topProdutos, vendas
    // ==================================================
    svr.Get("/api/relatorio/mensal", [conn](const httplib::Request& req, httplib::Response& res) {
        string ano = req.get_param_value("ano");
        string mes = req.get_param_value("mes");

        if (ano.empty() || mes.empty()) {
            errResp(res, "Parametros 'ano' e 'mes' sao obrigatorios"); return;
        }

        // -- 1. Lista de vendas do mes --
        const char* p2[2] = {ano.c_str(), mes.c_str()};
        PGresult* rVendas = PQexecParams(conn,
            "SELECT p.id, c.nome AS cliente, "
            "TO_CHAR(p.data, 'DD/MM/YYYY') AS data, "
            "p.forma_pagamento::text, p.status_pagamento::text, p.total "
            "FROM pedidos p "
            "JOIN clientes c ON c.id = p.cliente_id "
            "WHERE EXTRACT(YEAR  FROM p.data) = $1::integer "
            "  AND EXTRACT(MONTH FROM p.data) = $2::integer "
            "ORDER BY p.data DESC, p.id DESC",
            2, nullptr, p2, nullptr, nullptr, 0);

        json vendas = json::array();
        int nVendas = PQntuples(rVendas);
        double recConf = 0, recPend = 0, totalGeral = 0;

        for (int i = 0; i < nVendas; i++) {
            string status = PQgetvalue(rVendas, i, 4);
            double total  = atof(PQgetvalue(rVendas, i, 5));
            if (status == "confirmado") recConf += total;
            if (status == "pendente")   recPend += total;
            totalGeral += total;

            vendas.push_back({
                {"id",              atoi(PQgetvalue(rVendas, i, 0))},
                {"cliente",         PQgetvalue(rVendas, i, 1)},
                {"data",            PQgetvalue(rVendas, i, 2)},
                {"formaPagamento",  PQgetvalue(rVendas, i, 3)},
                {"statusPagamento", status},
                {"total",           total}
            });
        }
        PQclear(rVendas);

        double ticketMedio = nVendas > 0 ? totalGeral / nVendas : 0;

        // -- 2. Top produtos do mes --
        PGresult* rTop = PQexecParams(conn,
            "SELECT i.nome, SUM(ip.quantidade) AS qtd, "
            "SUM(ip.quantidade * ip.preco_unitario) AS receita "
            "FROM itens_pedido ip "
            "JOIN instrumentos i ON i.id = ip.instrumento_id "
            "JOIN pedidos p ON p.id = ip.pedido_id "
            "WHERE EXTRACT(YEAR  FROM p.data) = $1::integer "
            "  AND EXTRACT(MONTH FROM p.data) = $2::integer "
            "GROUP BY i.nome "
            "ORDER BY qtd DESC "
            "LIMIT 10",
            2, nullptr, p2, nullptr, nullptr, 0);

        json topProdutos = json::array();
        int nTop = PQntuples(rTop);
        for (int i = 0; i < nTop; i++) {
            topProdutos.push_back({
                {"nome",       PQgetvalue(rTop, i, 0)},
                {"quantidade", atoi(PQgetvalue(rTop, i, 1))},
                {"receita",    atof(PQgetvalue(rTop, i, 2))}
            });
        }

        json prodMaisVendido = nullptr;
        if (nTop > 0) {
            prodMaisVendido = {
                {"nome",       PQgetvalue(rTop, 0, 0)},
                {"quantidade", atoi(PQgetvalue(rTop, 0, 1))}
            };
        }
        PQclear(rTop);

        json resposta = {
            {"totalVendas",        nVendas},
            {"receitaConfirmada",  recConf},
            {"receitaPendente",    recPend},
            {"ticketMedio",        ticketMedio},
            {"produtoMaisVendido", prodMaisVendido},
            {"topProdutos",        topProdutos},
            {"vendas",             vendas}
        };
        jsonResp(res, resposta);
    });

    cout << "Endpoints ativos em http://localhost:8080" << endl;
    svr.listen("0.0.0.0", 8080);
    PQfinish(conn);
    return 0;
}
