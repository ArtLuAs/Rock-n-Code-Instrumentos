import { useState, useEffect } from 'react';
import { Table, Button, Modal, Form, Container, Row, Col, Alert } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const ProdutosList = () => {
  const { theme } = useOutletContext();
  const [produtos, setProdutos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  // --- Estados de Filtro e Ordenação ---
  const [categoriasFiltro, setCategoriasFiltro] = useState([]);
  const [somenteStock, setSomenteStock] = useState(false);
  const [sortBy, setSortBy] = useState('id-asc');
  
  const categoriasDisponiveis = ["Cordas", "Teclas", "Percussão", "Sopro", "Acessórios"];

  // --- Estados dos Modais ---
  const [showFormModal, setShowFormModal] = useState(false);
  const [isEditing, setIsEditing] = useState(false);
  const [currentProduto, setCurrentProduto] = useState({ id: null, nome: '', categoria: '', preco: '', quantidade: '' });
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [produtoToDelete, setProdutoToDelete] = useState(null);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const carregarProdutos = async () => {
    setIsLoading(true);
    try {
      const data = await api.get('/produtos');
      setProdutos(data);
    } catch (error) {
      mostrarAlerta('danger', 'Erro ao carregar os dados.');
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    carregarProdutos();
  }, []);

  const handleCloseForm = () => setShowFormModal(false);
  
  const handleShowNew = () => {
    setIsEditing(false);
    setCurrentProduto({ id: null, nome: '', categoria: '', preco: '', quantidade: '' });
    setShowFormModal(true);
  };
  
  const handleShowEdit = (produto) => {
    setIsEditing(true);
    setCurrentProduto(produto);
    setShowFormModal(true);
  };

  const handleCloseDelete = () => setShowDeleteModal(false);
  
  const handleShowDelete = (produto) => {
    setProdutoToDelete(produto);
    setShowDeleteModal(true);
  };

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCurrentProduto({ ...currentProduto, [name]: value });
  };

  const handleSave = async () => {
    try {
      if (isEditing) {
        await api.put(`/produtos/${currentProduto.id}`, currentProduto);
        mostrarAlerta('success', 'Registo atualizado com sucesso!');
      } else {
        await api.post('/produtos', currentProduto);
        mostrarAlerta('success', 'Novo registo criado com sucesso!');
      }
      handleCloseForm();
      carregarProdutos();
    } catch (error) {
      mostrarAlerta('danger', 'Ocorreu um erro ao salvar as alterações.');
    }
  };

  const handleDelete = async () => {
    try {
      await api.delete(`/produtos/${produtoToDelete.id}`);
      mostrarAlerta('success', 'Registo eliminado com sucesso!');
      handleCloseDelete();
      carregarProdutos();
    } catch (error) {
      mostrarAlerta('danger', 'Erro ao tentar eliminar o registo.');
    }
  };

  // --- Lógica de Checkbox de Categorias ---
  const toggleCategoria = (cat) => {
    setCategoriasFiltro(prev => 
      prev.includes(cat) ? prev.filter(c => c !== cat) : [...prev, cat]
    );
  };

  // --- Lógica de Derivação (Filtro + Ordenação) ---
  const produtosFiltrados = produtos
    .filter(p => {
      // Se não houver categorias marcadas, mostra todas. Senão, mostra apenas as marcadas.
      const matchCategoria = categoriasFiltro.length === 0 || categoriasFiltro.includes(p.categoria);
      // Se o switch de stock estiver ativo, exige quantidade > 0
      const matchStock = somenteStock ? parseInt(p.quantidade) > 0 : true;
      
      return matchCategoria && matchStock;
    })
    .sort((a, b) => {
      if (sortBy === 'nome-asc') return a.nome.localeCompare(b.nome);
      if (sortBy === 'nome-desc') return b.nome.localeCompare(a.nome);
      if (sortBy === 'preco-asc') return parseFloat(a.preco) - parseFloat(b.preco);
      if (sortBy === 'preco-desc') return parseFloat(b.preco) - parseFloat(a.preco);
      if (sortBy === 'id-desc') return parseInt(b.id) - parseInt(a.id);
      return parseInt(a.id) - parseInt(b.id); // id-asc (padrão)
    });

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="mb-3 align-items-center">
        <Col md={4}><h2>Catálogo de Produtos</h2></Col>
        <Col md={8} className="text-end">
          <Button variant="success" onClick={handleShowNew} disabled={isLoading}>
            + Adicionar
          </Button>
        </Col>
      </Row>

      {/* --- Toolbar de Filtros --- */}
      <div className="p-3 mb-4 rounded border" style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row>
          <Col lg={8}>
            <div className="mb-2 fw-bold">Filtrar por Categoria:</div>
            <div className="d-flex flex-wrap gap-3">
              {categoriasDisponiveis.map(cat => (
                <Form.Check 
                  key={cat} 
                  type="checkbox" 
                  label={cat} 
                  checked={categoriasFiltro.includes(cat)}
                  onChange={() => toggleCategoria(cat)}
                />
              ))}
            </div>
            <Form.Check 
              type="switch" 
              id="stock-switch" 
              label="Mostrar apenas produtos em stock" 
              className="mt-3 text-primary fw-bold"
              checked={somenteStock} 
              onChange={(e) => setSomenteStock(e.target.checked)}
            />
          </Col>
          <Col lg={4} className="mt-3 mt-lg-0 d-flex align-items-end">
            <Form.Group className="w-100">
              <Form.Label className="fw-bold">Ordenar por:</Form.Label>
              <Form.Select value={sortBy} onChange={(e) => setSortBy(e.target.value)}>
                <option value="id-asc">ID (Crescente)</option>
                <option value="id-desc">ID (Decrescente)</option>
                <option value="nome-asc">Ordem Alfabética (A-Z)</option>
                <option value="nome-desc">Ordem Alfabética (Z-A)</option>
                <option value="preco-desc">Preço (Maior para Menor)</option>
                <option value="preco-asc">Preço (Menor para Maior)</option>
              </Form.Select>
            </Form.Group>
          </Col>
        </Row>
      </div>

      <Table variant={theme} striped bordered hover responsive>
        <thead>
          <tr><th>ID</th><th>Nome</th><th>Categoria</th><th>Preço</th><th>Qtd</th><th>Ações</th></tr>
        </thead>
        <tbody>
          {isLoading ? (
            <tr><td colSpan="6" className="text-center">A processar...</td></tr>
          ) : produtosFiltrados.length === 0 ? (
            <tr><td colSpan="6" className="text-center">Nenhum registo encontrado com estes filtros.</td></tr>
          ) : (
            produtosFiltrados.map((produto) => (
              <tr key={produto.id}>
                <td>{produto.id}</td>
                <td>{produto.nome}</td>
                <td>{produto.categoria}</td>
                <td>R$ {Number(produto.preco).toFixed(2)}</td>
                <td>{produto.quantidade}</td>
                <td>
                  <Button variant="primary" size="sm" className="me-2" onClick={() => handleShowEdit(produto)}>Editar</Button>
                  <Button variant="danger" size="sm" onClick={() => handleShowDelete(produto)}>Eliminar</Button>
                </td>
              </tr>
            ))
          )}
        </tbody>
      </Table>

      {/* --- Modais --- */}
      <Modal show={showFormModal} onHide={handleCloseForm}>
        <Modal.Header closeButton><Modal.Title>{isEditing ? 'Editar' : 'Criar Novo'}</Modal.Title></Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Nome</Form.Label>
              <Form.Control type="text" name="nome" value={currentProduto.nome} onChange={handleChange} />
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Categoria</Form.Label>
              <Form.Select name="categoria" value={currentProduto.categoria} onChange={handleChange}>
                <option value="">Selecione...</option>
                {categoriasDisponiveis.map(c => <option key={c} value={c}>{c}</option>)}
              </Form.Select>
            </Form.Group>
            <Row>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Preço (R$)</Form.Label>
                  <Form.Control type="number" step="0.01" name="preco" value={currentProduto.preco} onChange={handleChange} />
                </Form.Group>
              </Col>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Quantidade</Form.Label>
                  <Form.Control type="number" name="quantidade" value={currentProduto.quantidade} onChange={handleChange} />
                </Form.Group>
              </Col>
            </Row>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseForm}>Cancelar</Button>
          <Button variant="primary" onClick={handleSave}>Salvar</Button>
        </Modal.Footer>
      </Modal>

      <Modal show={showDeleteModal} onHide={handleCloseDelete}>
        <Modal.Header closeButton><Modal.Title>Atenção</Modal.Title></Modal.Header>
        <Modal.Body>Confirma a exclusão de <strong>{produtoToDelete?.nome}</strong>?</Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseDelete}>Cancelar</Button>
          <Button variant="danger" onClick={handleDelete}>Confirmar</Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
};

export default ProdutosList;