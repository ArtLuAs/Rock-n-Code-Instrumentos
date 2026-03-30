import { Card, Button, Row, Col } from 'react-bootstrap';

const Home = () => {
  return (
    <div>
      <h1>Dashboard Inicial</h1>
      <Row className="mt-4">
        <Col md={4}>
          <Card>
            <Card.Body>
              <Card.Title>Bem-vindo!</Card.Title>
              <Card.Text>Este é o seu novo template com React + Bootstrap.</Card.Text>
              <Button variant="primary">Explorar</Button>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </div>
  );
};

export default Home;