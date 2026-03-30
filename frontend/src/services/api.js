/**
 * ============================================================================
 * TODO: INTEGRAÇÃO COM BACKEND REAL
 * ============================================================================
 * Este ficheiro utiliza funções Mock (simuladas) para permitir o desenvolvimento
 * da interface sem um backend ativo. 
 * * PARA CONECTAR AO SEU BACKEND (C++, Node, Python, Java, etc.):
 * 1. Apague a variável `mockDB` e as lógicas de `setTimeout`.
 * 2. Descomente e adapte as funções utilizando `fetch` ou `axios`.
 * 3. Certifique-se de configurar a variável de ambiente VITE_API_BASE_URL.
 */

// const BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080/api';

// Banco de dados em memória para simular o comportamento da API
const mockDB = {
  clientes: [],
  produtos: [{ id: 1, nome: 'Item de Exemplo', categoria: 'Geral', preco: 99.90, quantidade: 5 }],
  vendas: []
};

// Função para simular o tempo de latência da rede (ex: 800ms)
const delay = (ms = 800) => new Promise(resolve => setTimeout(resolve, ms));

export const api = {
  get: async (endpoint) => {
    await delay();
    const resource = endpoint.replace('/', '');
    return [...(mockDB[resource] || [])];
    
    // EXEMPLO REAL:
    // const response = await fetch(`${BASE_URL}${endpoint}`);
    // return response.json();
  },

  post: async (endpoint, data) => {
    await delay();
    const resource = endpoint.replace('/', '');
    const items = mockDB[resource] || [];
    const newId = items.length > 0 ? Math.max(...items.map(i => i.id)) + 1 : 1;
    const newItem = { ...data, id: newId };
    mockDB[resource].push(newItem);
    return newItem;

    // EXEMPLO REAL:
    // const response = await fetch(`${BASE_URL}${endpoint}`, {
    //   method: 'POST',
    //   headers: { 'Content-Type': 'application/json' },
    //   body: JSON.stringify(data),
    // });
    // return response.json();
  },

  put: async (endpoint, data) => {
    await delay();
    const [, resource, id] = endpoint.split('/');
    const items = mockDB[resource];
    const index = items.findIndex(i => i.id === parseInt(id));
    if (index !== -1) items[index] = { ...data, id: parseInt(id) };
    return items[index];
  },

  delete: async (endpoint) => {
    await delay();
    const [, resource, id] = endpoint.split('/');
    mockDB[resource] = mockDB[resource].filter(i => i.id !== parseInt(id));
    return { success: true };
  }
};