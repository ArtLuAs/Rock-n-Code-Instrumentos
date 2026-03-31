import { useState, useEffect } from 'react';
import { Table, Button, Modal, Form, Container, Row, Col, Alert, Badge } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const TIPOS = ['guitarra', 'violao', 'baixo'];

const PRODUTO_VAZIO = {
  id: null, nome: '', tipo: '', marca: '', preco: '',
  quantidade: '', categoria: '', fabricadoEmMari: false,
};

const ProdutosList = () => {
  const outletCtx = useOutletContext();
  const theme = outletCtx?.theme || 'light';

  const userRole = localStorage.getItem('auth-role');
  const isFuncionario = userRole === 'funcionario';

  const [produtos, setProdutos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  // Filtros
  const [busca, setBusca] = useState('');
  const [precoMin, setPrecoMin] = useState('');
  const [precoMax, setPrecoMax] = useState('');
  const [tiposFiltro, setTiposFiltro] = useState([]);
  const [somenteMari, setSomenteMari] = useState(false);
  const [somenteStock, setSomenteStock] = useState(false);
  const [estoqueBaixo, setEstoqueBaixo] = useState(false);
  const [sortBy, setSortBy] = useState('id-asc');

  // Modais (só para funcionário)
  const [showFormModal, setShowFormModal] = useState(false);
  const [isEditing, setIsEditing] = useState(false);
  const [currentProduto, setCurrentProduto] = useState(PRODUTO_VAZIO);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [produtoToDelete, setProdutoToDelete] = useState(null);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const carregarProdutos = async () => {
    setIsLoading(true);
    try { setProdutos(await api.get('/produtos')); }
    catch { mostrarAlerta('danger', 'Erro ao carregar os dados.'); }
    finally { setIsLoading(false); }
  };

  useEffect(() => { carregarProdutos(); }, []);

  const handleCloseForm = () => setShowFormModal(false);
  const handleShowNew = () => { setIsEditing(false); setCurrentProduto(PRODUTO_VAZIO); setShowFormModal(true); };
  const handleShowEdit = (p) => { setIsEditing(true); setCurrentProduto({ ...PRODUTO_VAZIO, ...p }); setShowFormModal(true); };
  const handleCloseDelete = () => setShowDeleteModal(false);
  const handleShowDelete = (p) => { setProdutoToDelete(p); setShowDeleteModal(true); };

  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setCurrentProduto(prev => ({ ...prev, [name]: type === 'checkbox' ? checked : value }));
  };

  const handleSave = async () => {
    if (!currentProduto.nome || !currentProduto.tipo || !currentProduto.marca) {
      mostrarAlerta('warning', 'Nome, Tipo e Marca são obrigatórios.'); return;
    }
    try {
      if (isEditing) await api.put(`/produtos/${currentProduto.id}`, currentProduto);
      else await api.post('/produtos', currentProduto);
      mostrarAlerta('success', 'Produto salvo com sucesso!');
      handleCloseForm(); carregarProdutos();
    } catch (err) { mostrarAlerta('danger', `Erro ao salvar: ${err.message}`); }
  };

  const handleDelete = async () => {
    try {
      await api.delete(`/produtos/${produtoToDelete.id}`);
      mostrarAlerta('success', 'Produto eliminado!');
      handleCloseDelete(); carregarProdutos();
    } catch (err) { mostrarAlerta('danger', `Erro: ${err.message}`); }
  };

  const toggleTipo = (tipo) =>
    setTiposFiltro(prev => prev.includes(tipo) ? prev.filter(t => t !== tipo) : [...prev, tipo]);

  const produtosFiltrados = produtos
    .filter(p => {
      const matchNome  = busca.trim() === '' || p.nome.toLowerCase().includes(busca.toLowerCase());
      const matchTipo  = tiposFiltro.length === 0 || tiposFiltro.includes(p.tipo);
      const matchMin   = precoMin === '' || parseFloat(p.preco) >= parseFloat(precoMin);
      const matchMax   = precoMax === '' || parseFloat(p.preco) <= parseFloat(precoMax);
      const matchMari  = !somenteMari || p.fabricadoEmMari;
      const matchStock = somenteStock ? parseInt(p.quantidade) > 0 : true;
      const matchBaixo = !estoqueBaixo || parseInt(p.quantidade) < 5;
      return matchNome && matchTipo && matchMin && matchMax && matchMari && matchStock && matchBaixo;
    })
    .sort((a, b) => {
      if (sortBy === 'nome-asc')   return a.nome.localeCompare(b.nome);
      if (sortBy === 'nome-desc')  return b.nome.localeCompare(a.nome);
      if (sortBy === 'preco-asc')  return parseFloat(a.preco) - parseFloat(b.preco);
      if (sortBy === 'preco-desc') return parseFloat(b.preco) - parseFloat(a.preco);
      if (sortBy === 'id-desc')    return parseInt(b.id) - parseInt(a.id);
      return parseInt(a.id) - parseInt(b.id);
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
          {isFuncionario && (
            <Button variant="success" onClick={handleShowNew} disabled={isLoading}>+ Adicionar</Button>
          )}
        </Col>
      </Row>

      {/* Painel de filtros */}
      <div className="p-3 mb-4 rounded border" style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row className="g-3">

          {/* Busca por nome */}
          <Col md={4}>
            <Form.Label className="fw-bold">Buscar por nome:</Form.Label>
            <Form.Control
              type="text" placeholder="Ex: Fender..."
              value={busca} onChange={e => setBusca(e.target.value)}
            />
          </Col>

          {/* Faixa de preço */}
          <Col md={4}>
            <Form.Label className="fw-bold">Faixa de preço (R$):</Form.Label>
            <Row>
              <Col>
                <Form.Control type="number" placeholder="Mín" min="0" step="0.01"
                  value={precoMin} onChange={e => setPrecoMin(e.target.value)} />
              </Col>
              <Col xs="auto" className="d-flex align-items-center px-0">—</Col>
              <Col>
                <Form.Control type="number" placeholder="Máx" min="0" step="0.01"
                  value={precoMax} onChange={e => setPrecoMax(e.target.value)} />
              </Col>
            </Row>
          </Col>

          {/* Ordenar */}
          <Col md={4}>
            <Form.Label className="fw-bold">Ordenar por:</Form.Label>
            <Form.Select value={sortBy} onChange={e => setSortBy(e.target.value)}>
              <option value="id-asc">ID (Crescente)</option>
              <option value="id-desc">ID (Decrescente)</option>
              <option value="nome-asc">Nome (A-Z)</option>
              <option value="nome-desc">Nome (Z-A)</option>
              <option value="preco-asc">Preço (Menor)</option>
              <option value="preco-desc">Preço (Maior)</option>
            </Form.Select>
          </Col>

          {/* Filtros de tipo */}
          <Col md={6}>
            <div className="fw-bold mb-2">Filtrar por Tipo:</div>
            <div className="d-flex flex-wrap gap-3">
              {TIPOS.map(tipo => (
                <Form.Check key={tipo} type="checkbox" label={tipo}
                  checked={tiposFiltro.includes(tipo)}
                  onChange={() => toggleTipo(tipo)} />
              ))}
            </div>
          </Col>

          {/* Switches */}
          <Col md={6} className="d-flex flex-column justify-content-end gap-2">
            <Form.Check type="switch" id="mari-filter" label="🎸 Fabricados em Mari"
              checked={somenteMari} onChange={e => setSomenteMari(e.target.checked)} />
            <Form.Check type="switch" id="stock-switch" label="Mostrar apenas em stock"
              className="text-primary"
              checked={somenteStock} onChange={e => setSomenteStock(e.target.checked)} />
            {isFuncionario && (
              <Form.Check type="switch" id="baixo-switch"
                label="⚠️ Estoque baixo (menos de 5 unidades)"
                className="text-danger fw-bold"
                checked={estoqueBaixo} onChange={e => setEstoqueBaixo(e.target.checked)} />
            )}
          </Col>
        </Row>
      </div>

      <Table variant={theme} striped bordered hover responsive>
        <thead>
          <tr>
            <th>ID</th><th>Nome</th><th>Tipo</th><th>Marca</th>
            <th>Categoria</th><th>Preço</th><th>Qtd</th><th>Mari?</th>
            {isFuncionario && <th>Ações</th>}
          </tr>
        </thead>
        <tbody>
          {isLoading ? (
            <tr><td colSpan={isFuncionario ? 9 : 8} className="text-center">A carregar...</td></tr>
          ) : produtosFiltrados.length === 0 ? (
            <tr><td colSpan={isFuncionario ? 9 : 8} className="text-center">Nenhum produto encontrado.</td></tr>
          ) : (
            produtosFiltrados.map(p => (
              <tr key={p.id}>
                <td>{p.id}</td>
                <td>{p.nome}</td>
                <td>{p.tipo}</td>
                <td>{p.marca}</td>
                <td>{p.categoria}</td>
                <td>R$ {Number(p.preco).toFixed(2)}</td>
                <td>
                  {parseInt(p.quantidade) < 5
                    ? <span className="text-danger fw-bold">{p.quantidade} ⚠️</span>
                    : p.quantidade}
                </td>
                <td>{p.fabricadoEmMari
                  ? <Badge bg="success">Sim</Badge>
                  : <Badge bg="secondary">Não</Badge>}
                </td>
                {isFuncionario && (
                  <td>
                    <Button variant="primary" size="sm" className="me-2" onClick={() => handleShowEdit(p)}>Editar</Button>
                    <Button variant="danger" size="sm" onClick={() => handleShowDelete(p)}>Eliminar</Button>
                  </td>
                )}
              </tr>
            ))
          )}
        </tbody>
      </Table>

      {/* Modal Formulário (só funcionário) */}
      <Modal show={showFormModal} onHide={handleCloseForm}>
        <Modal.Header closeButton>
          <Modal.Title>{isEditing ? 'Editar Produto' : 'Novo Produto'}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Nome *</Form.Label>
              <Form.Control type="text" name="nome" value={currentProduto.nome} onChange={handleChange} />
            </Form.Group>
            <Row>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Tipo *</Form.Label>
                  <Form.Select name="tipo" value={currentProduto.tipo} onChange={handleChange}>
                    <option value="">Selecione...</option>
                    {TIPOS.map(t => <option key={t} value={t}>{t}</option>)}
                  </Form.Select>
                </Form.Group>
              </Col>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Marca *</Form.Label>
                  <Form.Control type="text" name="marca" value={currentProduto.marca} onChange={handleChange} />
                </Form.Group>
              </Col>
            </Row>
            <Form.Group className="mb-3">
              <Form.Label>Categoria</Form.Label>
              <Form.Control type="text" name="categoria" placeholder="ex: elétrico, acústico..." value={currentProduto.categoria} onChange={handleChange} />
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
            <Form.Check type="switch" id="mari-switch" label="🎸 Fabricado em Mari"
              name="fabricadoEmMari" checked={currentProduto.fabricadoEmMari} onChange={handleChange} />
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseForm}>Cancelar</Button>
          <Button variant="primary" onClick={handleSave}>Salvar</Button>
        </Modal.Footer>
      </Modal>

      {/* Modal Delete */}
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
