-- ====================================================================
-- SCRIPT DE CRIAÇÃO DO BANCO: LOJA MUSICAL
-- ====================================================================

-- 1. Criar usuário e senha (comente se o usuário já existir)
CREATE USER lojamusical_user WITH PASSWORD 'SenhaSegura123';

-- 2. Criar banco de dados (comente se o banco já existir)
CREATE DATABASE loja_musical;

-- 3. Dar permissões ao usuário no banco
GRANT ALL PRIVILEGES ON DATABASE loja_musical TO lojamusical_user;

-- 4. Conectar no banco (comando específico do psql)
\c loja_musical;

-- ===================== TABELA: INSTRUMENTOS =====================
CREATE TABLE instrumentos (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    tipo VARCHAR(50) NOT NULL CHECK (tipo IN ('guitarra', 'violao', 'baixo')),
    marca VARCHAR(100) NOT NULL,
    preco NUMERIC(10,2) NOT NULL CHECK (preco >= 0),
    quantidade INTEGER NOT NULL CHECK (quantidade >= 0)
);

-- ===================== TABELA: CLIENTES =====================
CREATE TABLE clientes (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    cpf VARCHAR(14) NOT NULL UNIQUE,
    telefone VARCHAR(20),
    email VARCHAR(100),
    sexo VARCHAR(20) CHECK (sexo IN ('Masculino', 'Feminino', 'Outro'))
);

-- ===================== TABELA: VENDAS =====================
CREATE TABLE vendas (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER NOT NULL REFERENCES clientes(id),
    valor_total NUMERIC(10,2) NOT NULL DEFAULT 0.00,
    data DATE NOT NULL
);

-- ===================== TABELA: ITENS DA VENDA =====================
CREATE TABLE itens_venda (
    id SERIAL PRIMARY KEY,
    venda_id INTEGER NOT NULL REFERENCES vendas(id) ON DELETE CASCADE,
    instrumento_id INTEGER NOT NULL REFERENCES instrumentos(id),
    quantidade INTEGER NOT NULL CHECK (quantidade > 0),
    preco_unitario NUMERIC(10,2) NOT NULL CHECK (preco_unitario >= 0)
);

-- ===================== PERMISSÕES =====================

-- Dar permissões no schema public
GRANT ALL ON SCHEMA public TO lojamusical_user;

-- Garantir permissão total nas tabelas para o usuário
GRANT ALL PRIVILEGES ON TABLE instrumentos TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE clientes TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE vendas TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE itens_venda TO lojamusical_user;

-- Garantir permissão nas sequências (necessário para o SERIAL do ID funcionar nos INSERTS)
GRANT USAGE, SELECT ON SEQUENCE instrumentos_id_seq TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE clientes_id_seq TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE vendas_id_seq TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE itens_venda_id_seq TO lojamusical_user;