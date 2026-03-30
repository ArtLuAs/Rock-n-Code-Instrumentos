import { useState, useEffect } from 'react';
import { Table, Button, Container, Row, Col, Alert, InputGroup, Form } from 'react-bootstrap';
import { Link, useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const VendasList = () => {
  const { theme } = useOutletContext();
  const [vendas, setVendas] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  // --- Estados de Filtro e Ordenação ---
  const [searchTerm, setSearchTerm] = useState('');
  const [sortBy, setSortBy] = useState('data-desc'); // Mais recente por defeito

  const carregarVendas = async () => {
    setIsLoading(true);
    try {
      const data = await api.get('/vendas');
      setVendas(data);
    } catch (error) {
      setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar o histórico de vendas do servidor.' });
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    carregarVendas();
  }, []);

  // Função auxiliar para converter strings de data (DD/MM/YYYY) para objetos Date ordenáveis
  const parseData = (dataStr) => {
    if (!dataStr) return new Date(0);
    const partes = dataStr.split('/');
    if (partes.length === 3) {
      return new Date(`${partes[2]}-${partes[1]}-${partes[0]}`);
    }
    return new Date(dataStr); // Fallback caso o backend já envie em formato ISO
  };

  // --- Lógica de Derivação (Filtro + Ordenação) ---
  const vendasFiltradas = vendas
    .filter(v => {
      const term = searchTerm.toLowerCase();
      // Filtra pelo ID da venda ou pelo ID do cliente
      const idMatch = v.id?.toString().includes(term);
      const clienteMatch = v.clienteId?.toString().includes(term);
      return idMatch || clienteMatch;
    })
    .sort((a, b) => {
      if (sortBy === 'valor-desc') return parseFloat(b.total) - parseFloat(a.total);
      if (sortBy === 'valor-asc') return parseFloat(a.total) - parseFloat(b.total);
      if (sortBy === 'data-desc') return parseData(b.data) - parseData(a.data);
      if (sortBy === 'data-asc') return parseData(a.data) - parseData(b.data);
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
        <Col md={6}>
          <h2>Histórico de Vendas</h2>
        </Col>
        <Col md={6} className="text-end">
          <Button variant="success" as={Link} to="/vendas/nova" disabled={isLoading}>
            + Nova Venda
          </Button>
        </Col>
      </Row>

      {/* --- Toolbar de Filtros --- */}
      <div className="p-3 mb-4 rounded border" style={{ backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
        <Row>
          <Col md={6}>
            <Form.Group>
              <Form.Label className="fw-bold">Buscar Venda:</Form.Label>
              <InputGroup>
                <InputGroup.Text>🔍</InputGroup.Text>
                <Form.Control 
                  placeholder="Buscar por ID da Venda ou do Cliente..." 
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
            <th>ID da Venda</th>
            <th>Cliente (ID)</th>
            <th>Data</th>
            <th>Valor Total (R$)</th>
          </tr>
        </thead>
        <tbody>
          {isLoading ? (
             <tr>
               <td colSpan="4" className="text-center">A processar...</td>
             </tr>
          ) : vendasFiltradas.length === 0 ? (
            <tr>
              <td colSpan="4" className="text-center">Nenhum registo de venda encontrado.</td>
            </tr>
          ) : (
            vendasFiltradas.map((venda) => (
              <tr key={venda.id}>
                <td>{venda.id}</td>
                <td>Cliente #{venda.clienteId}</td>
                <td>{venda.data}</td>
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