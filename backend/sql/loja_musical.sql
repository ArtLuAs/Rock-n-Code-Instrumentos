-- ====================================================================
-- SCRIPT DE CRIAÇÃO DO BANCO: LOJA MUSICAL
-- ====================================================================

CREATE USER lojamusical_user WITH PASSWORD 'SenhaSegura123';
CREATE DATABASE loja_musical;
GRANT ALL PRIVILEGES ON DATABASE loja_musical TO lojamusical_user;
\c loja_musical;

-- ===================== ENUMS =====================
CREATE TYPE forma_pgto AS ENUM (
    'dinheiro',
    'cartao_credito',
    'cartao_debito',
    'pix',
    'boleto',
    'berries'
);

CREATE TYPE status_pgto AS ENUM (
    'pendente',
    'confirmado',
    'recusado'
);

-- ===================== INSTRUMENTOS =====================
CREATE TABLE instrumentos (
    id                SERIAL        PRIMARY KEY,
    nome              VARCHAR(100)  NOT NULL,
    tipo              VARCHAR(50)   NOT NULL CHECK (tipo IN ('guitarra', 'violao', 'baixo')),
    marca             VARCHAR(100)  NOT NULL,
    preco             NUMERIC(10,2) NOT NULL CHECK (preco >= 0),
    quantidade        INTEGER       NOT NULL CHECK (quantidade >= 0),
    categoria         VARCHAR(100)  NOT NULL DEFAULT 'outros',
    fabricado_em_mari BOOLEAN       NOT NULL DEFAULT FALSE
);

-- ===================== CLIENTES =====================
CREATE TABLE clientes (
    id                SERIAL       PRIMARY KEY,
    nome              VARCHAR(100) NOT NULL,
    cpf               VARCHAR(14)  NOT NULL UNIQUE,
    telefone          VARCHAR(20),
    email             VARCHAR(100),
    sexo              VARCHAR(20)  CHECK (sexo IN ('Masculino', 'Feminino', 'Outro')),
    torce_flamengo    BOOLEAN      NOT NULL DEFAULT FALSE,
    assiste_one_piece BOOLEAN      NOT NULL DEFAULT FALSE,
    cidade            VARCHAR(100),
    senha             VARCHAR(100)
);

-- ===================== FUNCIONARIOS =====================
CREATE TABLE funcionarios (
    id       SERIAL       PRIMARY KEY,
    nome     VARCHAR(100) NOT NULL,
    cpf      VARCHAR(14)  NOT NULL UNIQUE,
    telefone VARCHAR(20),
    email    VARCHAR(100),
    cargo    VARCHAR(100) NOT NULL DEFAULT 'vendedor',
    senha    VARCHAR(100)
);

-- ===================== PEDIDOS =====================
-- funcionario_id: NULL quando o cliente faz o pedido pelo site;
--                 preenchido quando um funcionário registra ou confirma/recusa o pagamento.
CREATE TABLE pedidos (
    id               SERIAL        PRIMARY KEY,
    cliente_id       INTEGER       NOT NULL REFERENCES clientes(id)     ON DELETE RESTRICT,
    funcionario_id   INTEGER                REFERENCES funcionarios(id) ON DELETE RESTRICT,
    data             DATE          NOT NULL DEFAULT CURRENT_DATE,
    forma_pagamento  forma_pgto    NOT NULL DEFAULT 'dinheiro',
    status_pagamento status_pgto   NOT NULL DEFAULT 'pendente',
    desconto         NUMERIC(5,2)  NOT NULL DEFAULT 0.00 CHECK (desconto >= 0 AND desconto <= 100),
    total            NUMERIC(10,2) NOT NULL DEFAULT 0.00 CHECK (total >= 0)
);

-- ===================== ITENS DO PEDIDO =====================
CREATE TABLE itens_pedido (
    id             SERIAL        PRIMARY KEY,
    pedido_id      INTEGER       NOT NULL REFERENCES pedidos(id)      ON DELETE CASCADE,
    instrumento_id INTEGER       NOT NULL REFERENCES instrumentos(id) ON DELETE RESTRICT,
    quantidade     INTEGER       NOT NULL CHECK (quantidade > 0),
    preco_unitario NUMERIC(10,2) NOT NULL CHECK (preco_unitario >= 0)
);

-- ===================== ÍNDICES =====================
CREATE INDEX idx_instrumentos_nome      ON instrumentos(nome);
CREATE INDEX idx_instrumentos_categoria ON instrumentos(categoria);
CREATE INDEX idx_instrumentos_mari      ON instrumentos(fabricado_em_mari);
CREATE INDEX idx_clientes_nome          ON clientes(nome);
CREATE INDEX idx_clientes_cidade        ON clientes(cidade);
CREATE INDEX idx_funcionarios_nome      ON funcionarios(nome);
CREATE INDEX idx_pedidos_cliente        ON pedidos(cliente_id);
CREATE INDEX idx_pedidos_funcionario    ON pedidos(funcionario_id);
CREATE INDEX idx_pedidos_data           ON pedidos(data);
CREATE INDEX idx_pedidos_status         ON pedidos(status_pagamento);
CREATE INDEX idx_itens_pedido_pedido    ON itens_pedido(pedido_id);

