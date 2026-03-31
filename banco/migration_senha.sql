-- Execute este script no seu banco PostgreSQL antes de rodar o backend

-- Adiciona coluna senha na tabela clientes (texto simples, sem hash por ora)
ALTER TABLE clientes ADD COLUMN IF NOT EXISTS senha TEXT;

-- Adiciona coluna senha na tabela funcionarios
ALTER TABLE funcionarios ADD COLUMN IF NOT EXISTS senha TEXT;

-- Opcional: define uma senha padrão para funcionários já existentes
-- UPDATE funcionarios SET senha = '1234' WHERE senha IS NULL;
