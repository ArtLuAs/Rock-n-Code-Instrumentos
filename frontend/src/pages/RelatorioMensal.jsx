import { useState, useEffect } from 'react';
import {
  Container, Row, Col, Form, Table, Card,
  Alert, Badge, Spinner
} from 'react-bootstrap';
import { useOutletContext } from 'react-router-dom';
import { api } from '../services/api';

const fmt = (v) =>
  Number(v).toLocaleString('pt-BR', { style: 'currency', currency: 'BRL' });

const mesAtual = () => {
  const d = new Date();
  return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}`;
};

const RelatorioMensal = () => {
  const { theme } = useOutletContext();
  const [periodo, setPeriodo]     = useState(mesAtual());
  const [dados, setDados]         = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [erro, setErro]           = useState('');

  const carregar = async () => {
    setIsLoading(true);
    setErro('');
    try {
      const [ano, mes] = periodo.split('-');
      const data = await api.get(`/relatorio/mensal?ano=${ano}&mes=${mes}`);
      setDados(data);
    } catch (e) {
      setErro('Erro ao carregar relat\u00f3rio: ' + e.message);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => { carregar(); }, [periodo]);

  const statusVariant = (s) =>
    s === 'confirmado' ? 'success' : s === 'recusado' ? 'danger' : 'warning';

  return (
    <Container className="mt-4">
      <Row className="mb-3 align-items-center">
        <Col md={6}><h2>Relat\u00f3rio Mensal</h2></Col>
        <Col md={3} className="ms-auto">
          <Form.Group>
            <Form.Label className="fw-bold">M\u00eas / Ano:</Form.Label>
            <Form.Control
              type="month"
              value={periodo}
              onChange={(e) => setPeriodo(e.target.value)}
            />
          </Form.Group>
        </Col>
      </Row>

      {erro && <Alert variant="danger">{erro}</Alert>}

      {isLoading && (
        <div className="text-center py-5">
          <Spinner animation="border" />
        </div>
      )}

      {!isLoading && dados && (
        <>
          {/* Cards de resumo */}
          <Row className="g-3 mb-4">
            <Col md={3} sm={6}>
              <Card bg={theme === 'dark' ? 'dark' : 'light'} className="border text-center h-100">
                <Card.Body>
                  <div className="text-muted small mb-1">Total de Vendas</div>
                  <div className="fs-3 fw-bold">{dados.totalVendas}</div>
                </Card.Body>
              </Card>
            </Col>
            <Col md={3} sm={6}>
              <Card bg={theme === 'dark' ? 'dark' : 'light'} className="border text-center h-100">
                <Card.Body>
                  <div className="text-muted small mb-1">Receita Confirmada</div>
                  <div className="fs-4 fw-bold text-success">{fmt(dados.receitaConfirmada)}</div>
                </Card.Body>
              </Card>
            </Col>
            <Col md={3} sm={6}>
              <Card bg={theme === 'dark' ? 'dark' : 'light'} className="border text-center h-100">
                <Card.Body>
                  <div className="text-muted small mb-1">Receita Pendente</div>
                  <div className="fs-4 fw-bold text-warning">{fmt(dados.receitaPendente)}</div>
                </Card.Body>
              </Card>
            </Col>
            <Col md={3} sm={6}>
              <Card bg={theme === 'dark' ? 'dark' : 'light'} className="border text-center h-100">
                <Card.Body>
                  <div className="text-muted small mb-1">Ticket M\u00e9dio</div>
                  <div className="fs-4 fw-bold">{fmt(dados.ticketMedio)}</div>
                </Card.Body>
              </Card>
            </Col>
          </Row>

          {/* Produto mais vendido */}
          {dados.produtoMaisVendido && (
            <Alert variant="info" className="mb-4">
              \ud83c\udfc6 <strong>Produto mais vendido:</strong>{' '}
              {dados.produtoMaisVendido.nome} \u2014{' '}
              {dados.produtoMaisVendido.quantidade} unidade(s) vendida(s)
            </Alert>
          )}

          <Row className="g-4 mb-4">
            {/* Tabela de vendas */}
            <Col lg={8}>
              <h5 className="mb-3">Vendas do per\u00edodo</h5>
              {dados.vendas.length === 0 ? (
                <Alert variant="secondary">Nenhuma venda neste per\u00edodo.</Alert>
              ) : (
                <Table variant={theme} striped bordered hover responsive>
                  <thead>
                    <tr>
                      <th>#</th>
                      <th>Cliente</th>
                      <th>Data</th>
                      <th>Pagamento</th>
                      <th>Total</th>
                      <th>Status</th>
                    </tr>
                  </thead>
                  <tbody>
                    {dados.vendas.map((v) => (
                      <tr key={v.id}>
                        <td>{v.id}</td>
                        <td>{v.cliente}</td>
                        <td>{v.data}</td>
                        <td>{(v.formaPagamento || '').replace(/_/g, ' ')}</td>
                        <td>{fmt(v.total)}</td>
                        <td>
                          <Badge bg={statusVariant(v.statusPagamento)}>
                            {v.statusPagamento}
                          </Badge>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </Table>
              )}
            </Col>

            {/* Top produtos */}
            <Col lg={4}>
              <h5 className="mb-3">Top produtos</h5>
              {dados.topProdutos.length === 0 ? (
                <Alert variant="secondary">Sem dados.</Alert>
              ) : (
                <Table variant={theme} bordered hover size="sm" responsive>
                  <thead>
                    <tr>
                      <th>Produto</th>
                      <th className="text-center">Qtd</th>
                      <th>Receita</th>
                    </tr>
                  </thead>
                  <tbody>
                    {dados.topProdutos.map((p, i) => (
                      <tr key={i}>
                        <td>{p.nome}</td>
                        <td className="text-center">{p.quantidade}</td>
                        <td>{fmt(p.receita)}</td>
                      </tr>
                    ))}
                  </tbody>
                </Table>
              )}
            </Col>
          </Row>

          {/* Ranking de vendedores (vw_vendas_por_vendedor_mes) */}
          <Row>
            <Col>
              <h5 className="mb-3">\ud83d\udcca Ranking de Vendedores</h5>
              <p className="text-muted small mb-2">
                Gerado a partir da view <code>vw_vendas_por_vendedor_mes</code>.
              </p>
              {dados.rankingVendedores.length === 0 ? (
                <Alert variant="secondary">Nenhum vendedor registrado neste per\u00edodo.</Alert>
              ) : (
                <Table variant={theme} striped bordered hover responsive>
                  <thead>
                    <tr>
                      <th>Posi\u00e7\u00e3o</th>
                      <th>Vendedor</th>
                      <th className="text-center">Pedidos</th>
                      <th>Total Vendido</th>
                    </tr>
                  </thead>
                  <tbody>
                    {dados.rankingVendedores.map((v, i) => (
                      <tr key={i}>
                        <td className="fw-bold">
                          {i === 0 ? '\ud83e\udd47' : i === 1 ? '\ud83e\udd48' : i === 2 ? '\ud83e\udd49' : `${i + 1}\u00ba`}
                        </td>
                        <td>{v.vendedor}</td>
                        <td className="text-center">{v.totalPedidos}</td>
                        <td className="fw-bold text-success">{fmt(v.totalVendido)}</td>
                      </tr>
                    ))}
                  </tbody>
                </Table>
              )}
            </Col>
          </Row>
        </>
      )}
    </Container>
  );
};

export default RelatorioMensal;
