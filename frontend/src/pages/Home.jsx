import { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Button, Badge, Offcanvas, ListGroup, Alert, Form, Nav } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const Home = () => {
  const { theme } = useOutletContext();
  const [produtos, setProdutos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  // Estados do Carrinho
  const [carrinho, setCarrinho] = useState([]);
  const [showCart, setShowCart] = useState(false);
  const [isProcessing, setIsProcessing] = useState(false);

  // Filtros de Categoria na Vitrine
  const [categoriaAtiva, setCategoriaAtiva] = useState('Todas');
  const categorias = ["Todas", "Cordas", "Teclas", "Percussão", "Sopro", "Acessórios"];

  const userRole = localStorage.getItem('auth-role');
  const userId = localStorage.getItem('auth-user');

  useEffect(() => {
    const carregarDados = async () => {
      setIsLoading(true);
      try {
        const data = await api.get('/produtos');
        setProdutos(data);
      } catch (error) {
        setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar os instrumentos.' });
      } finally {
        setIsLoading(false);
      }
    };
    carregarDados();
  }, []);

  // --- Funções do Carrinho ---
  const handleToggleCart = () => setShowCart(!showCart);

  const adicionarAoCarrinho = (produto) => {
    setCarrinho(prev => {
      const existe = prev.find(item => item.id === produto.id);
      if (existe) {
        return prev.map(item => item.id === produto.id ? { ...item, qtd: item.qtd + 1 } : item);
      }
      return [...prev, { ...produto, qtd: 1 }];
    });
    setShowCart(true);
  };

  const alterarQuantidade = (id, delta) => {
    setCarrinho(prev => prev.map(item => {
      if (item.id === id) {
        const novaQtd = item.qtd + delta;
        return { ...item, qtd: novaQtd > 0 ? novaQtd : 1 };
      }
      return item;
    }));
  };

  const removerDoCarrinho = (id) => setCarrinho(prev => prev.filter(item => item.id !== id));

  const totalCarrinho = carrinho.reduce((acc, item) => acc + (parseFloat(item.preco) * item.qtd), 0);

  const finalizarPedido = async () => {
    setIsProcessing(true);
    try {
      await api.post('/vendas', {
        clienteId: userId,
        data: new Date().toLocaleDateString('pt-BR'),
        total: totalCarrinho,
        itens: carrinho.map(i => ({ produtoId: i.id, quantidade: i.qtd }))
      });
      setCarrinho([]);
      setShowCart(false);
      setAlerta({ show: true, variant: 'success', message: 'Pedido realizado! Verifique em "Meus Pedidos".' });
      window.scrollTo(0, 0);
    } catch (e) {
      setAlerta({ show: true, variant: 'danger', message: 'Falha ao processar pedido.' });
    } finally {
      setIsProcessing(false);
    }
  };

  // --- Lógica de Exibição ---
  const destaques = produtos.filter(p => p.destaque).slice(0, 2); // Pega os 2 primeiros marcados como destaque
  const catalogoGeral = produtos.filter(p => {
    const matchCat = categoriaAtiva === 'Todas' || p.categoria === categoriaAtiva;
    return matchCat;
  });

  return (
    <Container className="mt-4">
      {alerta.show && <Alert variant={alerta.variant} dismissible onClose={() => setAlerta({show:false})}>{alerta.message}</Alert>}

      {/* Hero Section */}
      <div className={`p-5 mb-4 rounded-3 ${theme === 'dark' ? 'bg-dark border border-secondary' : 'bg-light shadow-sm'}`}>
        <Row className="align-items-center">
          <Col lg={8}>
            <h1 className="display-5 fw-bold">Rock 'n' Code Instrumentos 🎸</h1>
            <p className="fs-4 text-muted">A sua música começa aqui. Explore a nossa coleção curada por especialistas.</p>
          </Col>
          <Col lg={4} className="text-lg-end">
            {userRole === 'cliente' && (
               <Button variant="primary" size="lg" onClick={handleToggleCart}>
                 🛒 Carrinho ({carrinho.length})
               </Button>
            )}
          </Col>
        </Row>
      </div>

      {isLoading ? (
        <div className="text-center my-5">Carregando catálogo...</div>
      ) : (
        <>
          {/* Seção de Destaques (Sempre no topo) */}
          {destaques.length > 0 && (
            <div className="mb-5">
              <h2 className="mb-4">🔥 Em Destaque</h2>
              <Row>
                {destaques.map(p => (
                  <Col md={6} key={`dest-${p.id}`} className="mb-3">
                    <Card className={`h-100 border-0 shadow ${theme === 'dark' ? 'bg-secondary text-white' : 'bg-white'}`}>
                      <Card.Body className="p-4 d-flex flex-column">
                        <Badge bg="warning" text="dark" className="mb-2 align-self-start">OFERTA ESPECIAL</Badge>
                        <Card.Title className="display-6">{p.nome}</Card.Title>
                        <Card.Text className="text-muted">{p.categoria}</Card.Text>
                        <div className="mt-auto d-flex justify-content-between align-items-center">
                          <h3 className="text-success mb-0">R$ {Number(p.preco).toFixed(2)}</h3>
                          {userRole === 'cliente' && <Button variant="dark" onClick={() => adicionarAoCarrinho(p)}>Comprar Agora</Button>}
                        </div>
                      </Card.Body>
                    </Card>
                  </Col>
                ))}
              </Row>
            </div>
          )}

          {/* Vitrine Geral com Filtros */}
          <div className="mb-5">
            <h2 className="mb-4">Explore a Nossa Loja</h2>
            
            <Nav variant="pills" className="mb-4 gap-2">
              {categorias.map(cat => (
                <Nav.Item key={cat}>
                  <Nav.Link 
                    active={categoriaAtiva === cat} 
                    onClick={() => setCategoriaAtiva(cat)}
                    className="rounded-pill border"
                    style={{ cursor: 'pointer' }}
                  >
                    {cat}
                  </Nav.Link>
                </Nav.Item>
              ))}
            </Nav>

            <Row>
              {catalogoGeral.map(p => (
                <Col xs={12} sm={6} md={4} lg={3} key={p.id} className="mb-4">
                  <Card className={`h-100 ${theme === 'dark' ? 'bg-dark border-secondary' : 'shadow-sm'}`}>
                    <Card.Body className="d-flex flex-column p-3">
                      <div className="mb-2">
                         <Badge bg="info" className="fw-normal">{p.categoria}</Badge>
                      </div>
                      <Card.Title className="fs-5">{p.nome}</Card.Title>
                      <h5 className="text-success mt-auto">R$ {Number(p.preco).toFixed(2)}</h5>
                      <div className="text-muted small mb-2">{p.quantidade > 0 ? `${p.quantidade} em estoque` : 'Esgotado'}</div>
                      
                      {userRole === 'cliente' && (
                        <Button 
                          variant="outline-primary" 
                          size="sm" 
                          disabled={p.quantidade <= 0}
                          onClick={() => adicionarAoCarrinho(p)}
                        >
                          Adicionar
                        </Button>
                      )}
                    </Card.Body>
                  </Card>
                </Col>
              ))}
            </Row>
          </div>
        </>
      )}

      {/* Carrinho Lateral */}
      <Offcanvas show={showCart} onHide={handleToggleCart} placement="end" className={theme === 'dark' ? 'bg-dark text-light' : ''}>
        <Offcanvas.Header closeButton closeVariant={theme === 'dark' ? 'white' : undefined}>
          <Offcanvas.Title>📦 Seu Carrinho</Offcanvas.Title>
        </Offcanvas.Header>
        <Offcanvas.Body className="d-flex flex-column">
          {carrinho.length === 0 ? (
            <p className="text-center mt-5 text-muted">O carrinho está vazio.</p>
          ) : (
            <>
              <ListGroup variant="flush" className="mb-auto">
                {carrinho.map(item => (
                  <ListGroup.Item key={item.id} className={`d-flex justify-content-between align-items-center ${theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}`}>
                    <div className="me-2">
                      <div className="fw-bold">{item.nome}</div>
                      <small className="text-success">R$ {Number(item.preco).toFixed(2)}</small>
                    </div>
                    <div className="d-flex align-items-center gap-2">
                      <Button variant="outline-secondary" size="sm" onClick={() => alterarQuantidade(item.id, -1)}>-</Button>
                      <span className="fw-bold">{item.qtd}</span>
                      <Button variant="outline-secondary" size="sm" onClick={() => alterarQuantidade(item.id, 1)}>+</Button>
                      <Button variant="danger" size="sm" className="ms-1" onClick={() => removerDoCarrinho(item.id)}>X</Button>
                    </div>
                  </ListGroup.Item>
                ))}
              </ListGroup>
              <div className="border-top pt-3 mt-4">
                <h4 className="d-flex justify-content-between">
                  <span>Total:</span>
                  <span className="text-success">R$ {totalCarrinho.toFixed(2)}</span>
                </h4>
                <Button variant="success" size="lg" className="w-100 mt-3" onClick={finalizarPedido} disabled={isProcessing}>
                  {isProcessing ? 'Finalizando...' : 'Concluir Compra'}
                </Button>
              </div>
            </>
          )}
        </Offcanvas.Body>
      </Offcanvas>
    </Container>
  );
};

export default Home;