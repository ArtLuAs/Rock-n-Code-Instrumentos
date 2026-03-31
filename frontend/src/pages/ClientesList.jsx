import { useState, useEffect } from 'react';
import { Table, Button, Modal, Form, Container, Row, Col, Alert, InputGroup, Badge } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const SEXOS = ['Masculino', 'Feminino', 'Outro'];

const CLIENTE_VAZIO = {
  id: null,
  nome: '',
  cpf: '',
  telefone: '',
  email: '',
  sexo: '',
  cidade: '',
  torceFlamengo: false,
  assisteOnePiece: false,
};

const ClientesList = () => {
  const { theme } = useOutletContext();
  const [clientes, setClientes] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const [searchTerm, setSearchTerm] = useState('');
  const [sortBy, setSortBy] = useState('id-asc');

  const [showFormModal, setShowFormModal] = useState(false);
  const [isEditing, setIsEditing] = useState(false);
  const [currentCliente, setCurrentCliente] = useState(CLIENTE_VAZIO);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [clienteToDelete, setClienteToDelete] = useState(null);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const carregarClientes = async () => {
    setIsLoading(true);
    try {
      setClientes(await api.get('/clientes'));
    } catch {
      mostrarAlerta('danger', 'Erro ao carregar os clientes.');
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => { carregarClientes(); }, []);

  const handleCloseForm = () => setShowFormModal(false);
  const handleShowNew = () => {
    setIsEditing(false);
    setCurrentCliente(CLIENTE_VAZIO);
    setShowFormModal(true);
  };
  const handleShowEdit = (cliente) => {
    setIsEditing(true);
    setCurrentCliente({ ...CLIENTE_VAZIO, ...cliente });
    setShowFormModal(true);
  };
  const handleCloseDelete = () => setShowDeleteModal(false);
  const handleShowDelete = (cliente) => { setClienteToDelete(cliente); setShowDeleteModal(true); };

  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setCurrentCliente(prev => ({ ...prev, [name]: type === 'checkbox' ? checked : value }));
  };

  const handleSave = async () => {
    if (!currentCliente.nome || !currentCliente.cpf) {
      mostrarAlerta('warning', 'Nome e CPF são obrigatórios.');
      return;
    }
    try {
      if (isEditing) await api.put(`/clientes/${currentCliente.id}`, currentCliente);
      else await api.post('/clientes', currentCliente);
      mostrarAlerta('success', 'Cliente salvo com sucesso!');
      handleCloseForm();
      carregarClientes();
    } catch (err) {
      mostrarAlerta('danger', `Erro ao salvar: ${err.message}`);
    }
  };

  const handleDelete = async () => {
    try {
      await api.delete(`/clientes/${clienteToDelete.id}`);
      mostrarAlerta('success', 'Cliente eliminado com sucesso!');
      handleCloseDelete();
      carregarClientes();
    } catch (err) {
      mostrarAlerta('danger', `Erro: ${err.message}`);
    }
  };

  const clientesFiltrados = clientes
    .filter(c =>
      c.nome.toLowerCase().includes(searchTerm.toLowerCase()) ||
      c.cpf.includes(searchTerm) ||
      (c.cidade || '').toLowerCase().includes(searchTerm.toLowerCase())
    )
    .sort((a, b) => {
      if (sortBy === 'nome-asc')  return a.nome.localeCompare(b.nome);
      if (sortBy === 'nome-desc') return b.nome.localeCompare(a.nome);
      if (sortBy === 'id-desc')   return b.id - a.id;
      return a.id - b.id;
    });

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="mb-3 align-items-center">
        <Col md={4}><h2>Gestão de Clientes</h2></Col>
        <Col md={8} className="text-end">
          <Button variant="success" onClick={handleShowNew} disabled={isLoading}>+ Novo Cliente</Button>
        </Col>
      </Row>

      <div className="p-3 mb-4 rounded border" style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row>
          <Col md={6}>
            <Form.Group>
              <Form.Label className="fw-bold">Buscar Cliente:</Form.Label>
              <InputGroup>
                <InputGroup.Text>🔍</InputGroup.Text>
                <Form.Control
                  placeholder="Nome, CPF ou cidade..."
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                />
              </InputGroup>
            </Form.Group>
          </Col>
          <Col md={6} className="mt-3 mt-md-0">
            <Form.Group>
              <Form.Label className="fw-bold">Ordenar por:</Form.Label>
              <Form.Select value={sortBy} onChange={(e) => setSortBy(e.target.value)}>
                <option value="id-asc">ID (Crescente)</option>
                <option value="id-desc">ID (Decrescente)</option>
                <option value="nome-asc">Ordem Alfabética (A-Z)</option>
                <option value="nome-desc">Ordem Alfabética (Z-A)</option>
              </Form.Select>
            </Form.Group>
          </Col>
        </Row>
      </div>

      <Table variant={theme} striped bordered hover responsive>
        <thead>
          <tr>
            <th>ID</th><th>Nome</th><th>CPF</th><th>Telefone</th>
            <th>Cidade</th><th>Sexo</th><th>Flags</th><th>Ações</th>
          </tr>
        </thead>
        <tbody>
          {isLoading ? (
            <tr><td colSpan="8" className="text-center">A carregar...</td></tr>
          ) : clientesFiltrados.length === 0 ? (
            <tr><td colSpan="8" className="text-center">Nenhum cliente encontrado.</td></tr>
          ) : (
            clientesFiltrados.map(cliente => (
              <tr key={cliente.id}>
                <td>{cliente.id}</td>
                <td>{cliente.nome}</td>
                <td>{cliente.cpf}</td>
                <td>{cliente.telefone}</td>
                <td>{cliente.cidade}</td>
                <td>{cliente.sexo}</td>
                <td className="d-flex gap-1 flex-wrap">
                  {cliente.torceFlamengo   && <Badge bg="danger">Flamengo</Badge>}
                  {cliente.assisteOnePiece && <Badge bg="warning" text="dark">One Piece</Badge>}
                  {cliente.cidade?.toLowerCase() === 'sousa' && <Badge bg="info" text="dark">Sousa</Badge>}
                </td>
                <td>
                  <Button variant="primary" size="sm" className="me-2" onClick={() => handleShowEdit(cliente)}>Editar</Button>
                  <Button variant="danger" size="sm" onClick={() => handleShowDelete(cliente)}>Eliminar</Button>
                </td>
              </tr>
            ))
          )}
        </tbody>
      </Table>

      {/* Modal Criar/Editar */}
      <Modal show={showFormModal} onHide={handleCloseForm}>
        <Modal.Header closeButton>
          <Modal.Title>{isEditing ? 'Editar Cliente' : 'Novo Cliente'}</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Nome *</Form.Label>
              <Form.Control type="text" name="nome" value={currentCliente.nome} onChange={handleChange} />
            </Form.Group>
            <Row>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>CPF *</Form.Label>
                  <Form.Control type="text" name="cpf" placeholder="Somente números" value={currentCliente.cpf} onChange={handleChange} />
                </Form.Group>
              </Col>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Telefone</Form.Label>
                  <Form.Control type="text" name="telefone" value={currentCliente.telefone} onChange={handleChange} />
                </Form.Group>
              </Col>
            </Row>
            <Form.Group className="mb-3">
              <Form.Label>E-mail</Form.Label>
              <Form.Control type="email" name="email" value={currentCliente.email} onChange={handleChange} />
            </Form.Group>
            <Row>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Sexo</Form.Label>
                  <Form.Select name="sexo" value={currentCliente.sexo} onChange={handleChange}>
                    <option value="">Selecione...</option>
                    {SEXOS.map(s => <option key={s} value={s}>{s}</option>)}
                  </Form.Select>
                </Form.Group>
              </Col>
              <Col md={6}>
                <Form.Group className="mb-3">
                  <Form.Label>Cidade</Form.Label>
                  <Form.Control type="text" name="cidade" value={currentCliente.cidade} onChange={handleChange} />
                </Form.Group>
              </Col>
            </Row>
            <div className="mt-2 d-flex flex-column gap-2">
              <Form.Check
                type="switch"
                id="flamengo-switch"
                label="❤️ Torce para o Flamengo (10% de desconto)"
                name="torceFlamengo"
                checked={currentCliente.torceFlamengo}
                onChange={handleChange}
              />
              <Form.Check
                type="switch"
                id="onepiece-switch"
                label="⚓ Assiste One Piece (10% de desconto)"
                name="assisteOnePiece"
                checked={currentCliente.assisteOnePiece}
                onChange={handleChange}
              />
            </div>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseForm}>Cancelar</Button>
          <Button variant="primary" onClick={handleSave}>Salvar</Button>
        </Modal.Footer>
      </Modal>

      {/* Modal Confirmar Delete */}
      <Modal show={showDeleteModal} onHide={handleCloseDelete}>
        <Modal.Header closeButton><Modal.Title>Confirmar Exclusão</Modal.Title></Modal.Header>
        <Modal.Body>Tem certeza que deseja eliminar o cliente <strong>{clienteToDelete?.nome}</strong>?</Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseDelete}>Cancelar</Button>
          <Button variant="danger" onClick={handleDelete}>Eliminar</Button>
        </Modal.Footer>
      </Modal>
    </Container>
  );
};

export default ClientesList;
