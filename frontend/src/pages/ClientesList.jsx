import { useState, useEffect } from 'react';
import { Table, Button, Modal, Form, Container, Row, Col, Alert, InputGroup } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const ClientesList = () => {
  const { theme } = useOutletContext();
  const [clientes, setClientes] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  // --- Estados de Filtro e Ordenação ---
  const [searchTerm, setSearchTerm] = useState('');
  const [sortBy, setSortBy] = useState('id-asc');

  const [showFormModal, setShowFormModal] = useState(false);
  const [isEditing, setIsEditing] = useState(false);
  const [currentCliente, setCurrentCliente] = useState({ id: null, nome: '', email: '', contato: '' });
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  const [clienteToDelete, setClienteToDelete] = useState(null);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const carregarClientes = async () => {
    setIsLoading(true);
    try {
      const data = await api.get('/clientes');
      setClientes(data);
    } catch (error) {
      mostrarAlerta('danger', 'Erro ao carregar os clientes.');
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    carregarClientes();
  }, []);

  const handleCloseForm = () => setShowFormModal(false);
  
  const handleShowNew = () => {
    setIsEditing(false);
    setCurrentCliente({ id: null, nome: '', email: '', contato: '' });
    setShowFormModal(true);
  };
  
  const handleShowEdit = (cliente) => {
    setIsEditing(true);
    setCurrentCliente(cliente);
    setShowFormModal(true);
  };

  const handleCloseDelete = () => setShowDeleteModal(false);
  
  const handleShowDelete = (cliente) => {
    setClienteToDelete(cliente);
    setShowDeleteModal(true);
  };

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCurrentCliente({ ...currentCliente, [name]: value });
  };

  const handleSave = async () => {
    try {
      if (isEditing) {
        await api.put(`/clientes/${currentCliente.id}`, currentCliente);
        mostrarAlerta('success', 'Cliente atualizado com sucesso!');
      } else {
        await api.post('/clientes', currentCliente);
        mostrarAlerta('success', 'Novo cliente registado com sucesso!');
      }
      handleCloseForm();
      carregarClientes();
    } catch (error) {
      mostrarAlerta('danger', 'Ocorreu um erro ao salvar o cliente.');
    }
  };

  const handleDelete = async () => {
    try {
      await api.delete(`/clientes/${clienteToDelete.id}`);
      mostrarAlerta('success', 'Cliente eliminado com sucesso!');
      handleCloseDelete();
      carregarClientes();
    } catch (error) {
      mostrarAlerta('danger', 'Erro ao tentar eliminar o cliente.');
    }
  };

  // --- Lógica de Derivação (Filtro + Ordenação) ---
  const clientesFiltrados = clientes
    .filter(cliente => cliente.nome.toLowerCase().includes(searchTerm.toLowerCase()))
    .sort((a, b) => {
      if (sortBy === 'nome-asc') return a.nome.localeCompare(b.nome);
      if (sortBy === 'nome-desc') return b.nome.localeCompare(a.nome);
      if (sortBy === 'id-desc') return b.id - a.id;
      return a.id - b.id; // id-asc (padrão)
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
          <Button variant="success" onClick={handleShowNew} disabled={isLoading}>
            + Novo Cliente
          </Button>
        </Col>
      </Row>

      {/* --- Toolbar de Filtros --- */}
      <div className="p-3 mb-4 rounded border" style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row>
          <Col md={6}>
            <Form.Group>
              <Form.Label className="fw-bold">Buscar Cliente:</Form.Label>
              <InputGroup>
                <InputGroup.Text>🔍</InputGroup.Text>
                <Form.Control 
                  placeholder="Buscar por nome..." 
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
          <tr><th>ID</th><th>Nome</th><th>E-mail</th><th>Contato</th><th>Ações</th></tr>
        </thead>
        <tbody>
          {isLoading ? (
            <tr><td colSpan="5" className="text-center">A processar...</td></tr>
          ) : clientesFiltrados.length === 0 ? (
            <tr><td colSpan="5" className="text-center">Nenhum cliente encontrado.</td></tr>
          ) : (
            clientesFiltrados.map((cliente) => (
              <tr key={cliente.id}>
                <td>{cliente.id}</td><td>{cliente.nome}</td><td>{cliente.email}</td><td>{cliente.contato}</td>
                <td>
                  <Button variant="primary" size="sm" className="me-2" onClick={() => handleShowEdit(cliente)}>Editar</Button>
                  <Button variant="danger" size="sm" onClick={() => handleShowDelete(cliente)}>Eliminar</Button>
                </td>
              </tr>
            ))
          )}
        </tbody>
      </Table>

      {/* --- Modais --- */}
      <Modal show={showFormModal} onHide={handleCloseForm}>
        <Modal.Header closeButton><Modal.Title>{isEditing ? 'Editar Cliente' : 'Novo Cliente'}</Modal.Title></Modal.Header>
        <Modal.Body>
          <Form>
            <Form.Group className="mb-3">
              <Form.Label>Nome</Form.Label>
              <Form.Control type="text" name="nome" value={currentCliente.nome} onChange={handleChange} />
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>E-mail</Form.Label>
              <Form.Control type="email" name="email" value={currentCliente.email} onChange={handleChange} />
            </Form.Group>
            <Form.Group className="mb-3">
              <Form.Label>Contato</Form.Label>
              <Form.Control type="text" name="contato" value={currentCliente.contato} onChange={handleChange} />
            </Form.Group>
          </Form>
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={handleCloseForm}>Cancelar</Button>
          <Button variant="primary" onClick={handleSave}>Salvar</Button>
        </Modal.Footer>
      </Modal>

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