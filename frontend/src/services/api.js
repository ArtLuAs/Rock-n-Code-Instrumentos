const BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080/api';

const handleResponse = async (response) => {
  if (!response.ok) {
    const error = await response.json().catch(() => ({ error: response.statusText }));
    throw new Error(error.error || `Erro HTTP ${response.status}`);
  }
  return response.json();
};

export const api = {
  get: (endpoint) =>
    fetch(`${BASE_URL}${endpoint}`).then(handleResponse),

  post: (endpoint, data) =>
    fetch(`${BASE_URL}${endpoint}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }).then(handleResponse),

  put: (endpoint, data) =>
    fetch(`${BASE_URL}${endpoint}`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }).then(handleResponse),

  delete: (endpoint) =>
    fetch(`${BASE_URL}${endpoint}`, {
      method: 'DELETE',
    }).then(handleResponse),
};