-- ===================== VIEW: VENDAS POR VENDEDOR/MÊS =====================
CREATE VIEW vw_vendas_por_vendedor_mes AS
SELECT
    f.id                              AS funcionario_id,
    f.nome                            AS vendedor,
    DATE_TRUNC('month', p.data)::DATE AS mes,
    COUNT(p.id)                       AS total_pedidos,
    SUM(p.total)                      AS total_vendido
FROM pedidos p
JOIN funcionarios f ON f.id = p.funcionario_id
GROUP BY f.id, f.nome, DATE_TRUNC('month', p.data)
ORDER BY mes DESC, total_vendido DESC;

-- ===================== STORED PROCEDURE: EFETUAR COMPRA =====================
-- p_funcionario_id pode ser NULL (pedido feito pelo próprio cliente no site).
CREATE OR REPLACE PROCEDURE efetuar_compra(
    p_cliente_id      INTEGER,
    p_funcionario_id  INTEGER,
    p_instrumentos    INTEGER[],
    p_quantidades     INTEGER[],
    OUT p_pedido_id   INTEGER,
    p_forma_pagamento VARCHAR DEFAULT 'dinheiro'
)
LANGUAGE plpgsql
AS $$
DECLARE
    v_desconto   NUMERIC(5,2)  := 0.00;
    v_total      NUMERIC(10,2) := 0.00;
    v_preco      NUMERIC(10,2);
    v_estoque    INTEGER;
    v_subtotal   NUMERIC(10,2);
    i            INTEGER;
    v_torce_flam BOOLEAN;
    v_one_piece  BOOLEAN;
    v_cidade     VARCHAR(100);
BEGIN
    IF NOT EXISTS (SELECT 1 FROM clientes WHERE id = p_cliente_id) THEN
        RAISE EXCEPTION 'Cliente % não encontrado.', p_cliente_id;
    END IF;
    -- Só valida funcionário se um foi informado
    IF p_funcionario_id IS NOT NULL AND
       NOT EXISTS (SELECT 1 FROM funcionarios WHERE id = p_funcionario_id) THEN
        RAISE EXCEPTION 'Funcionário % não encontrado.', p_funcionario_id;
    END IF;

    SELECT torce_flamengo, assiste_one_piece, LOWER(COALESCE(cidade, ''))
    INTO v_torce_flam, v_one_piece, v_cidade
    FROM clientes WHERE id = p_cliente_id;

    IF v_torce_flam OR v_one_piece OR v_cidade = 'sousa' THEN
        v_desconto := 10.00;
    END IF;

    FOR i IN 1 .. array_length(p_instrumentos, 1) LOOP
        SELECT preco, quantidade
        INTO v_preco, v_estoque
        FROM instrumentos WHERE id = p_instrumentos[i];

        IF NOT FOUND THEN
            RAISE EXCEPTION 'Instrumento % não encontrado.', p_instrumentos[i];
        END IF;
        IF v_estoque < p_quantidades[i] THEN
            RAISE EXCEPTION 'Estoque insuficiente para instrumento % (disponível: %).', p_instrumentos[i], v_estoque;
        END IF;

        v_subtotal := v_preco * p_quantidades[i];
        v_total    := v_total + v_subtotal;
    END LOOP;

    v_total := v_total * (1.0 - v_desconto / 100.0);

    INSERT INTO pedidos (cliente_id, funcionario_id, forma_pagamento, desconto, total, status_pagamento)
    VALUES (p_cliente_id, p_funcionario_id, p_forma_pagamento::forma_pgto, v_desconto, v_total, 'pendente')
    RETURNING id INTO p_pedido_id;

    FOR i IN 1 .. array_length(p_instrumentos, 1) LOOP
        SELECT preco INTO v_preco FROM instrumentos WHERE id = p_instrumentos[i];
        INSERT INTO itens_pedido (pedido_id, instrumento_id, quantidade, preco_unitario)
        VALUES (p_pedido_id, p_instrumentos[i], p_quantidades[i], v_preco);
        UPDATE instrumentos SET quantidade = quantidade - p_quantidades[i] WHERE id = p_instrumentos[i];
    END LOOP;
END;
$$;

-- ===================== PERMISSÕES =====================
GRANT ALL ON SCHEMA public TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE instrumentos  TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE clientes      TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE funcionarios  TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE pedidos       TO lojamusical_user;
GRANT ALL PRIVILEGES ON TABLE itens_pedido  TO lojamusical_user;
GRANT EXECUTE ON PROCEDURE efetuar_compra   TO lojamusical_user;

GRANT USAGE, SELECT ON SEQUENCE instrumentos_id_seq TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE clientes_id_seq     TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE funcionarios_id_seq TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE pedidos_id_seq      TO lojamusical_user;
GRANT USAGE, SELECT ON SEQUENCE itens_pedido_id_seq TO lojamusical_user;
