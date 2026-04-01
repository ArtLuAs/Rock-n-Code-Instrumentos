import { useState, useEffect } from 'react';
import { Table, Button, Container, Row, Col, Alert, InputGroup, Form, Badge } from 'react-bootstrap';
import { Link, useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const statusVariant = (status) => {
  if (status === 'confirmado') return 'success';
  if (status === 'recusado')   return 'danger';
  return 'warning';
};

const statusLabel = (status) => {
  if (status === 'confirmado') return '✔ confirmado';
  if (status === 'recusado')   return '✘ recusado';
  return '⏳ pendente';
};

const VendasList = () => {
  const { theme }   = useOutletContext();
  const funcionarioId = parseInt(localStorage.getItem('auth-id'), 10);

  const [vendas, setVendas]             = useState([]);
  const [isLoading, setIsLoading]       = useState(false);
  const [alerta, setAlerta]             = useState({ show: false, variant: '', message: '' });
  const [atualizando, setAtualizando]   = useState(null);
  const [searchTerm, setSearchTerm]     = useState('');
  const [sortBy, setSortBy]             = useState('data-desc');

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const carregarVendas = async () => {
    setIsLoading(true);
    try {
      const data = await api.get('/vendas');
      setVendas(data);
    } catch {
      mostrarAlerta('danger', 'Erro ao carregar o historico de vendas.');
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => { carregarVendas(); }, []);

  const handleAlterarStatus = async (vendaId, novoStatus) => {
    setAtualizando(vendaId);
    try {
      // Envia o ID do funcionario logado ao confirmar/recusar
      await api.patch(`/vendas/${vendaId}/status`, {
        status:        novoStatus,
        funcionarioId: novoStatus !== 'pendente' ? funcionarioId : null,
      });
      setVendas(prev =>
        prev.map(v => v.id === vendaId
          ? { ...v,
              statusPagamento: novoStatus,
              confirmadoPor: novoStatus !== 'pendente'
                ? localStorage.getItem('auth-nome')
                : null
            }
          : v
        )
      );
      mostrarAlerta('success', `Venda #${vendaId} marcada como "${novoStatus}".`);
    } catch (err) {
      mostrarAlerta('danger', `Erro ao atualizar status: ${err.message}`);
    } finally {
      setAtualizando(null);
    }
  };

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
      return v.id?.toString().includes(term) ||
             (v.cliente || '').toLowerCase().includes(term) ||
             v.clienteId?.toString().includes(term);
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
            <th>Total (R$)</th>
            <th>Status / Confirmado por</th>
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
                <td className="align-middle">{venda.id}</td>
                <td className="align-middle">
                  {venda.cliente ? `${venda.cliente} (#${venda.clienteId})` : `#${venda.clienteId}`}
                </td>
                <td className="align-middle">{venda.data}</td>
                <td className="align-middle">{(venda.formaPagamento || '').replace(/_/g, ' ')}</td>
                <td className="align-middle">R$ {Number(venda.total).toFixed(2)}</td>
                <td className="align-middle">
                  <div className="d-flex align-items-center gap-2 flex-wrap">
                    <div>
                      <Badge bg={statusVariant(venda.statusPagamento)}>
                        {statusLabel(venda.statusPagamento)}
                      </Badge>
                      {venda.confirmadoPor && (
                        <div className="text-muted" style={{ fontSize: '0.75rem' }}>
                          por {venda.confirmadoPor}
                        </div>
                      )}
                    </div>
                    {venda.statusPagamento !== 'confirmado' && (
                      <Button size="sm" variant="outline-success"
                        disabled={atualizando === venda.id}
                        onClick={() => handleAlterarStatus(venda.id, 'confirmado')}
                        title="Confirmar pagamento">✔</Button>
                    )}
                    {venda.statusPagamento !== 'recusado' && (
                      <Button size="sm" variant="outline-danger"
                        disabled={atualizando === venda.id}
                        onClick={() => handleAlterarStatus(venda.id, 'recusado')}
                        title="Recusar pagamento">✘</Button>
                    )}
                    {venda.statusPagamento !== 'pendente' && (
                      <Button size="sm" variant="outline-warning"
                        disabled={atualizando === venda.id}
                        onClick={() => handleAlterarStatus(venda.id, 'pendente')}
                        title="Voltar para pendente">↩</Button>
                    )}
                  </div>
                </td>
              </tr>
            ))
          )}
        </tbody>
      </Table>
    </Container>
  );
};

export default VendasList;
