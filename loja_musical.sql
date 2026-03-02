-- Criar banco
CREATE DATABASE loja_musical;

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

-- ===================== TABELA CLIENTES =====================
CREATE TABLE clientes (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(150) NOT NULL,
    cpf VARCHAR(14) UNIQUE NOT NULL,
    telefone VARCHAR(20),
    email VARCHAR(150),
    sexo VARCHAR(15)
);

-- ===================== TABELA VENDAS =====================
CREATE TABLE vendas (
    id SERIAL PRIMARY KEY,
    cliente_id INTEGER NOT NULL,
    instrumento_id INTEGER NOT NULL,
    quantidade INTEGER NOT NULL CHECK (quantidade > 0),
    valor_total NUMERIC(10,2) NOT NULL,
    data DATE NOT NULL DEFAULT CURRENT_DATE,

    CONSTRAINT fk_cliente
        FOREIGN KEY (cliente_id)
        REFERENCES clientes(id)
        ON DELETE RESTRICT,

    CONSTRAINT fk_instrumento
        FOREIGN KEY (instrumento_id)
        REFERENCES instrumentos(id)
        ON DELETE RESTRICT
);