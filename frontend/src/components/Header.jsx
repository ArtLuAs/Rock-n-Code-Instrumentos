import { Navbar, Container, Nav, Form, Button } from 'react-bootstrap';
import { Link, useNavigate } from 'react-router-dom';

const Header = ({ theme, toggleTheme }) => {
  const navigate = useNavigate();
  const navBackgroundColor = theme === 'dark' ? '#121212' : '#212529';
  
  const username = localStorage.getItem('auth-user') || 'Usuário';
  const userRole = localStorage.getItem('auth-role');

  const handleLogout = () => {
    localStorage.removeItem('auth-token');
    localStorage.removeItem('auth-user');
    localStorage.removeItem('auth-role');
    navigate('/login');
  };

  return (
    <Navbar variant="dark" expand="lg" className="shadow-sm" style={{ backgroundColor: navBackgroundColor }}>
      <Container>
        <Navbar.Brand as={Link} to="/">Rock 'n' Code</Navbar.Brand>
        <Navbar.Toggle aria-controls="basic-navbar-nav" />
        <Navbar.Collapse id="basic-navbar-nav">
          <Nav className="me-auto">
            <Nav.Link as={Link} to="/">Home</Nav.Link>
            
            {userRole === 'funcionario' && (
              <>
                <Nav.Link as={Link} to="/clientes">Gestão de Clientes</Nav.Link>
                <Nav.Link as={Link} to="/produtos">Gestão de Produtos</Nav.Link>
                <Nav.Link as={Link} to="/vendas">Histórico de Vendas</Nav.Link>
              </>
            )}

            {userRole === 'cliente' && (
              <>
                {/* Links atualizados para apontarem para as novas rotas */}
                <Nav.Link as={Link} to="/minha-conta">A Minha Conta</Nav.Link>
                <Nav.Link as={Link} to="/meus-pedidos">Meus Pedidos</Nav.Link>
              </>
            )}
          </Nav>
          
          <div className="d-flex align-items-center gap-4 mt-3 mt-lg-0">
            <span className="text-light d-none d-lg-block fw-bold">
              Olá, {username} {userRole === 'funcionario' ? '(Admin)' : ''}
            </span>
            
            <Form className="d-flex align-items-center">
              <Form.Check 
                type="switch" id="theme-switch" label={theme === 'dark' ? '🌙 Escuro' : '☀️ Claro'}
                checked={theme === 'dark'} onChange={toggleTheme} className="text-light m-0" 
              />
            </Form>

            <Button variant="outline-danger" size="sm" onClick={handleLogout}>Sair</Button>
          </div>
        </Navbar.Collapse>
      </Container>
    </Navbar>
  );
};

export default Header;