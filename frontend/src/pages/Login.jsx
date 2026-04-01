import { useState, useEffect } from 'react';
import { Container, Row, Col, Card, Form, Button, Alert, Tab, Tabs, Badge } from 'react-bootstrap';
import { useNavigate } from 'react-router-dom';
import { api } from '../services/api';

const SEXOS = ['Masculino', 'Feminino', 'Outro'];
const CARGOS = ['vendedor', 'gerente', 'caixa'];

const REGISTRO_VAZIO = {
  nome: '', cpf: '', senha: '', confirmarSenha: '',
  telefone: '', email: '', sexo: '', cidade: '',
  torceFlamengo: false, assisteOnePiece: false,
  // campos exclusivos de funcionário
  cargo: 'vendedor',
};

const Login = () => {
  const navigate = useNavigate();
  const theme = localStorage.getItem('app-theme') || 'light';

  // --- Aba ativa: 'entrar' | 'registrar' ---
  const [aba, setAba] = useState('entrar');

  // --- Login ---
  const [login, setLogin] = useState({ cpf: '', senha: '', tipo: 'funcionario' });
  const [erroLogin, setErroLogin] = useState('');
  const [loadingLogin, setLoadingLogin] = useState(false);

  // --- Registro ---
  const [reg, setReg] = useState(REGISTRO_VAZIO);
  const [isFuncionario, setIsFuncionario] = useState(false);
  const [erroReg, setErroReg] = useState('');
  const [sucessoReg, setSucessoReg] = useState('');
  const [loadingReg, setLoadingReg] = useState(false);

  useEffect(() => {
    if (localStorage.getItem('auth-token') === 'true') navigate('/');
  }, [navigate]);

  // ---------- LOGIN ----------
  const handleLoginChange = (e) => {
    const { name, value } = e.target;
    setLogin(prev => ({ ...prev, [name]: value }));
  };

  const handleLogin = async (e) => {
    e.preventDefault();
    setErroLogin('');
    setLoadingLogin(true);
    try {
      const data = await api.post('/login', {
        cpf: login.cpf,
        senha: login.senha,
        tipo: login.tipo,
      });
      localStorage.setItem('auth-token', 'true');
      localStorage.setItem('auth-user', data.nome);
      localStorage.setItem('auth-role', data.role);
      localStorage.setItem('auth-id', String(data.id));
      navigate('/');
    } catch {
      setErroLogin('CPF ou senha incorretos.');
    } finally {
      setLoadingLogin(false);
    }
  };

  // ---------- REGISTRO ----------
  const handleRegChange = (e) => {
    const { name, value, type, checked } = e.target;
    setReg(prev => ({ ...prev, [name]: type === 'checkbox' ? checked : value }));
  };

  const handleRegistro = async (e) => {
    e.preventDefault();
    setErroReg('');
    setSucessoReg('');

    if (!reg.nome || !reg.cpf || !reg.senha) {
      setErroReg('Nome, CPF e senha são obrigatórios.');
      return;
    }
    if (reg.senha !== reg.confirmarSenha) {
      setErroReg('As senhas não coincidem.');
      return;
    }

    setLoadingReg(true);
    try {
      const endpoint = isFuncionario ? '/registro/funcionario' : '/registro/cliente';
      await api.post(endpoint, reg);
      setSucessoReg('Conta criada com sucesso! Faça login.');
      setReg(REGISTRO_VAZIO);
      setTimeout(() => { setSucessoReg(''); setAba('entrar'); }, 2000);
    } catch (err) {
      setErroReg(err.message?.includes('409') ? 'CPF já cadastrado.' : `Erro: ${err.message}`);
    } finally {
      setLoadingReg(false);
    }
  };

  const cardBg = theme === 'dark' ? 'bg-dark text-light' : 'bg-white';
  const inputCls = theme === 'dark' ? 'bg-dark text-light border-secondary' : '';

  return (
    <Container fluid className="d-flex align-items-center justify-content-center py-5"
      style={{ minHeight: '100vh', backgroundColor: theme === 'dark' ? '#212529' : '#f8f9fa' }}>
      <Row className="w-100">
        <Col md={{ span: 8, offset: 2 }} lg={{ span: 6, offset: 3 }}>
          <Card className={`shadow-lg border-0 ${cardBg}`}>
            <Card.Header className="text-center bg-primary text-white py-4 border-0">
              <h3 className="mb-0 fw-bold">🎸 Rock 'n' Code</h3>
              <p className="mb-0 text-white-50">Instrumentos Musicais</p>
            </Card.Header>
            <Card.Body className="p-4">
              <Tabs activeKey={aba} onSelect={setAba} className="mb-4" fill>

                {/* ===== ABA ENTRAR ===== */}
                <Tab eventKey="entrar" title="Entrar">
                  {erroLogin && <Alert variant="danger">{erroLogin}</Alert>}
                  <Form onSubmit={handleLogin}>
                    <Form.Group className="mb-3">
                      <Form.Label>Tipo de usuário</Form.Label>
                      <Form.Select name="tipo" value={login.tipo} onChange={handleLoginChange} className={inputCls}>
                        <option value="funcionario">Funcionário</option>
                        <option value="cliente">Cliente</option>
                      </Form.Select>
                    </Form.Group>
                    <Form.Group className="mb-3">
                      <Form.Label>CPF</Form.Label>
                      <Form.Control type="text" name="cpf" placeholder="Somente números"
                        value={login.cpf} onChange={handleLoginChange} required className={inputCls} />
                    </Form.Group>
                    <Form.Group className="mb-4">
                      <Form.Label>Senha</Form.Label>
                      <Form.Control type="password" name="senha" placeholder="Sua senha"
                        value={login.senha} onChange={handleLoginChange} required className={inputCls} />
                    </Form.Group>
                    <Button variant="primary" type="submit" className="w-100 py-2 fw-bold" disabled={loadingLogin}>
                      {loadingLogin ? 'Autenticando...' : 'Entrar'}
                    </Button>
                  </Form>
                </Tab>

                {/* ===== ABA REGISTRAR ===== */}
                <Tab eventKey="registrar" title="Criar Conta">
                  {erroReg    && <Alert variant="danger">{erroReg}</Alert>}
                  {sucessoReg && <Alert variant="success">{sucessoReg}</Alert>}

                  <div className="d-flex align-items-center justify-content-between mb-3 p-2 rounded"
                    style={{ backgroundColor: theme === 'dark' ? '#343a40' : '#e9ecef' }}>
                    <span className="fw-bold">
                      {isFuncionario
                        ? <><Badge bg="warning" text="dark">Funcionário</Badge> Registrar como funcionário</>
                        : <><Badge bg="success">Cliente</Badge> Registrar como cliente</>}
                    </span>
                    <Form.Check type="switch" id="tipo-switch"
                      label="Sou funcionário"
                      checked={isFuncionario}
                      onChange={(e) => setIsFuncionario(e.target.checked)} />
                  </div>

                  <Form onSubmit={handleRegistro}>
                    {/* Campos comuns */}
                    <Form.Group className="mb-3">
                      <Form.Label>Nome completo *</Form.Label>
                      <Form.Control type="text" name="nome" value={reg.nome} onChange={handleRegChange} className={inputCls} />
                    </Form.Group>
                    <Row>
                      <Col md={6}>
                        <Form.Group className="mb-3">
                          <Form.Label>CPF * <small className="text-muted">(somente números)</small></Form.Label>
                          <Form.Control type="text" name="cpf" value={reg.cpf} onChange={handleRegChange} className={inputCls} />
                        </Form.Group>
                      </Col>
                      <Col md={6}>
                        <Form.Group className="mb-3">
                          <Form.Label>Telefone</Form.Label>
                          <Form.Control type="text" name="telefone" value={reg.telefone} onChange={handleRegChange} className={inputCls} />
                        </Form.Group>
                      </Col>
                    </Row>
                    <Form.Group className="mb-3">
                      <Form.Label>E-mail</Form.Label>
                      <Form.Control type="email" name="email" value={reg.email} onChange={handleRegChange} className={inputCls} />
                    </Form.Group>

                    {/* Campos exclusivos de CLIENTE */}
                    {!isFuncionario && (
                      <>
                        <Row>
                          <Col md={6}>
                            <Form.Group className="mb-3">
                              <Form.Label>Sexo</Form.Label>
                              <Form.Select name="sexo" value={reg.sexo} onChange={handleRegChange} className={inputCls}>
                                <option value="">Selecione...</option>
                                {SEXOS.map(s => <option key={s} value={s}>{s}</option>)}
                              </Form.Select>
                            </Form.Group>
                          </Col>
                          <Col md={6}>
                            <Form.Group className="mb-3">
                              <Form.Label>Cidade</Form.Label>
                              <Form.Control type="text" name="cidade" value={reg.cidade} onChange={handleRegChange} className={inputCls} />
                            </Form.Group>
                          </Col>
                        </Row>
                        <div className="mb-3 d-flex flex-column gap-2">
                          <Form.Check type="switch" id="reg-flamengo"
                            label="🔴⚫ Torce para o Flamengo (10% de desconto)"
                            name="torceFlamengo" checked={reg.torceFlamengo} onChange={handleRegChange} />
                          <Form.Check type="switch" id="reg-onepiece"
                            label="🏴‍☠️Assiste One Piece (10% de desconto)"
                            name="assisteOnePiece" checked={reg.assisteOnePiece} onChange={handleRegChange} />
                        </div>
                      </>
                    )}

                    {/* Campos exclusivos de FUNCIONÁRIO */}
                    {isFuncionario && (
                      <Form.Group className="mb-3">
                        <Form.Label>Cargo</Form.Label>
                        <Form.Select name="cargo" value={reg.cargo} onChange={handleRegChange} className={inputCls}>
                          {CARGOS.map(c => <option key={c} value={c}>{c}</option>)}
                        </Form.Select>
                      </Form.Group>
                    )}

                    {/* Senha */}
                    <Row>
                      <Col md={6}>
                        <Form.Group className="mb-3">
                          <Form.Label>Senha *</Form.Label>
                          <Form.Control type="password" name="senha" value={reg.senha} onChange={handleRegChange} className={inputCls} />
                        </Form.Group>
                      </Col>
                      <Col md={6}>
                        <Form.Group className="mb-3">
                          <Form.Label>Confirmar Senha *</Form.Label>
                          <Form.Control type="password" name="confirmarSenha" value={reg.confirmarSenha} onChange={handleRegChange} className={inputCls} />
                        </Form.Group>
                      </Col>
                    </Row>

                    <Button variant={isFuncionario ? 'warning' : 'success'} type="submit"
                      className="w-100 py-2 fw-bold" disabled={loadingReg}>
                      {loadingReg ? 'Criando conta...' : `Criar conta de ${isFuncionario ? 'funcionário' : 'cliente'}`}
                    </Button>
                  </Form>
                </Tab>
              </Tabs>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </Container>
  );
};

export default Login;
