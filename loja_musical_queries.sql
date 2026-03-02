-- ===================== INSERIR INSTRUMENTOS =====================
INSERT INTO instrumentos (nome, tipo, marca, preco, quantidade) VALUES
('Stratocaster', 'guitarra', 'Fender', 7500.00, 5),
('Les Paul', 'guitarra', 'Gibson', 12000.00, 3),
('Violão Folk', 'violao', 'Yamaha', 1500.00, 10),
('Baixo Precision', 'baixo', 'Fender', 4000.00, 4),
('Violão Clássico', 'violao', 'Cordoba', 1800.00, 7);

-- ===================== INSERIR CLIENTES =====================
-- Atualizado para incluir a coluna e os valores de 'sexo'
INSERT INTO clientes (nome, cpf, telefone, email, sexo) VALUES
('Ana Souza', '123.456.789-00', '(11) 99999-1111', 'ana.souza@email.com', 'Feminino'),
('Bruno Lima', '987.654.321-00', '(21) 98888-2222', 'bruno.lima@email.com', 'Masculino'),
('Carla Mendes', '111.222.333-44', '(31) 97777-3333', 'carla.mendes@email.com', 'Feminino'),
('Diego Santos', '555.666.777-88', '(41) 96666-4444', 'diego.santos@email.com', 'Masculino'),
('Elisa Costa', '999.888.777-66', '(51) 95555-5555', 'elisa.costa@email.com', 'Feminino');

-- ===================== INSERIR VENDAS =====================
INSERT INTO vendas (cliente_id, instrumento_id, quantidade, valor_total) VALUES
(1, 1, 1, 7500.00),
(2, 3, 2, 3000.00),
(3, 2, 1, 12000.00),
(4, 4, 1, 4000.00),
(5, 5, 3, 5400.00);

-- ===================== CONSULTAS =====================
SELECT * FROM instrumentos;
SELECT * FROM clientes;
SELECT * FROM vendas;