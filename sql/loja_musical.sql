-- Criar usuário e senha
CREATE USER lojamusical_user WITH PASSWORD 'SenhaSegura123';

-- Criar banco
CREATE DATABASE loja_musical;

-- Dar permissões ao usuário no banco
GRANT ALL PRIVILEGES ON DATABASE loja_musical TO lojamusical_user;

-- Conectar no banco
\c loja_musical;

-- ===================== TABELA INSTRUMENTOS =====================
CREATE TABLE instrumentos (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    tipo VARCHAR(50) NOT NULL CHECK (tipo IN ('guitarra', 'violao', 'baixo')),
    marca VARCHAR(100) NOT NULL,
    preco NUMERIC(10,2) NOT NULL CHECK (preco >= 0),
    quantidade INTEGER NOT NULL CHECK (quantidade >= 0)
);

-- Dar permissões no schema public
GRANT ALL ON SCHEMA public TO lojamusical_user;

-- Garantir permissão total na tabela para o usuário
GRANT ALL PRIVILEGES ON TABLE instrumentos TO lojamusical_user;

-- Garantir permissão na sequência (necessário para o SERIAL do ID funcionar nos INSERTS)
GRANT USAGE, SELECT ON SEQUENCE instrumentos_id_seq TO lojamusical_user;