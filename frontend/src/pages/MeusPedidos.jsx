import { useState, useEffect } from 'react';
import { Table, Container, Row, Col, Alert } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const MeusPedidos = () => {
  const { theme } = useOutletContext();
  const [pedidos, setPedidos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const clienteId = localStorage.getItem('auth-id');

  useEffect(() => {
    const carregarPedidos = async () => {
      if (!clienteId) {
        setAlerta({ show: true, variant: 'warning', message: 'Você precisa estar logado para ver seus pedidos.' });
        return;
      }
      setIsLoading(true);
      try {
        const data = await api.get(`/vendas/me?clienteId=${clienteId}`);
        setPedidos(data);
      } catch (error) {
        setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar o histórico de pedidos.' });
      } finally {
        setIsLoading(false);
      }
    };

    carregarPedidos();
  }, [clienteId]);

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
            <th>Desconto</th>
            <th>Valor Total (R$)</th>
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
                <td className="align-middle">
                  {pedido.desconto > 0
                    ? <span className="badge bg-success">{pedido.desconto}% OFF</span>
                    : <span className="text-muted">—</span>}
                </td>
                <td className="align-middle">R$ {Number(pedido.total).toFixed(2)}</td>
              </tr>
            ))
          )}
        </tbody>
      </Table>
    </Container>
  );
};

export default MeusPedidos;
