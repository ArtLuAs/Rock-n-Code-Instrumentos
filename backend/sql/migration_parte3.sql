-- ====================================================================
-- MIGRATION PARTE 3
-- Recria a view com LEFT JOIN para incluir pedidos sem vendedor,
-- e adiciona GRANT SELECT na view para o usuário da aplicação.
--
-- Execute como superusuário (postgres):
--   psql -U postgres -d loja_musical -f backend/sql/migration_parte3.sql
-- ====================================================================

-- DROP obrigatório: CREATE OR REPLACE não aceita mudança de tipo de coluna
DROP VIEW IF EXISTS vw_vendas_por_vendedor_mes;

CREATE VIEW vw_vendas_por_vendedor_mes AS
SELECT
    f.id                                          AS funcionario_id,
    COALESCE(f.nome, '(site / sem vendedor)')     AS vendedor,
    DATE_TRUNC('month', p.data)::DATE             AS mes,
    COUNT(p.id)                                   AS total_pedidos,
    SUM(p.total)                                  AS total_vendido
FROM pedidos p
LEFT JOIN funcionarios f ON f.id = p.funcionario_id
GROUP BY f.id, f.nome, DATE_TRUNC('month', p.data)
ORDER BY mes DESC, total_vendido DESC;

-- Garante permissão de leitura para o usuário da aplicação
GRANT SELECT ON vw_vendas_por_vendedor_mes TO lojamusical_user;
