import { useState, useEffect, useMemo } from 'react';
import { Container, Row, Col, Card, Button, Badge, Offcanvas, ListGroup, Alert, Form, Nav } from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const FORMAS_PAGAMENTO = [
  { value: 'dinheiro',       label: '💵 Dinheiro' },
  { value: 'cartao_credito', label: '💳 Cartão de Crédito' },
  { value: 'cartao_debito',  label: '💳 Cartão de Débito' },
  { value: 'pix',            label: '⚡ Pix' },
  { value: 'boleto',         label: '📄 Boleto' },
  { value: 'berries',        label: '🍓 Berries' },
];

// Arredonda para o múltiplo de 50 >= ao valor — garante que o slider sempre chega no máximo
const arredondarParaMultiplo50 = (valor) => Math.ceil(valor / 50) * 50;

const Home = () => {
  const { theme } = useOutletContext();
  const [produtos, setProdutos]             = useState([]);
  const [isLoading, setIsLoading]           = useState(false);
  const [alerta, setAlerta]                 = useState({ show: false, variant: '', message: '' });
  const [carrinho, setCarrinho]             = useState([]);
  const [showCart, setShowCart]             = useState(false);
  const [isProcessing, setIsProcessing]     = useState(false);
  const [formaPagamento, setFormaPagamento] = useState('dinheiro');

  // Filtro por tipo
  const [categoriaAtiva, setCategoriaAtiva] = useState('Todas');
  const categorias = ['Todas', 'Guitarra', 'Violão', 'Baixo'];
  const mapTipoBanco = { 'Guitarra': 'guitarra', 'Violão': 'violao', 'Baixo': 'baixo' };

  // Filtro por faixa de preço
  const [precoMax, setPrecoMax] = useState(10000);

  const userRole = localStorage.getItem('auth-role');
  const userId   = parseInt(localStorage.getItem('auth-id'), 10);

  useEffect(() => {
    const carregarDados = async () => {
      setIsLoading(true);
      try {
        const data = await api.get('/produtos');
        setProdutos(data);
        if (data.length > 0) {
          const maxReal = Math.max(...data.map(p => parseFloat(p.preco)));
          setPrecoMax(arredondarParaMultiplo50(maxReal));
        }
      } catch {
        setAlerta({ show: true, variant: 'danger', message: 'Erro ao carregar os instrumentos.' });
      } finally {
        setIsLoading(false);
      }
    };
    carregarDados();
  }, []);

  // Limite do slider = múltiplo de 50 >= preço máximo real
  const precoLimite = useMemo(() => {
    if (produtos.length === 0) return 10000;
    const maxReal = Math.max(...produtos.map(p => parseFloat(p.preco)));
    return arredondarParaMultiplo50(maxReal);
  }, [produtos]);

  const handleToggleCart = () => setShowCart(!showCart);

  const adicionarAoCarrinho = (produto) => {
    setCarrinho(prev => {
      const existe = prev.find(item => item.id === produto.id);
      if (existe)
        return prev.map(item => item.id === produto.id ? { ...item, qtd: item.qtd + 1 } : item);
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
        clienteId:      userId,
        funcionarioId:  null,
        formaPagamento: formaPagamento,
        instrumentos:   carrinho.map(i => i.id),
        quantidades:    carrinho.map(i => i.qtd),
      });
      setCarrinho([]);
      setShowCart(false);
      setFormaPagamento('dinheiro');
      setAlerta({ show: true, variant: 'success', message: 'Pedido realizado! Verifique em "Meus Pedidos".' });
      window.scrollTo(0, 0);
    } catch {
      setAlerta({ show: true, variant: 'danger', message: 'Falha ao processar pedido.' });
    } finally {
      setIsProcessing(false);
    }
  };

  const filtrar = (p) => {
    const matchTipo  = categoriaAtiva === 'Todas' || p.tipo === mapTipoBanco[categoriaAtiva];
    const matchPreco = parseFloat(p.preco) <= precoMax;
    return matchTipo && matchPreco;
  };

  const destaques     = produtos.filter(p => p.destaque && filtrar(p)).slice(0, 2);
  const catalogoGeral = produtos.filter(filtrar);

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} dismissible onClose={() => setAlerta({ show: false })}>
          {alerta.message}
        </Alert>
      )}

      {/* Hero */}
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
          {/* Filtros */}
          <div className={`p-3 mb-4 rounded-3 ${theme === 'dark' ? 'bg-dark border border-secondary' : 'bg-light shadow-sm'}`}>
            <Row className="align-items-center g-3">
              {/* Filtro por tipo */}
              <Col xs={12} md={7}>
                <div className="fw-semibold mb-2">Tipo</div>
                <Nav variant="pills" className="gap-2 flex-wrap">
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
              </Col>

              {/* Slider de preço */}
              <Col xs={12} md={5}>
                <div className="fw-semibold mb-2">
                  Preço máximo:{' '}
                  <span className="text-success">R$ {precoMax.toLocaleString('pt-BR')}</span>
                </div>
                <Form.Range
                  min={0}
                  max={precoLimite}
                  step={50}
                  value={precoMax}
                  onChange={e => setPrecoMax(Number(e.target.value))}
                  className="w-100"
                />
                <div className="d-flex justify-content-between">
                  <small className="text-muted">R$ 0</small>
                  <small className="text-muted">R$ {precoLimite.toLocaleString('pt-BR')}</small>
                </div>
              </Col>
            </Row>
          </div>

          {/* Destaques */}
          {destaques.length > 0 && (
            <div className="mb-5">
              <h3 className="mb-4 text-danger">🔥 Em Destaque {categoriaAtiva !== 'Todas' ? `(${categoriaAtiva})` : ''}</h3>
              <Row>
                {destaques.map(p => (
                  <Col md={6} key={`dest-${p.id}`} className="mb-3">
                    <Card className={`h-100 border-0 shadow ${theme === 'dark' ? 'bg-secondary text-white' : 'bg-white'}`}>
                      <Card.Body className="p-4 d-flex flex-column">
                        <Badge bg="warning" text="dark" className="mb-2 align-self-start">OFERTA ESPECIAL</Badge>
                        <Card.Title className="display-6">{p.nome}</Card.Title>
                        <Card.Text className="text-muted text-capitalize">{p.marca} • {p.tipo}</Card.Text>
                        <div className="mt-auto d-flex justify-content-between align-items-center">
                          <h3 className="text-success mb-0">R$ {Number(p.preco).toFixed(2)}</h3>
                          {userRole === 'cliente' && (
                            <Button variant="dark" onClick={() => adicionarAoCarrinho(p)}>Comprar Agora</Button>
                          )}
                        </div>
                      </Card.Body>
                    </Card>
                  </Col>
                ))}
              </Row>
            </div>
          )}

          {/* Catálogo */}
          <div className="mb-5">
            <h3 className="mb-4">
              Catálogo Geral
              <Badge bg="secondary" className="ms-2 fs-6">{catalogoGeral.length}</Badge>
            </h3>
            {catalogoGeral.length === 0 ? (
              <p className="text-muted">Nenhum instrumento encontrado para os filtros selecionados.</p>
            ) : (
              <Row>
                {catalogoGeral.map(p => (
                  <Col xs={12} sm={6} md={4} lg={3} key={p.id} className="mb-4">
                    <Card className={`h-100 ${theme === 'dark' ? 'bg-dark border-secondary' : 'shadow-sm'}`}>
                      <Card.Body className="d-flex flex-column p-3">
                        <div className="mb-2">
                          <Badge bg="info" className="fw-normal text-capitalize">{p.tipo}</Badge>
                        </div>
                        <Card.Title className="fs-5">{p.nome}</Card.Title>
                        <Card.Text className="text-muted small">{p.marca}</Card.Text>
                        <h5 className="text-success mt-auto">R$ {Number(p.preco).toFixed(2)}</h5>
                        <div className="text-muted small mb-2">
                          {p.quantidade > 0 ? `${p.quantidade} em estoque` : 'Esgotado'}
                        </div>
                        {userRole === 'cliente' && (
                          <Button
                            variant="outline-primary" size="sm"
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
            )}
          </div>
        </>
      )}

      {/* Carrinho Lateral */}
      <Offcanvas show={showCart} onHide={handleToggleCart} placement="end"
                 className={theme === 'dark' ? 'bg-dark text-light' : ''}>
        <Offcanvas.Header closeButton closeVariant={theme === 'dark' ? 'white' : undefined}>
          <Offcanvas.Title>📦 Seu Carrinho</Offcanvas.Title>
        </Offcanvas.Header>
        <Offcanvas.Body className="d-flex flex-column">
          {carrinho.length === 0 ? (
            <p className="text-center mt-5 text-muted">O carrinho está vazio.</p>
          ) : (
            <>
              <ListGroup variant="flush" className="mb-3">
                {carrinho.map(item => (
                  <ListGroup.Item key={item.id}
                    className={`d-flex justify-content-between align-items-center ${theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}`}>
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

              <Form.Group className="mb-3 mt-2 px-1">
                <Form.Label className="fw-bold">Forma de Pagamento</Form.Label>
                <Form.Select
                  value={formaPagamento}
                  onChange={e => setFormaPagamento(e.target.value)}
                  className={theme === 'dark' ? 'bg-dark text-light border-secondary' : ''}
                >
                  {FORMAS_PAGAMENTO.map(f => (
                    <option key={f.value} value={f.value}>{f.label}</option>
                  ))}
                </Form.Select>
              </Form.Group>

              <div className="border-top pt-3">
                <h4 className="d-flex justify-content-between">
                  <span>Total:</span>
                  <span className="text-success">R$ {totalCarrinho.toFixed(2)}</span>
                </h4>
                <Button variant="success" size="lg" className="w-100 mt-3"
                  onClick={finalizarPedido} disabled={isProcessing}>
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
