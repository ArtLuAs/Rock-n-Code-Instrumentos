-- ====================================================================
-- MIGRATION PARTE 3
-- Aplique APENAS se o banco foi criado antes desta atualização.
-- Se criou o banco do zero com loja_musical.sql atualizado, ignore.
-- ====================================================================

\c loja_musical;

-- 1. Torna funcionario_id opcional (NULL = pedido feito pelo próprio cliente)
ALTER TABLE pedidos
    ALTER COLUMN funcionario_id DROP NOT NULL;

-- 2. Adiciona coluna confirmado_por_id (quem confirmou/recusou o pagamento)
--    Só executa se a coluna ainda não existir
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM information_schema.columns
        WHERE table_name = 'pedidos' AND column_name = 'confirmado_por_id'
    ) THEN
        ALTER TABLE pedidos
            ADD COLUMN confirmado_por_id INTEGER REFERENCES funcionarios(id) ON DELETE SET NULL;

        CREATE INDEX idx_pedidos_confirmado_por ON pedidos(confirmado_por_id);
    END IF;
END;
$$;

-- 3. Atualiza a stored procedure para aceitar funcionario_id NULL
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
