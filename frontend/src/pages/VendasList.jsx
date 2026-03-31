import { useState, useEffect } from 'react';
import { Table, Button, Container, Row, Col, Alert, InputGroup, Form, Badge } from 'react-bootstrap';
import { Link, useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

// Mapeia status_pagamento para cor do badge
const statusVariant = (status) => {
  if (status === 'confirmado') return 'success';
  if (status === 'recusado')   return 'danger';
  return 'warning'; // pendente
};

const VendasList = () => {
  const { theme } = useOutletContext();
  const [vendas, setVendas] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const [searchTerm, setSearchTerm] = useState('');
  const [sortBy, setSortBy]         = useState('data-desc');

  const carregarVendas = async () => {
    setIsLoading(true);
    try {
      const data = await api.get('/vendas');
      setVendas(data);
    } catch {
      setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar o historico de vendas do servidor.' });
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => { carregarVendas(); }, []);

  const parseData = (dataStr) => {
    if (!dataStr) return new Date(0);
    const partes = dataStr.split('/');
    if (partes.length === 3)
      return new Date(`${partes[2]}-${partes[1]}-${partes[0]}`);
    return new Date(dataStr);
  };

  const vendasFiltradas = vendas
    .filter(v => {
      const term = searchTerm.toLowerCase();
      const idMatch      = v.id?.toString().includes(term);
      const clienteMatch = (v.cliente || '').toLowerCase().includes(term) ||
                           v.clienteId?.toString().includes(term);
      return idMatch || clienteMatch;
    })
    .sort((a, b) => {
      if (sortBy === 'valor-desc') return parseFloat(b.total) - parseFloat(a.total);
      if (sortBy === 'valor-asc')  return parseFloat(a.total) - parseFloat(b.total);
      if (sortBy === 'data-desc')  return parseData(b.data)   - parseData(a.data);
      if (sortBy === 'data-asc')   return parseData(a.data)   - parseData(b.data);
      return 0;
    });

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="mb-3 align-items-center">
        <Col md={6}><h2>Historico de Vendas</h2></Col>
        <Col md={6} className="text-end">
          <Button variant="success" as={Link} to="/vendas/nova" disabled={isLoading}>
            + Nova Venda
          </Button>
        </Col>
      </Row>

      <div className="p-3 mb-4 rounded border"
           style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row>
          <Col md={6}>
            <Form.Group>
              <Form.Label className="fw-bold">Buscar Venda:</Form.Label>
              <InputGroup>
                <InputGroup.Text>&#128269;</InputGroup.Text>
                <Form.Control
                  placeholder="Buscar por ID, nome ou ID do cliente..."
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
                <option value="data-desc">Data (Mais Recente)</option>
                <option value="data-asc">Data (Mais Antiga)</option>
                <option value="valor-desc">Valor Total (Maior para Menor)</option>
                <option value="valor-asc">Valor Total (Menor para Maior)</option>
              </Form.Select>
            </Form.Group>
          </Col>
        </Row>
      </div>

      <Table variant={theme} striped bordered hover responsive>
        <thead>
          <tr>
            <th>#</th>
            <th>Cliente</th>
            <th>Data</th>
            <th>Pagamento</th>
            <th>Status</th>
            <th>Total (R$)</th>
          </tr>
        </thead>
        <tbody>
          {isLoading ? (
            <tr><td colSpan="6" className="text-center">A carregar...</td></tr>
          ) : vendasFiltradas.length === 0 ? (
            <tr><td colSpan="6" className="text-center">Nenhum registo encontrado.</td></tr>
          ) : (
            vendasFiltradas.map((venda) => (
              <tr key={venda.id}>
                <td>{venda.id}</td>
                <td>{venda.cliente ? `${venda.cliente} (#${venda.clienteId})` : `#${venda.clienteId}`}</td>
                <td>{venda.data}</td>
                <td>{(venda.formaPagamento || '').replace(/_/g, ' ')}</td>
                <td>
                  <Badge bg={statusVariant(venda.statusPagamento)}>
                    {venda.statusPagamento || 'pendente'}
                  </Badge>
                </td>
                <td>{Number(venda.total).toFixed(2)}</td>
              </tr>
            ))
          )}
        </tbody>
      </Table>
    </Container>
  );
};

export default VendasList;
