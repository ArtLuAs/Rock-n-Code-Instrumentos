import { useState, useEffect } from 'react';
import { Container, Row, Col, Form, Button, Table, Card, Alert } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';
import { api } from '../services/api';

const NovaVenda = () => {
  const navigate = useNavigate();

  const [clientes, setClientes] = useState([]);
  const [produtos, setProdutos] = useState([]);
  const [isLoading, setIsLoading] = useState(false);
  const [alerta, setAlerta] = useState({ show: false, variant: '', message: '' });

  const [selectedCliente, setSelectedCliente] = useState('');
  const [selectedProduto, setSelectedProduto] = useState('');
  const [quantidade, setQuantidade] = useState(1);
  const [carrinho, setCarrinho] = useState([]);

  const mostrarAlerta = (variant, message) => {
    setAlerta({ show: true, variant, message });
    setTimeout(() => setAlerta({ show: false, variant: '', message: '' }), 4000);
  };

  useEffect(() => {
    const carregarDadosFormulario = async () => {
      setIsLoading(true);
      try {
        // Busca clientes e produtos ao mesmo tempo
        const [clientesData, produtosData] = await Promise.all([
          api.get('/clientes'),
          api.get('/produtos')
        ]);
        setClientes(clientesData);
        setProdutos(produtosData);
      } catch (error) {
        mostrarAlerta('danger', 'Erro ao carregar os dados para o formulário.');
      } finally {
        setIsLoading(false);
      }
    };
    carregarDadosFormulario();
  }, []);

  const handleAddProduto = () => {
    if (!selectedProduto || quantidade <= 0) return;

    const produto = produtos.find(p => p.id === parseInt(selectedProduto));
    if (produto) {
      const novoItem = {
        ...produto,
        quantidade: parseInt(quantidade),
        subtotal: produto.preco * quantidade
      };
      setCarrinho([...carrinho, novoItem]);
      setSelectedProduto('');
      setQuantidade(1);
    }
  };

  const handleRemoverItem = (index) => {
    const novoCarrinho = [...carrinho];
    novoCarrinho.splice(index, 1);
    setCarrinho(novoCarrinho);
  };

  const handleConcluirVenda = async () => {
    if (!selectedCliente || carrinho.length === 0) {
      mostrarAlerta('warning', 'Selecione um cliente e adicione produtos.');
      return;
    }

    const payloadVenda = {
      clienteId: parseInt(selectedCliente),
      itens: carrinho.map(item => ({ produtoId: item.id, quantidade: item.quantidade })),
      total: totalVenda,
      data: new Date().toLocaleDateString('pt-BR')
    };

    setIsLoading(true);
    try {
      await api.post('/vendas', payloadVenda);
      // Redireciona rapidamente para a lista simulando sucesso
      navigate('/vendas');
    } catch (error) {
      mostrarAlerta('danger', 'Erro ao processar a venda.');
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

      <Row className="mb-3"><Col><h2>Nova Venda</h2></Col></Row>
      
      <Row>
        <Col md={6}>
          <Card className="mb-4">
            <Card.Header>Dados da Encomenda {isLoading && "(A processar...)"}</Card.Header>
            <Card.Body>
              <Form.Group className="mb-4">
                <Form.Label>Cliente</Form.Label>
                <Form.Select value={selectedCliente} onChange={(e) => setSelectedCliente(e.target.value)} disabled={isLoading}>
                  <option value="">Selecione o cliente...</option>
                  {clientes.map(c => (<option key={c.id} value={c.id}>{c.nome}</option>))}
                </Form.Select>
              </Form.Group>
              
              <hr />
              <h5 className="mb-3">Adicionar Produto</h5>
              
              <Form.Group className="mb-3">
                <Form.Label>Produto</Form.Label>
                <Form.Select value={selectedProduto} onChange={(e) => setSelectedProduto(e.target.value)} disabled={isLoading}>
                  <option value="">Selecione o produto...</option>
                  {produtos.map(p => (<option key={p.id} value={p.id}>{p.nome} - R$ {Number(p.preco).toFixed(2)}</option>))}
                </Form.Select>
              </Form.Group>
              
              <Row>
                <Col md={6}>
                  <Form.Group className="mb-3">
                    <Form.Label>Quantidade</Form.Label>
                    <Form.Control type="number" min="1" value={quantidade} onChange={(e) => setQuantidade(e.target.value)} disabled={isLoading} />
                  </Form.Group>
                </Col>
                <Col md={6} className="d-flex align-items-end mb-3">
                  <Button variant="primary" className="w-100" onClick={handleAddProduto} disabled={isLoading}>Adicionar</Button>
                </Col>
              </Row>
            </Card.Body>
          </Card>
        </Col>
        
        <Col md={6}>
          <Card>
            <Card.Header>Resumo da Encomenda</Card.Header>
            <Card.Body>
              <Table size="sm" responsive>
                <thead><tr><th>Produto</th><th>Qtd</th><th>Subtotal</th><th></th></tr></thead>
                <tbody>
                  {carrinho.map((item, index) => (
                    <tr key={index}>
                      <td className="align-middle">{item.nome}</td><td className="align-middle">{item.quantidade}</td>
                      <td className="align-middle">R$ {item.subtotal.toFixed(2)}</td>
                      <td className="text-end"><Button variant="danger" size="sm" onClick={() => handleRemoverItem(index)}>X</Button></td>
                    </tr>
                  ))}
                  {carrinho.length === 0 && (<tr><td colSpan="4" className="text-center text-muted py-3">Nenhum produto adicionado</td></tr>)}
                </tbody>
              </Table>
              
              <h4 className="text-end mt-4 border-top pt-3">Total: R$ {totalVenda.toFixed(2)}</h4>
              
              <Button 
                variant="success" 
                size="lg" 
                className="w-100 mt-3" 
                onClick={handleConcluirVenda} 
                disabled={carrinho.length === 0 || !selectedCliente || isLoading}
              >
                {isLoading ? 'A Processar...' : 'Concluir Venda'}
              </Button>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default NovaVenda;