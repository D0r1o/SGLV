# SGLV — Sistema de Gestão de Locação de Veículos

Sistema de gerenciamento de locadora desenvolvido em C, com persistência em arquivos binários, validação de dados, geração de relatórios e log de operações.

Projeto final da disciplina de **Fundamentos de Programação** — ADS · IFSP

---

## Funcionalidades

- **Veículos** — cadastro, listagem, busca por placa e exclusão com proteção contra remoção de veículo alugado
- **Clientes** — cadastro, listagem, busca por nome e exclusão com proteção contra remoção de cliente com locação ativa
- **Locações** — registro de aluguel com cálculo automático de valor total, histórico filtrado por veículo ou cliente, e registro de devolução
- **Relatórios** — exibição em tela de veículos disponíveis e locações ativas; geração automática de arquivos `.txt` após cada operação de escrita
- **Log de auditoria** — registro com timestamp de cada operação realizada (cadastro, exclusão, aluguel, devolução, inicialização e encerramento)
- **Dados de demonstração** — geração automática de 10 veículos, 10 clientes e 8 locações de exemplo na primeira execução

---

## Tecnologias e conceitos aplicados

| Recurso | Detalhe |
|---|---|
| Linguagem | C (padrão C99) |
| Persistência | Arquivos binários via `fwrite`/`fread`; atualização pontual com `fseek` |
| Estruturas de dados | `struct` para Veículo, Cliente e Locação; arrays estáticos com limite configurável |
| Validações | Placa (formato antigo e Mercosul), CNH (11 dígitos), telefone (10–11 dígitos), ano e diária |
| Modularização | Funções separadas por responsabilidade; menus hierárquicos por domínio |
| Log | Arquivo `sistema.log` com timestamp via `localtime()` |
| Relatórios automáticos | `.txt` gerados após cada operação de escrita |
| Compilação | GCC com `-Wall -Wextra` sem warnings |

---

## Como compilar e executar

**Pré-requisito:** GCC instalado.

```bash
# Clonar o repositório
git clone https://github.com/D0r1o/sglv.git
cd sglv

# Compilar
gcc -Wall -Wextra -o sglv sglv.c

# Executar
./sglv
```

Na primeira execução, o sistema detecta a ausência de arquivos `.dat` e gera dados de exemplo automaticamente para demonstração.

---

## Estrutura do projeto

```
sglv/
├── sglv.c                        # Código-fonte principal (único arquivo)
├── veiculos.dat                  # Dados persistidos de veículos (gerado em execução)
├── clientes.dat                  # Dados persistidos de clientes (gerado em execução)
├── locacoes.dat                  # Dados persistidos de locações (gerado em execução)
├── sistema.log                   # Log de auditoria (gerado em execução)
├── relatorio_disponiveis.txt     # Relatório de veículos disponíveis (gerado em execução)
└── relatorio_locacoes_ativas.txt # Relatório de locações ativas (gerado em execução)
```

---

## Arquitetura interna

O código é organizado em camadas funcionais dentro de um único arquivo:

```
main()
├── Inicialização — carrega arquivos .dat ou gera dados de exemplo
│
├── menu_veiculos()
│   ├── cadastrar_veiculo()  →  validar_placa(), validar_ano(), validar_diaria()
│   ├── listar_veiculos()
│   ├── buscar_veiculo()
│   └── excluir_veiculo()    →  proteção: status != alugado
│
├── menu_clientes()
│   ├── cadastrar_cliente()  →  validar_cnh(), validar_telefone()
│   ├── listar_clientes()
│   ├── buscar_cliente()
│   └── excluir_cliente()    →  proteção: sem locação ativa
│
├── menu_locacao()
│   ├── registrar_aluguel()  →  atualiza status do veículo via fseek
│   ├── historico_locacoes() →  filtro por veículo, cliente ou geral
│   └── registrar_devolucao() → libera veículo via fseek
│
└── menu_relatorios()
    ├── relatorio_disponiveis()
    └── relatorio_locacoes_ativas()
```

Cada operação de escrita aciona `gerar_relatorios_automaticos()` e `escrever_log()`.

---

## Autor

**Renan Dorio da Silva**
ADS — IFSP · São Paulo

[![GitHub](https://img.shields.io/badge/GitHub-D0r1o-181717?logo=github)](https://github.com/D0r1o)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Renan%20Dorio-0A66C2?logo=linkedin)](https://www.linkedin.com/in/renan-dorio-0b9b55a5)
