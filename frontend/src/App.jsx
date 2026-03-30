import { useState, useEffect } from 'react';
import { BrowserRouter as Router, Routes, Route, Outlet, Navigate } from 'react-router-dom';
import { Container, Row, Col } from 'react-bootstrap';
import Header from './components/Header';
import Home from './pages/Home';
import ClientesList from './pages/ClientesList';
import ProdutosList from './pages/ProdutosList';
import VendasList from './pages/VendasList';
import NovaVenda from './pages/NovaVenda';
import Login from './pages/Login';
import MinhaConta from './pages/MinhaConta';   // <-- Nova importação
import MeusPedidos from './pages/MeusPedidos'; // <-- Nova importação

const Layout = ({ theme, toggleTheme }) => (
  <>
    <Header theme={theme} toggleTheme={toggleTheme} />
    <Container className="mt-4 pb-5">
      <Row>
        <Col>
          <Outlet context={{ theme }} />
        </Col>
      </Row>
    </Container>
  </>
);

const PrivateRoute = ({ children, allowedRoles }) => {
  const isAuthenticated = localStorage.getItem('auth-token') === 'true';
  const userRole = localStorage.getItem('auth-role');

  if (!isAuthenticated) {
    return <Navigate to="/login" replace />;
  }

  if (allowedRoles && !allowedRoles.includes(userRole)) {
    return <Navigate to="/" replace />;
  }

  return children;
};

function App() {
  const [theme, setTheme] = useState(() => localStorage.getItem('app-theme') || 'light');

  useEffect(() => {
    document.documentElement.setAttribute('data-bs-theme', theme);
    localStorage.setItem('app-theme', theme);
  }, [theme]);

  const toggleTheme = () => setTheme((prev) => (prev === 'light' ? 'dark' : 'light'));

  return (
    <Router>
      <Routes>
        <Route path="/login" element={<Login />} />

        <Route path="/" element={<PrivateRoute><Layout theme={theme} toggleTheme={toggleTheme} /></PrivateRoute>}>
          
          <Route index element={<Home />} />

          {/* Rotas Administrativas (Funcionários) */}
          <Route path="clientes" element={<PrivateRoute allowedRoles={['funcionario']}><ClientesList /></PrivateRoute>} />
          <Route path="produtos" element={<PrivateRoute allowedRoles={['funcionario']}><ProdutosList /></PrivateRoute>} />
          <Route path="vendas" element={<PrivateRoute allowedRoles={['funcionario']}><VendasList /></PrivateRoute>} />
          <Route path="vendas/nova" element={<PrivateRoute allowedRoles={['funcionario']}><NovaVenda /></PrivateRoute>} />

          {/* Novas Rotas do Cliente */}
          <Route path="minha-conta" element={<PrivateRoute allowedRoles={['cliente']}><MinhaConta /></PrivateRoute>} />
          <Route path="meus-pedidos" element={<PrivateRoute allowedRoles={['cliente']}><MeusPedidos /></PrivateRoute>} />
          
        </Route>
      </Routes>
    </Router>
  );
}

export default App;