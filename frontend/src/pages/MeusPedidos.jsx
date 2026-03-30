import { useState, useEffect } from 'react';
import { Table, Container, Row, Col, Alert } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const MeusPedidos = () => {
  const { theme } = useOutletContext();
  const [pedidos, setPedidos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  useEffect(() => {
    const carregarPedidos = async () => {
      setIsLoading(true);
      try {
        // Num cenário real com a API, a rota seria algo como GET /vendas/me
        // Como estamos a usar o mock global de /vendas, vamos filtrar localmente
        const data = await api.get('/vendas');
        
        // Simulação: assumindo que o "joaocliente" tem o ID 1 no nosso mock
        const meusPedidos = data.filter(venda => venda.clienteId === 1 || venda.clienteId === '1');
        
        // Ordena para mostrar os mais recentes primeiro (assumindo formato DD/MM/YYYY)
        const parseData = (dataStr) => {
          if (!dataStr) return new Date(0);
          const partes = dataStr.split('/');
          return partes.length === 3 ? new Date(`${partes[2]}-${partes[1]}-${partes[0]}`) : new Date(dataStr);
        };
        
        meusPedidos.sort((a, b) => parseData(b.data) - parseData(a.data));
        
        setPedidos(meusPedidos);
      } catch (error) {
        setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar o histórico de pedidos.' });
      } finally {
        setIsLoading(false);
      }
    };

    carregarPedidos();
  }, []);

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="mb-4">
        <Col>
          <h2>Os Meus Pedidos</h2>
          <p className="text-muted">Consulte aqui o histórico das suas compras efetuadas.</p>
        </Col>
      </Row>

      <Table variant={theme} striped bordered hover responsive>
        <thead>
          <tr>
            <th>Nº do Pedido</th>
            <th>Data da Compra</th>
            <th>Valor Total (R$)</th>
            <th>Status</th>
          </tr>
        </thead>
        <tbody>
          {isLoading ? (
             <tr>
               <td colSpan="4" className="text-center py-4">A carregar os seus pedidos...</td>
             </tr>
          ) : pedidos.length === 0 ? (
            <tr>
              <td colSpan="4" className="text-center py-4">Ainda não efetuou nenhuma compra.</td>
            </tr>
          ) : (
            pedidos.map((pedido) => (
              <tr key={pedido.id}>
                <td className="align-middle fw-bold">#{pedido.id}</td>
                <td className="align-middle">{pedido.data}</td>
                <td className="align-middle">R$ {Number(pedido.total).toFixed(2)}</td>
                <td className="align-middle">
                  <span className="badge bg-success">Concluído</span>
                </td>
              </tr>
            ))
          )}
        </tbody>
      </Table>
    </Container>
  );
};

export default MeusPedidos;