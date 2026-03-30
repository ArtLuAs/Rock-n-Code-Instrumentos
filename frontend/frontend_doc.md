# 🚀 Template Frontend - Projeto de Banco de Dados

Este repositório contém o código base do frontend desenvolvido para os projetos da disciplina de Banco de Dados. A aplicação foi construída utilizando **React e Bootstrap**, focando em uma arquitetura Single Page Application (SPA) para consumir uma API RESTful feita em **C++** (conectada ao **PostgreSQL**).

## 🎯 Objetivo e Filosofia de Desenvolvimento

A ideia principal deste template é **agilidade e pragmatismo**. O foco não é criar a arquitetura frontend mais complexa do mundo, mas sim **entregar algo funcional, rápido e minimamente bonito**. 

Por conta disso, adotamos as seguintes premissas:
* **Uso de IA Liberado:** O uso de Inteligências Artificiais (ChatGPT, Gemini, GitHub Copilot, etc.) é totalmente encorajado para acelerar a criação de componentes, telas e estilizações. O objetivo é ganhar tempo na interface para focar na integração com o banco de dados.
* **Componentes Prontos:** Utilizamos o Bootstrap para não precisarmos escrever CSS do zero.
* **Simplicidade:** O código está estruturado de forma direta, priorizando o funcionamento das operações de CRUD (Create, Read, Update, Delete) via `fetch` nativo do JavaScript.

## 🤝 Colaboração e Adaptação entre Grupos

Este código foi feito para ser compartilhado! A estrutura base (tabelas de listagem, formulários de cadastro, modais) é genérica o suficiente para qualquer escopo.

### 🔄 Contribuindo com o Template Base (Issues e Pull Requests)
Para que este template evolua e ajude todos os grupos da disciplina, a colaboração de todos é muito bem-vinda! Se você ou seu grupo criaram um componente útil (como um modal de confirmação de exclusão) ou corrigiram um bug genérico, compartilhe com o repositório principal.

**Fluxo de contribuição:**
1. **Abra uma Issue:** Antes de começar a codar, crie uma *Issue* descrevendo a melhoria, o bug ou a nova feature. Isso ajuda a segmentar o trabalho e avisa os colegas sobre o que está sendo feito. Caso seja a implementação/debate/sugestão correspondente a uma *Issue* já existente, você deve adiconar comentários na *Issue* e sempre referenciar nos Pull Requests. 
2. **Crie uma Branch:** Trabalhe em uma branch (criada a partir da branch `develop`) separada seguindo um padrão claro (ex: `feature/modal-bonito` ou `fix/erro-no-fetch`).
3. **Faça o Commit:** Faça commits claros, objetivos e, se necessário, use o `git commit --amend` para manter o histórico limpo antes de enviar.
4. **Abra um Pull Request (PR):** Faça o push da sua branch e abra um PR para a `develop`. Descreva brevemente o que foi feito e referencie a Issue original (ex: "Resolve #3").

**OBS.:** Só será feito merge na `main` na conclusão do projeto, procure a branch `develop`

**Padrão de Commits e PRs:**
Não há critérios rígidos e obrigatórios de formatação para os commits ou descrições de Pull Requests neste repositório. A regra de ouro é: **seja claro e simples de entender**. 

No entanto, se você quiser uma sugestão de boas práticas para manter o histórico legível e organizado, recomendamos fortemente o uso do padrão **[Conventional Commits](https://www.conventionalcommits.org/pt-br/v1.0.0/)** (exemplo: `feat: adiciona componente de tabela` ou `fix: corrige quebra de layout no mobile`). Ou algo semelhante a isso...

### 🧩 **Como utilizar no seu grupo:**

**Fluxo de adoção do template:**
1. Faça um clone ou fork deste repositório.
2. Altere os nomes das variáveis e as rotas da API no código para bater com o seu servidor C++.
3. **Adapte a interface ao seu tema:** Se a base tem variáveis como `item` e `preco`, você pode facilmente alterar para a sua realidade. Por exemplo, transformar a listagem genérica em um gerenciador de estoque para uma loja como a *Rock 'n' Code Instrumentos*, uma locadora de carros, ou um sistema de biblioteca.

## Nossos colaboradores:

Time de engenheiros internacionais:

- :cn: Chinês [Xa ti Gip Ti]()
- :de: Alemão [gröek]()
- :ru: Russo [Klávdi]()
- :india: Indiano [Dji Mí Nai]()
- :kr: Coreano [Dip Sik]()
