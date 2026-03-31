import { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Form, Button, Alert } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const MinhaConta = () => {
  const { theme } = useOutletContext();
  const [cliente, setCliente] = useState({ nome: '', email: '', telefone: '', cidade: '' });
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const userId = localStorage.getItem('auth-id');

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  useEffect(() => {
    if (!userId) return;
    const carregarDados = async () => {
      setIsLoading(true);
      try {
        const data = await api.get(`/clientes/${userId}`);
        setCliente({
          nome:     data.nome     || '',
          email:    data.email    || '',
          telefone: data.telefone || '',
          cidade:   data.cidade   || '',
        });
      } catch {
        mostrarAlerta('danger', 'Erro ao carregar os dados da conta.');
      } finally {
        setIsLoading(false);
      }
    };
    carregarDados();
  }, [userId]);

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCliente(prev => ({ ...prev, [name]: value }));
  };

  const handleSave = async (e) => {
    e.preventDefault();
    if (!userId) { mostrarAlerta('danger', 'Sessao invalida. Faca login novamente.'); return; }
    setIsLoading(true);
    try {
      await api.put(`/clientes/${userId}`, cliente);
      mostrarAlerta('success', 'Dados atualizados com sucesso!');
    } catch {
      mostrarAlerta('danger', 'Erro ao salvar os dados. Tente novamente.');
    } finally {
      setIsLoading(false);
    }
  };

  const inputClass = theme === 'dark' ? 'bg-dark text-light border-secondary' : '';

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="justify-content-center">
        <Col md={8} lg={6}>
          <Card className={`shadow-sm border-0 ${theme === 'dark' ? 'bg-dark text-light' : 'bg-white'}`}>
            <Card.Header className="bg-primary text-white py-3">
              <h4 className="mb-0">A Minha Conta</h4>
            </Card.Header>
            <Card.Body className="p-4">
              <Form onSubmit={handleSave}>
                <Form.Group className="mb-3">
                  <Form.Label className="fw-bold">Nome Completo</Form.Label>
                  <Form.Control
                    type="text"
                    name="nome"
                    value={cliente.nome}
                    onChange={handleChange}
                    disabled={isLoading}
                    className={inputClass}
                  />
                </Form.Group>

                <Form.Group className="mb-3">
                  <Form.Label className="fw-bold">E-mail</Form.Label>
                  <Form.Control
                    type="email"
                    name="email"
                    value={cliente.email}
                    onChange={handleChange}
                    disabled={isLoading}
                    className={inputClass}
                  />
                </Form.Group>

                <Form.Group className="mb-3">
                  <Form.Label className="fw-bold">Telefone</Form.Label>
                  <Form.Control
                    type="text"
                    name="telefone"
                    value={cliente.telefone}
                    onChange={handleChange}
                    disabled={isLoading}
                    className={inputClass}
                  />
                </Form.Group>

                <Form.Group className="mb-4">
                  <Form.Label className="fw-bold">Cidade</Form.Label>
                  <Form.Control
                    type="text"
                    name="cidade"
                    value={cliente.cidade}
                    onChange={handleChange}
                    disabled={isLoading}
                    className={inputClass}
                  />
                </Form.Group>

                <Button variant="success" type="submit" className="w-100 py-2" disabled={isLoading}>
                  {isLoading ? 'A guardar...' : 'Guardar Alteracoes'}
                </Button>
              </Form>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default MinhaConta;
