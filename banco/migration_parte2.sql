-- ====================================================================
-- MIGRATION PARTE 2 — Executar no banco já existente
-- ====================================================================

\c loja_musical;

-- 1. ENUM forma_pgto
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'forma_pgto') THEN
        CREATE TYPE forma_pgto AS ENUM (
            'dinheiro', 'cartao_credito', 'cartao_debito',
            'pix', 'boleto', 'berries'
        );
    END IF;
END;
$$;

-- 2. ENUM status_pgto
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'status_pgto') THEN
        CREATE TYPE status_pgto AS ENUM (
            'pendente', 'confirmado', 'recusado'
        );
    END IF;
END;
$$;

-- 3. Converte forma_pagamento para ENUM
-- Precisa remover o DEFAULT antes de mudar o tipo,
-- depois reaplica o DEFAULT já como ENUM.
DO $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM information_schema.columns
        WHERE table_name = 'pedidos'
          AND column_name = 'forma_pagamento'
          AND data_type = 'character varying'
    ) THEN
        ALTER TABLE pedidos ALTER COLUMN forma_pagamento DROP DEFAULT;
        ALTER TABLE pedidos
            ALTER COLUMN forma_pagamento TYPE forma_pgto
            USING forma_pagamento::forma_pgto;
        ALTER TABLE pedidos ALTER COLUMN forma_pagamento SET DEFAULT 'dinheiro';
    END IF;
END;
$$;

-- 4. Adiciona status_pagamento se não existir
ALTER TABLE pedidos
    ADD COLUMN IF NOT EXISTS status_pagamento status_pgto NOT NULL DEFAULT 'pendente';

-- 5. Índice
CREATE INDEX IF NOT EXISTS idx_pedidos_status ON pedidos(status_pagamento);

-- 6. Remove versões antigas da procedure
-- Assinatura sem OUT (Postgres ignora OUT no DROP)
DROP PROCEDURE IF EXISTS efetuar_compra(integer, integer, integer[], integer[], forma_pgto);
DROP PROCEDURE IF EXISTS efetuar_compra(integer, integer, integer[], integer[], varchar);
DROP PROCEDURE IF EXISTS efetuar_compra(integer, integer, integer[], integer[]);

-- 7. Recria a procedure
-- REGRA: OUT nunca pode vir DEPOIS de um parâmetro com DEFAULT no Postgres.
-- Solução: OUT fica antes do parâmetro opcional com DEFAULT.
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
    IF NOT EXISTS (SELECT 1 FROM funcionarios WHERE id = p_funcionario_id) THEN
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

GRANT EXECUTE ON PROCEDURE efetuar_compra TO lojamusical_user;
