import { useState, useEffect } from 'react';
import { Container, Row, Col, Form, Button, Table, Card, Alert } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';
import { api } from '../services/api';

const FORMAS_PAGAMENTO = ['dinheiro', 'cartao_credito', 'cartao_debito', 'pix'];

const NovaVenda = () => {
  const navigate = useNavigate();

  const [clientes, setClientes]           = useState([]);
  const [produtos, setProdutos]           = useState([]);
  const [funcionarios, setFuncionarios]   = useState([]);
  const [isLoading, setIsLoading]         = useState(false);
  const [alerta, setAlerta]               = useState({ show: false, variant: '', message: '' });

  const [selectedCliente, setSelectedCliente]         = useState('');
  const [selectedFuncionario, setSelectedFuncionario] = useState('');
  const [formaPagamento, setFormaPagamento]           = useState('dinheiro');
  const [selectedProduto, setSelectedProduto]         = useState('');
  const [quantidade, setQuantidade]                   = useState(1);
  const [carrinho, setCarrinho]                       = useState([]);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 5000);
  };

  useEffect(() => {
    const carregar = async () => {
      setIsLoading(true);
      try {
        const [clientesData, produtosData, funcionariosData] = await Promise.all([
          api.get('/clientes'),
          api.get('/produtos'),
          api.get('/funcionarios'),
        ]);
        setClientes(clientesData);
        setProdutos(produtosData.filter(p => parseInt(p.quantidade) > 0));
        setFuncionarios(funcionariosData);
      } catch {
        mostrarAlerta('danger', 'Erro ao carregar os dados do formulário.');
      } finally {
        setIsLoading(false);
      }
    };
    carregar();
  }, []);

  const handleAddProduto = () => {
    if (!selectedProduto || quantidade <= 0) return;
    const produto = produtos.find(p => p.id === parseInt(selectedProduto));
    if (!produto) return;

    const idx = carrinho.findIndex(i => i.id === produto.id);
    if (idx >= 0) {
      const novo = [...carrinho];
      novo[idx].quantidade += parseInt(quantidade);
      novo[idx].subtotal    = novo[idx].preco * novo[idx].quantidade;
      setCarrinho(novo);
    } else {
      setCarrinho(prev => [...prev, {
        ...produto,
        quantidade: parseInt(quantidade),
        subtotal:   produto.preco * parseInt(quantidade),
      }]);
    }
    setSelectedProduto('');
    setQuantidade(1);
  };

  const handleRemoverItem = (index) => {
    setCarrinho(prev => prev.filter((_, i) => i !== index));
  };

  const handleConcluirVenda = async () => {
    if (!selectedCliente) {
      mostrarAlerta('warning', 'Selecione um cliente.');
      return;
    }
    if (!selectedFuncionario) {
      mostrarAlerta('warning', 'Selecione o funcionário responsável.');
      return;
    }
    if (carrinho.length === 0) {
      mostrarAlerta('warning', 'Adicione pelo menos um produto.');
      return;
    }

    const payload = {
      clienteId:      parseInt(selectedCliente),
      funcionarioId:  parseInt(selectedFuncionario),
      formaPagamento: formaPagamento,
      instrumentos:   carrinho.map(i => i.id),
      quantidades:    carrinho.map(i => i.quantidade),
    };

    setIsLoading(true);
    try {
      await api.post('/vendas', payload);
      mostrarAlerta('success', 'Venda registrada com sucesso!');
      setTimeout(() => navigate('/vendas'), 1500);
    } catch (err) {
      mostrarAlerta('danger', `Erro ao processar a venda: ${err.message}`);
      setIsLoading(false);
    }
  };

  const totalVenda = carrinho.reduce((acc, item) => acc + item.subtotal, 0);

  return (
    <Container className="mt-4">
      {alerta.show && (
        <Alert variant={alerta.variant} onClose={() => setAlerta({ ...alerta, show: false })} dismissible>
          {alerta.message}
        </Alert>
      )}

      <Row className="mb-3 align-items-center">
        <Col><h2>Nova Venda</h2></Col>
      </Row>

      <Row>
        <Col md={6}>
          <Card className="mb-4">
            <Card.Header>Dados da Venda</Card.Header>
            <Card.Body>
              <Form.Group className="mb-3">
                <Form.Label>Cliente *</Form.Label>
                <Form.Select value={selectedCliente} onChange={e => setSelectedCliente(e.target.value)} disabled={isLoading}>
                  <option value="">Selecione o cliente...</option>
                  {clientes.map(c => <option key={c.id} value={c.id}>{c.nome} — {c.cpf}</option>)}
                </Form.Select>
              </Form.Group>

              <Form.Group className="mb-3">
                <Form.Label>Funcionário Responsável *</Form.Label>
                <Form.Select value={selectedFuncionario} onChange={e => setSelectedFuncionario(e.target.value)} disabled={isLoading}>
                  <option value="">Selecione o funcionário...</option>
                  {funcionarios.map(f => <option key={f.id} value={f.id}>{f.nome}</option>)}
                </Form.Select>
              </Form.Group>

              <Form.Group className="mb-4">
                <Form.Label>Forma de Pagamento *</Form.Label>
                <Form.Select value={formaPagamento} onChange={e => setFormaPagamento(e.target.value)} disabled={isLoading}>
                  {FORMAS_PAGAMENTO.map(f => <option key={f} value={f}>{f.replace('_', ' ')}</option>)}
                </Form.Select>
              </Form.Group>

              <hr />
              <h5 className="mb-3">Adicionar Produto</h5>

              <Form.Group className="mb-3">
                <Form.Label>Produto</Form.Label>
                <Form.Select value={selectedProduto} onChange={e => setSelectedProduto(e.target.value)} disabled={isLoading}>
                  <option value="">Selecione o produto...</option>
                  {produtos.map(p => (
                    <option key={p.id} value={p.id}>
                      {p.nome} — R$ {Number(p.preco).toFixed(2)} (estoque: {p.quantidade})
                    </option>
                  ))}
                </Form.Select>
              </Form.Group>

              <Row>
                <Col md={6}>
                  <Form.Group className="mb-3">
                    <Form.Label>Quantidade</Form.Label>
                    <Form.Control type="number" min="1" value={quantidade}
                      onChange={e => setQuantidade(e.target.value)} disabled={isLoading} />
                  </Form.Group>
                </Col>
                <Col md={6} className="d-flex align-items-end mb-3">
                  <Button variant="primary" className="w-100" onClick={handleAddProduto} disabled={isLoading || !selectedProduto}>
                    + Adicionar
                  </Button>
                </Col>
              </Row>
            </Card.Body>
          </Card>
        </Col>

        <Col md={6}>
          <Card>
            <Card.Header>Resumo da Venda</Card.Header>
            <Card.Body>
              <Table size="sm" responsive>
                <thead>
                  <tr><th>Produto</th><th>Qtd</th><th>Subtotal</th><th></th></tr>
                </thead>
                <tbody>
                  {carrinho.length === 0 ? (
                    <tr><td colSpan="4" className="text-center text-muted py-3">Nenhum produto adicionado</td></tr>
                  ) : (
                    carrinho.map((item, index) => (
                      <tr key={index}>
                        <td className="align-middle">{item.nome}</td>
                        <td className="align-middle">{item.quantidade}</td>
                        <td className="align-middle">R$ {item.subtotal.toFixed(2)}</td>
                        <td className="text-end">
                          <Button variant="outline-danger" size="sm" onClick={() => handleRemoverItem(index)}>✕</Button>
                        </td>
                      </tr>
                    ))
                  )}
                </tbody>
              </Table>

              <h4 className="text-end mt-3 border-top pt-3">
                Total: <span className="text-success">R$ {totalVenda.toFixed(2)}</span>
              </h4>

              <Button
                variant="success"
                size="lg"
                className="w-100 mt-3"
                onClick={handleConcluirVenda}
                disabled={carrinho.length === 0 || !selectedCliente || !selectedFuncionario || isLoading}
              >
                {isLoading ? 'A processar...' : '✔ Concluir Venda'}
              </Button>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default NovaVenda;
