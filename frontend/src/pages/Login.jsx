import { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Form, Button, Alert } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';

const Login = () => {
  const navigate = useNavigate();
  const [credenciais, setCredenciais] = useState({ username: '', password: '' });
  const [erro, setErro] = useState('');
  const [isLoading, setIsLoading] = useState(false);

  const theme = localStorage.getItem('app-theme') || 'light';

  useEffect(() => {
    // Se já estiver logado, vai direto para a Home
    if (localStorage.getItem('auth-token') === 'true') {
      navigate('/');
    }
  }, [navigate]);

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCredenciais({ ...credenciais, [name]: value });
  };

  const handleLogin = async (e) => {
    e.preventDefault();
    setErro('');
    setIsLoading(true);
    
    // --- MOCK COM PERFIS DE ACESSO (ROLES) ---
    const contasValidas = [
      { username: 'vendedor1', password: 'admin', role: 'funcionario' },
      { username: 'vendedor2', password: 'admin', role: 'funcionario' },
      { username: 'admin', password: 'admin', role: 'funcionario' },
      { username: 'joaocliente', password: '123', role: 'cliente' } // Nova conta de cliente
    ];

    // Simula tempo de rede
    await new Promise(resolve => setTimeout(resolve, 800));

    const contaEncontrada = contasValidas.find(
      (c) => c.username === credenciais.username && c.password === credenciais.password
    );

    if (contaEncontrada) {
      localStorage.setItem('auth-token', 'true');
      localStorage.setItem('auth-user', contaEncontrada.username);
      localStorage.setItem('auth-role', contaEncontrada.role); // Guarda se é funcionário ou cliente
      navigate('/');
    } else {
      setErro('Usuário ou senha incorretos.');
      setIsLoading(false);
    }
  };

  return (
    <Container fluid className="d-flex align-items-center justify-content-center" style={{ minHeight: '100vh', backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
      <Row className="w-100">
        <Col md={{ span: 6, offset: 3 }} lg={{ span: 4, offset: 4 }}>
          <Card className={`shadow-lg border-0 ${theme === 'dark' ? 'bg-dark text-light' : 'bg-white'}`}>
            <Card.Header className="text-center bg-primary text-white py-4 border-0">
              <h3 className="mb-0 fw-bold">Rock 'n' Code</h3>
              <p className="mb-0 text-white-50">Acesso ao Sistema</p>
            </Card.Header>
            <Card.Body className="p-4 p-md-5">
              <h5 className="text-center mb-4">Autenticação</h5>
              
              {erro && <Alert variant="danger">{erro}</Alert>}
              
              <Form onSubmit={handleLogin}>
                <Form.Group className="mb-3" controlId="formUsername">
                  <Form.Label>Usuário</Form.Label>
                  <Form.Control 
                    type="text" 
                    placeholder="Digite o seu usuário" 
                    name="username"
                    value={credenciais.username} 
                    onChange={handleChange} 
                    required
                    className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
                  />
                </Form.Group>
                
                <Form.Group className="mb-4" controlId="formPassword">
                  <Form.Label>Senha</Form.Label>
                  <Form.Control 
                    type="password" 
                    placeholder="Digite a sua senha" 
                    name="password"
                    value={credenciais.password} 
                    onChange={handleChange} 
                    required
                    className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
                  />
                </Form.Group>
                
                <Button variant="primary" type="submit" className="w-100 py-2 fw-bold" disabled={isLoading}>
                  {isLoading ? 'A autenticar...' : 'Entrar'}
                </Button>
              </Form>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default Login;