import { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Form, Button, Alert } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';

const MinhaConta = () => {
  const { theme } = useOutletContext();
  const [cliente, setCliente] = useState({ nome: '', email: '', contato: '' });
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const username = localStorage.getItem('auth-user') || '';

  useEffect(() => {
    // Simula a busca dos dados do cliente na API (ex: GET /clientes/me)
    const carregarDados = async () => {
      setIsLoading(true);
      try {
        await new Promise(resolve => setTimeout(resolve, 600)); // Simula delay de rede
        
        // Dados simulados para o utilizador mockado 'joaocliente'
        setCliente({
          nome: username === 'joaocliente' ? 'João Silva' : username,
          email: `${username}@exemplo.com`,
          contato: '(83) 99999-9999'
        });
      } catch (error) {
        mostrarAlerta('danger', 'Erro ao carregar os dados da conta.');
      } finally {
        setIsLoading(false);
      }
    };

    carregarDados();
  }, [username]);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCliente({ ...cliente, [name]: value });
  };

  const handleSave = async (e) => {
    e.preventDefault();
    setIsLoading(true);
    // Simula o PUT para atualizar os dados (ex: PUT /clientes/me)
    await new Promise(resolve => setTimeout(resolve, 800));
    setIsLoading(false);
    mostrarAlerta('success', 'Dados atualizados com sucesso!');
  };

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
                    className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
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
                    className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
                  />
                </Form.Group>

                <Form.Group className="mb-4">
                  <Form.Label className="fw-bold">Contato</Form.Label>
                  <Form.Control 
                    type="text" 
                    name="contato" 
                    value={cliente.contato} 
                    onChange={handleChange} 
                    disabled={isLoading}
                    className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
                  />
                </Form.Group>

                <Button variant="success" type="submit" className="w-100 py-2" disabled={isLoading}>
                  {isLoading ? 'A guardar...' : 'Guardar Alterações'}
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