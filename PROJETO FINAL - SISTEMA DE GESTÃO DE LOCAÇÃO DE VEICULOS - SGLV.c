/*
 * ============================================================
 * Sistema de Gestão de Locadora de Veículos
 * Aluno: Renan Dorio da Silva
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// ============================================================
// CONSTANTES
// ============================================================
#define MAX_VEICULOS 100
#define MAX_CLIENTES 100
#define MAX_LOCACOES 100
#define TAM_PLACA 8
#define TAM_MODELO 30
#define TAM_MARCA 30
#define TAM_NOME 50
#define TAM_CNH 12
#define TAM_TELEFONE 15

#define ARQ_VEICULOS "veiculos.dat"
#define ARQ_CLIENTES "clientes.dat"
#define ARQ_LOCACOES "locacoes.dat"
#define ARQ_LOG "sistema.log"

// ============================================================
// STRUCTS
// ============================================================

// Veiculo: placa, modelo, marca, ano, diária e status (0=livre, 1=alugado)
typedef struct
{
    int id;
    char placa[TAM_PLACA];
    char modelo[TAM_MODELO];
    char marca[TAM_MARCA];
    int ano;
    float diaria;
    int status;
} Veiculo;

// Cliente: nome, CNH e telefone
typedef struct
{
    int id;
    char nome[TAM_NOME];
    char cnh[TAM_CNH];
    char telefone[TAM_TELEFONE];
} Cliente;

// Locacao: vincula veículo e cliente, com dias, valor total e flag ativa
typedef struct
{
    int id;
    int id_veiculo;
    int id_cliente;
    int dias;
    float valor_total;
    int ativa;
} Locacao;

// ============================================================
// VARIAVEIS GLOBAIS
// ============================================================
Veiculo veiculos[MAX_VEICULOS];
Cliente clientes[MAX_CLIENTES];
Locacao locacoes[MAX_LOCACOES];
int total_veiculos = 0;
int total_clientes = 0;
int total_locacoes = 0;

// ============================================================
// FUNCAO DE LOG
// ============================================================

// Escreve uma mensagem no arquivo de log com timestamp
void escrever_log(const char *mensagem)
{
    FILE *log = fopen(ARQ_LOG, "a");
    if (!log)
        return;
    time_t agora = time(NULL);
    struct tm *tm_local = localtime(&agora);
    fprintf(log, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            tm_local->tm_year + 1900,
            tm_local->tm_mon + 1,
            tm_local->tm_mday,
            tm_local->tm_hour,
            tm_local->tm_min,
            tm_local->tm_sec,
            mensagem);
    fclose(log);
}

// ============================================================
// FUNCOES AUXILIARES
// ============================================================

// Limpa o buffer do teclado após scanf
void limpar_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

// Lê uma string com fgets, exibe um prompt e remove a quebra de linha final
int ler_string(const char *prompt, char *destino, int tamanho)
{
    printf("%s", prompt);
    if (fgets(destino, tamanho, stdin) == NULL)
        return 0;
    size_t len = strlen(destino);
    if (len > 0 && destino[len - 1] == '\n')
        destino[len - 1] = '\0';
    return 1;
}

// Pergunta ao usuário se ele confirma a operação; retorna 1 para 's' ou 'S'
int confirmar(const char *mensagem)
{
    char resp;
    printf("%s (s/n): ", mensagem);
    scanf(" %c", &resp);
    limpar_buffer();
    return (resp == 's' || resp == 'S');
}

// ============================================================
// VALIDACOES
// ============================================================

// Placa: 3 letras + 4 alfanuméricos (formato antigo ou Mercosul)
int validar_placa(const char *placa)
{
    int len = strlen(placa);
    if (len != 7)
        return 0;
    for (int i = 0; i < 3; i++)
        if (!isalpha(placa[i]))
            return 0;
    for (int i = 3; i < 7; i++)
        if (!isalnum(placa[i]))
            return 0;
    return 1;
}

// CNH: exatamente 11 dígitos numéricos
int validar_cnh(const char *cnh)
{
    if (strlen(cnh) != 11)
        return 0;
    for (int i = 0; i < 11; i++)
        if (!isdigit(cnh[i]))
            return 0;
    return 1;
}

// Telefone: 10 ou 11 dígitos (com DDD)
int validar_telefone(const char *tel)
{
    int len = strlen(tel);
    if (len != 10 && len != 11)
        return 0;
    for (int i = 0; i < len; i++)
        if (!isdigit(tel[i]))
            return 0;
    return 1;
}

// Ano: entre 1900 e o ano atual
int validar_ano(int ano)
{
    time_t t = time(NULL);
    struct tm *atual = localtime(&t);
    int ano_atual = atual->tm_year + 1900;
    return (ano >= 1900 && ano <= ano_atual);
}

// Diária: deve ser maior que zero
int validar_diaria(float valor)
{
    return (valor > 0.0);
}

// ============================================================
// PERSISTENCIA EM ARQUIVO BINARIO
// ============================================================

// Salva todo o vetor de veículos
int salvar_veiculos()
{
    FILE *arq = fopen(ARQ_VEICULOS, "wb");
    if (!arq)
        return -1;
    fwrite(&total_veiculos, sizeof(int), 1, arq);
    fwrite(veiculos, sizeof(Veiculo), total_veiculos, arq);
    fclose(arq);
    return 0;
}

// Salva todo o vetor de clientes
int salvar_clientes()
{
    FILE *arq = fopen(ARQ_CLIENTES, "wb");
    if (!arq)
        return -1;
    fwrite(&total_clientes, sizeof(int), 1, arq);
    fwrite(clientes, sizeof(Cliente), total_clientes, arq);
    fclose(arq);
    return 0;
}

// Salva todo o vetor de locações
int salvar_locacoes()
{
    FILE *arq = fopen(ARQ_LOCACOES, "wb");
    if (!arq)
        return -1;
    fwrite(&total_locacoes, sizeof(int), 1, arq);
    fwrite(locacoes, sizeof(Locacao), total_locacoes, arq);
    fclose(arq);
    return 0;
}

// Carrega o vetor de veículos; se arquivo não existir, retorna -1
int carregar_veiculos()
{
    FILE *arq = fopen(ARQ_VEICULOS, "rb");
    if (!arq)
        return -1;
    fread(&total_veiculos, sizeof(int), 1, arq);
    if (total_veiculos > MAX_VEICULOS)
        total_veiculos = MAX_VEICULOS;
    fread(veiculos, sizeof(Veiculo), total_veiculos, arq);
    fclose(arq);
    return 0;
}

// Carrega o vetor de clientes; se arquivo não existir, retorna -1
int carregar_clientes()
{
    FILE *arq = fopen(ARQ_CLIENTES, "rb");
    if (!arq)
        return -1;
    fread(&total_clientes, sizeof(int), 1, arq);
    if (total_clientes > MAX_CLIENTES)
        total_clientes = MAX_CLIENTES;
    fread(clientes, sizeof(Cliente), total_clientes, arq);
    fclose(arq);
    return 0;
}

// Carrega o vetor de locações; se arquivo não existir, retorna -1
int carregar_locacoes()
{
    FILE *arq = fopen(ARQ_LOCACOES, "rb");
    if (!arq)
        return -1;
    fread(&total_locacoes, sizeof(int), 1, arq);
    if (total_locacoes > MAX_LOCACOES)
        total_locacoes = MAX_LOCACOES;
    fread(locacoes, sizeof(Locacao), total_locacoes, arq);
    fclose(arq);
    return 0;
}

// ============================================================
// ATUALIZACAO PONTUAL COM FSEEK
// ============================================================

// Atualiza um veículo específico no arquivo (pelo ID)
int atualizar_veiculo_no_arquivo(int id, Veiculo *v)
{
    FILE *arq = fopen(ARQ_VEICULOS, "r+b");
    if (!arq)
        return -1;
    int total;
    fread(&total, sizeof(int), 1, arq);
    long pos = ftell(arq);
    fseek(arq, pos + (id - 1) * sizeof(Veiculo), SEEK_SET);
    fwrite(v, sizeof(Veiculo), 1, arq);
    fclose(arq);
    return 0;
}

// Atualiza uma locação específica no arquivo (pelo ID)
int atualizar_locacao_no_arquivo(int id, Locacao *l)
{
    FILE *arq = fopen(ARQ_LOCACOES, "r+b");
    if (!arq)
        return -1;
    int total;
    fread(&total, sizeof(int), 1, arq);
    long pos = ftell(arq);
    fseek(arq, pos + (id - 1) * sizeof(Locacao), SEEK_SET);
    fwrite(l, sizeof(Locacao), 1, arq);
    fclose(arq);
    return 0;
}

// ============================================================
// RELATORIOS AUTOMATICOS (arquivos .txt)
// ============================================================

// Gera arquivo com a lista de veículos disponíveis
void gerar_relatorio_disponiveis()
{
    FILE *arq = fopen("relatorio_disponiveis.txt", "w");
    if (!arq)
        return;
    time_t agora = time(NULL);
    fprintf(arq, "=== VEICULOS DISPONIVEIS ===\n");
    fprintf(arq, "Gerado em: %s", ctime(&agora));
    int count = 0;
    for (int i = 0; i < total_veiculos; i++)
    {
        if (veiculos[i].status == 0)
        {
            fprintf(arq, "V(%d) | %s | %s %s | Ano: %d | Diaria: R$%.2f\n",
                    veiculos[i].id, veiculos[i].placa, veiculos[i].marca,
                    veiculos[i].modelo, veiculos[i].ano, veiculos[i].diaria);
            count++;
        }
    }
    if (count == 0)
        fprintf(arq, "Nenhum veiculo disponivel no momento.\n");
    fclose(arq);
}

// Gera arquivo com a lista de locações ainda ativas
void gerar_relatorio_locacoes_ativas()
{
    FILE *arq = fopen("relatorio_locacoes_ativas.txt", "w");
    if (!arq)
        return;
    time_t agora = time(NULL);
    fprintf(arq, "=== LOCACOES ATIVAS ===\n");
    fprintf(arq, "Gerado em: %s", ctime(&agora));
    int count = 0;
    for (int i = 0; i < total_locacoes; i++)
    {
        if (locacoes[i].ativa == 1)
        {
            char placa_v[TAM_PLACA] = "?";
            char nome_c[TAM_NOME] = "?";
            for (int j = 0; j < total_veiculos; j++)
            {
                if (veiculos[j].id == locacoes[i].id_veiculo)
                {
                    strcpy(placa_v, veiculos[j].placa);
                    break;
                }
            }
            for (int j = 0; j < total_clientes; j++)
            {
                if (clientes[j].id == locacoes[i].id_cliente)
                {
                    strcpy(nome_c, clientes[j].nome);
                    break;
                }
            }
            fprintf(arq, "L(%d) | Veiculo: %s | Cliente: %s | Dias: %d | Total: R$%.2f\n",
                    locacoes[i].id, placa_v, nome_c, locacoes[i].dias, locacoes[i].valor_total);
            count++;
        }
    }
    if (count == 0)
        fprintf(arq, "Nenhuma locacao ativa no momento.\n");
    fclose(arq);
}

// Chama as duas funções acima (usado após cada operação de escrita)
void gerar_relatorios_automaticos()
{
    gerar_relatorio_disponiveis();
    gerar_relatorio_locacoes_ativas();
}

// ============================================================
// BUSCAS – retornam o índice nos arrays
// ============================================================

int buscar_indice_veiculo(const char *placa)
{
    for (int i = 0; i < total_veiculos; i++)
    {
        if (strcmp(placa, veiculos[i].placa) == 0)
            return i;
    }
    return -1;
}

int buscar_indice_cliente(const char *nome)
{
    for (int i = 0; i < total_clientes; i++)
    {
        if (strcmp(nome, clientes[i].nome) == 0)
            return i;
    }
    return -1;
}

int buscar_indice_locacao(int id_loc)
{
    for (int i = 0; i < total_locacoes; i++)
    {
        if (locacoes[i].id == id_loc)
            return i;
    }
    return -1;
}

// ============================================================
// MÓDULO DE TESTES – gera dados de exemplo (sempre disponível)
// ============================================================

// Gera dados fixos de exemplo para testar relatórios e funcionalidades
void gerar_dados_teste()
{
    // 10 veículos
    Veiculo v_exemplo[10] = {
        {1, "ABC1A23", "Civic", "Honda", 2022, 150.00, 0},
        {2, "XYZ9B87", "Uno", "Fiat", 2020, 80.00, 1},
        {3, "DEF4C56", "Corolla", "Toyota", 2023, 180.00, 0},
        {4, "GHI7D89", "Onix", "Chevrolet", 2021, 120.00, 1},
        {5, "JKL2E34", "HB20", "Hyundai", 2022, 130.00, 0},
        {6, "MNO5F67", "Gol", "Volkswagen", 2019, 90.00, 1},
        {7, "PQR8G01", "Argo", "Fiat", 2021, 110.00, 0},
        {8, "STU3H45", "Cruze", "Chevrolet", 2023, 160.00, 0},
        {9, "VWX6I78", "Sentra", "Nissan", 2020, 140.00, 1},
        {10, "YZA9J02", "Kicks", "Nissan", 2022, 135.00, 0}};
    total_veiculos = 10;
    for (int i = 0; i < total_veiculos; i++)
    {
        veiculos[i] = v_exemplo[i];
    }

    // 10 clientes
    Cliente c_exemplo[10] = {
        {1, "Ana Carolina Souza", "12345678901", "11987654321"},
        {2, "Bruno Oliveira", "98765432100", "11876543210"},
        {3, "Carla Mendes", "45678912345", "11912345678"},
        {4, "Daniel Pereira", "78912345678", "11823456789"},
        {5, "Eduarda Lima", "32165498700", "11934567890"},
        {6, "Felipe Santos", "65498732100", "11845678901"},
        {7, "Gabriela Rocha", "14725836900", "11956789012"},
        {8, "Henrique Alves", "25836914700", "11867890123"},
        {9, "Isabela Ferreira", "36914725800", "11978901234"},
        {10, "João Paulo Costa", "74185296300", "11889012345"}};
    total_clientes = 10;
    for (int i = 0; i < total_clientes; i++)
    {
        clientes[i] = c_exemplo[i];
    }

    // 8 locações (algumas ativas, outras devolvidas)
    Locacao l_exemplo[8] = {
        {1, 2, 1, 5, 400.00, 0},
        {2, 4, 3, 3, 360.00, 0},
        {3, 6, 5, 7, 630.00, 1},
        {4, 9, 2, 2, 280.00, 1},
        {5, 1, 4, 4, 600.00, 0},
        {6, 3, 6, 6, 1080.00, 1},
        {7, 7, 8, 3, 330.00, 0},
        {8, 10, 10, 10, 1350.00, 1}};
    total_locacoes = 8;
    for (int i = 0; i < total_locacoes; i++)
    {
        locacoes[i] = l_exemplo[i];
    }

    // Salva nos arquivos
    salvar_veiculos();
    salvar_clientes();
    salvar_locacoes();
}

// ============================================================
// MODULO VEICULOS
// ============================================================

// Cadastra um novo veículo com validações e confirmação
int cadastrar_veiculo()
{
    if (total_veiculos >= MAX_VEICULOS)
    {
        printf("Limite de veiculos atingido.\n");
        return -1;
    }
    Veiculo v;
    v.id = total_veiculos + 1;

    printf("\n-- Cadastrar Veiculo --\n");
    do
    {
        printf("Placa (7 caracteres, ex: ABC1234): ");
        scanf("%s", v.placa);
        limpar_buffer();
        if (!validar_placa(v.placa))
        {
            printf("Placa invalida. Deve ter 3 letras seguidas de 4 alfanumericos.\n");
        }
    } while (!validar_placa(v.placa));

    ler_string("Modelo: ", v.modelo, TAM_MODELO);
    ler_string("Marca: ", v.marca, TAM_MARCA);

    do
    {
        printf("Ano: ");
        if (scanf("%d", &v.ano) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            v.ano = 0;
        }
        limpar_buffer();
        if (!validar_ano(v.ano))
        {
            int ano_atual;
            time_t t = time(NULL);
            struct tm *atual = localtime(&t);
            ano_atual = atual->tm_year + 1900;
            printf("Ano invalido. Deve estar entre 1900 e %d.\n", ano_atual);
        }
    } while (!validar_ano(v.ano));

    do
    {
        printf("Valor da diaria: ");
        if (scanf("%f", &v.diaria) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            v.diaria = 0;
        }
        limpar_buffer();
        if (!validar_diaria(v.diaria))
        {
            printf("Diaria deve ser um valor positivo.\n");
        }
    } while (!validar_diaria(v.diaria));

    v.status = 0;

    printf("\n--- Resumo do cadastro ---\n");
    printf("ID: V(%d)\n", v.id);
    printf("Placa: %s\n", v.placa);
    printf("Modelo: %s\n", v.modelo);
    printf("Marca: %s\n", v.marca);
    printf("Ano: %d\n", v.ano);
    printf("Diaria: R$%.2f\n", v.diaria);
    printf("Status: Disponivel\n");

    if (!confirmar("Confirma o cadastro deste veiculo?"))
    {
        printf("Cadastro cancelado.\n");
        return -1;
    }

    veiculos[total_veiculos++] = v;
    if (salvar_veiculos() != 0)
    {
        printf("Erro ao salvar veiculo.\n");
        return -1;
    }
    gerar_relatorios_automaticos();
    printf("Veiculo V(%d) cadastrado com sucesso.\n", v.id);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Veiculo cadastrado: V(%d) - %s %s (Placa: %s)",
             v.id, v.marca, v.modelo, v.placa);
    escrever_log(log_msg);
    return 0;
}

// Lista todos os veículos com seus dados
void listar_veiculos()
{
    printf("\n-- Lista de Veiculos --\n");
    if (total_veiculos == 0)
    {
        printf("Nenhum veiculo cadastrado.\n");
        return;
    }
    for (int i = 0; i < total_veiculos; i++)
    {
        printf("V(%d) | %s | %s %s | Ano: %d | Diaria: R$%.2f | Status: %s\n",
               veiculos[i].id, veiculos[i].placa, veiculos[i].marca, veiculos[i].modelo,
               veiculos[i].ano, veiculos[i].diaria,
               veiculos[i].status == 0 ? "Disponivel" : "Alugado");
    }
}

// Busca veículo pela placa e exibe seus dados
void buscar_veiculo()
{
    char placa[TAM_PLACA];
    printf("\nPlaca para busca: ");
    scanf("%s", placa);
    limpar_buffer();
    int idx = buscar_indice_veiculo(placa);
    if (idx == -1)
    {
        printf("Veiculo nao encontrado.\n");
        return;
    }
    printf("Encontrado -> V(%d) | %s | %s %s | Ano: %d | Diaria: R$%.2f | Status: %s\n",
           veiculos[idx].id, veiculos[idx].placa, veiculos[idx].marca, veiculos[idx].modelo,
           veiculos[idx].ano, veiculos[idx].diaria,
           veiculos[idx].status == 0 ? "Disponivel" : "Alugado");
}

// Exclui veículo (apenas se não estiver alugado), com confirmação
int excluir_veiculo()
{
    char placa[TAM_PLACA];
    printf("\nPlaca do veiculo a excluir: ");
    scanf("%s", placa);
    limpar_buffer();

    int idx = buscar_indice_veiculo(placa);
    if (idx == -1)
    {
        printf("Veiculo nao encontrado.\n");
        return -1;
    }

    if (veiculos[idx].status == 1)
    {
        printf("Veiculo esta alugado. Nao pode ser excluido.\n");
        return -1;
    }

    printf("\n--- Veiculo a ser excluido ---\n");
    printf("ID: V(%d)\n", veiculos[idx].id);
    printf("Placa: %s\n", veiculos[idx].placa);
    printf("Modelo: %s\n", veiculos[idx].modelo);
    printf("Marca: %s\n", veiculos[idx].marca);
    printf("Ano: %d\n", veiculos[idx].ano);
    printf("Diaria: R$%.2f\n", veiculos[idx].diaria);
    printf("Status: %s\n", veiculos[idx].status == 0 ? "Disponivel" : "Alugado");

    if (!confirmar("Confirma a exclusao deste veiculo?"))
    {
        printf("Exclusao cancelada.\n");
        return -1;
    }

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Veiculo excluido: V(%d) - %s %s (Placa: %s)",
             veiculos[idx].id, veiculos[idx].marca, veiculos[idx].modelo, veiculos[idx].placa);

    for (int j = idx; j < total_veiculos - 1; j++)
    {
        veiculos[j] = veiculos[j + 1];
    }
    total_veiculos--;
    if (salvar_veiculos() != 0)
    {
        printf("Erro ao salvar alteracoes.\n");
        return -1;
    }
    gerar_relatorios_automaticos();
    printf("Veiculo excluido com sucesso.\n");
    escrever_log(log_msg);
    return 0;
}

// Menu do módulo de veículos
void menu_veiculos()
{
    int opcao;
    do
    {
        printf("\n=== VEICULOS ===\n");
        printf("1. Cadastrar\n2. Listar\n3. Buscar\n4. Excluir\n5. Voltar\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        switch (opcao)
        {
        case 1:
            cadastrar_veiculo();
            break;
        case 2:
            listar_veiculos();
            break;
        case 3:
            buscar_veiculo();
            break;
        case 4:
            excluir_veiculo();
            break;
        case 5:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opcao != 5);
}

// ============================================================
// MODULO CLIENTES
// ============================================================

// Cadastra um novo cliente com validações e confirmação
int cadastrar_cliente()
{
    if (total_clientes >= MAX_CLIENTES)
    {
        printf("Limite de clientes atingido.\n");
        return -1;
    }
    Cliente c;
    c.id = total_clientes + 1;

    printf("\n-- Cadastrar Cliente --\n");
    ler_string("Nome: ", c.nome, TAM_NOME);

    do
    {
        printf("CNH (11 digitos): ");
        scanf("%s", c.cnh);
        limpar_buffer();
        if (!validar_cnh(c.cnh))
            printf("CNH invalida. Deve ter 11 digitos numericos.\n");
    } while (!validar_cnh(c.cnh));

    do
    {
        printf("Telefone (10 ou 11 digitos): ");
        scanf("%s", c.telefone);
        limpar_buffer();
        if (!validar_telefone(c.telefone))
            printf("Telefone invalido. Deve ter 10 ou 11 digitos numericos.\n");
    } while (!validar_telefone(c.telefone));

    printf("\n--- Resumo do cadastro ---\n");
    printf("ID: C(%d)\n", c.id);
    printf("Nome: %s\n", c.nome);
    printf("CNH: %s\n", c.cnh);
    printf("Telefone: %s\n", c.telefone);

    if (!confirmar("Confirma o cadastro deste cliente?"))
    {
        printf("Cadastro cancelado.\n");
        return -1;
    }

    clientes[total_clientes++] = c;
    if (salvar_clientes() != 0)
    {
        printf("Erro ao salvar cliente.\n");
        return -1;
    }
    gerar_relatorios_automaticos();
    printf("Cliente C(%d) cadastrado com sucesso.\n", c.id);

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Cliente cadastrado: C(%d) - %s (CNH: %s)",
             c.id, c.nome, c.cnh);
    escrever_log(log_msg);
    return 0;
}

// Lista todos os clientes
void listar_clientes()
{
    printf("\n-- Lista de Clientes --\n");
    if (total_clientes == 0)
    {
        printf("Nenhum cliente cadastrado.\n");
        return;
    }
    for (int i = 0; i < total_clientes; i++)
    {
        printf("C(%d) | %s | CNH: %s | Tel: %s\n",
               clientes[i].id, clientes[i].nome, clientes[i].cnh, clientes[i].telefone);
    }
}

// Busca cliente pelo nome exato
void buscar_cliente()
{
    char nome[TAM_NOME];
    ler_string("Nome para busca: ", nome, TAM_NOME);
    int idx = buscar_indice_cliente(nome);
    if (idx == -1)
    {
        printf("Cliente nao encontrado.\n");
        return;
    }
    printf("Encontrado -> C(%d) | %s | CNH: %s | Tel: %s\n",
           clientes[idx].id, clientes[idx].nome, clientes[idx].cnh, clientes[idx].telefone);
}

// Exclui cliente (desde que não tenha locação ativa), com confirmação
int excluir_cliente()
{
    char nome[TAM_NOME];
    ler_string("Nome do cliente a excluir: ", nome, TAM_NOME);

    int idx = buscar_indice_cliente(nome);
    if (idx == -1)
    {
        printf("Cliente nao encontrado.\n");
        return -1;
    }

    for (int i = 0; i < total_locacoes; i++)
    {
        if (locacoes[i].id_cliente == clientes[idx].id && locacoes[i].ativa == 1)
        {
            printf("Cliente possui locacao ativa. Nao pode ser excluido.\n");
            return -1;
        }
    }

    printf("\n--- Cliente a ser excluido ---\n");
    printf("ID: C(%d)\n", clientes[idx].id);
    printf("Nome: %s\n", clientes[idx].nome);
    printf("CNH: %s\n", clientes[idx].cnh);
    printf("Telefone: %s\n", clientes[idx].telefone);

    if (!confirmar("Confirma a exclusao deste cliente?"))
    {
        printf("Exclusao cancelada.\n");
        return -1;
    }

    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Cliente excluido: C(%d) - %s (CNH: %s)",
             clientes[idx].id, clientes[idx].nome, clientes[idx].cnh);

    for (int j = idx; j < total_clientes - 1; j++)
    {
        clientes[j] = clientes[j + 1];
    }
    total_clientes--;
    if (salvar_clientes() != 0)
    {
        printf("Erro ao salvar alteracoes.\n");
        return -1;
    }
    gerar_relatorios_automaticos();
    printf("Cliente excluido com sucesso.\n");
    escrever_log(log_msg);
    return 0;
}

// Menu do módulo de clientes
void menu_clientes()
{
    int opcao;
    do
    {
        printf("\n=== CLIENTES ===\n");
        printf("1. Cadastrar\n2. Listar\n3. Buscar\n4. Excluir\n5. Voltar\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        switch (opcao)
        {
        case 1:
            cadastrar_cliente();
            break;
        case 2:
            listar_clientes();
            break;
        case 3:
            buscar_cliente();
            break;
        case 4:
            excluir_cliente();
            break;
        case 5:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opcao != 5);
}

// ============================================================
// MODULO LOCACAO
// ============================================================

// Registra um novo aluguel, atualiza status do veículo e gera relatórios
int registrar_aluguel()
{
    printf("\n-- Registrar Aluguel --\n");

    listar_clientes();
    int id_c;
    printf("ID do cliente (numero): ");
    if (scanf("%d", &id_c) != 1)
    {
        printf("Entrada invalida.\n");
        limpar_buffer();
        return -1;
    }
    limpar_buffer();

    int ic = -1;
    for (int i = 0; i < total_clientes; i++)
    {
        if (clientes[i].id == id_c)
        {
            ic = i;
            break;
        }
    }
    if (ic == -1)
    {
        printf("Cliente nao encontrado.\n");
        return -1;
    }

    printf("\nVeiculos disponiveis:\n");
    int disponiveis = 0;
    for (int i = 0; i < total_veiculos; i++)
    {
        if (veiculos[i].status == 0)
        {
            printf("V(%d) | %s | %s %s | Diaria: R$%.2f\n",
                   veiculos[i].id, veiculos[i].placa, veiculos[i].marca,
                   veiculos[i].modelo, veiculos[i].diaria);
            disponiveis++;
        }
    }
    if (disponiveis == 0)
    {
        printf("Nenhum veiculo disponivel.\n");
        return -1;
    }

    int id_v;
    printf("ID do veiculo (numero): ");
    if (scanf("%d", &id_v) != 1)
    {
        printf("Entrada invalida.\n");
        limpar_buffer();
        return -1;
    }
    limpar_buffer();

    int iv = -1;
    for (int i = 0; i < total_veiculos; i++)
    {
        if (veiculos[i].id == id_v && veiculos[i].status == 0)
        {
            iv = i;
            break;
        }
    }
    if (iv == -1)
    {
        printf("Veiculo nao disponivel ou nao encontrado.\n");
        return -1;
    }

    int dias;
    printf("Quantidade de dias: ");
    if (scanf("%d", &dias) != 1 || dias <= 0)
    {
        printf("Numero de dias invalido.\n");
        limpar_buffer();
        return -1;
    }
    limpar_buffer();

    float total = dias * veiculos[iv].diaria;

    printf("\n--- Resumo da locacao ---\n");
    printf("Cliente: %s (C(%d))\n", clientes[ic].nome, clientes[ic].id);
    printf("Veiculo: %s %s (V(%d)) - Placa: %s\n",
           veiculos[iv].marca, veiculos[iv].modelo, veiculos[iv].id, veiculos[iv].placa);
    printf("Dias: %d\n", dias);
    printf("Valor total: R$%.2f\n", total);

    if (!confirmar("Confirma o aluguel?"))
    {
        printf("Aluguel cancelado.\n");
        return -1;
    }

    Locacao l;
    l.id = total_locacoes + 1;
    l.id_veiculo = veiculos[iv].id;
    l.id_cliente = clientes[ic].id;
    l.dias = dias;
    l.valor_total = total;
    l.ativa = 1;
    locacoes[total_locacoes++] = l;

    // Atualiza status do veículo em memória e no arquivo com fseek
    veiculos[iv].status = 1;
    if (atualizar_veiculo_no_arquivo(veiculos[iv].id, &veiculos[iv]) != 0)
    {
        printf("Erro ao atualizar veiculo no arquivo.\n");
        return -1;
    }

    if (salvar_locacoes() != 0)
    {
        printf("Erro ao salvar locacao.\n");
        return -1;
    }
    gerar_relatorios_automaticos();
    printf("Locacao L(%d) registrada com sucesso.\n", l.id);

    char log_msg[300];
    snprintf(log_msg, sizeof(log_msg),
             "Aluguel registrado: L(%d) - Cliente: %s (C(%d)) | Veiculo: %s %s (V(%d)) | Dias: %d | Total: R$%.2f",
             l.id, clientes[ic].nome, clientes[ic].id,
             veiculos[iv].marca, veiculos[iv].modelo, veiculos[iv].id,
             dias, total);
    escrever_log(log_msg);
    return 0;
}

// Exibe o histórico de locações com opção de filtro por veículo ou cliente
void historico_locacoes()
{
    int opcao;
    printf("\n-- Historico de Locacoes --\n");
    printf("1. Buscar por veiculo\n2. Buscar por cliente\n3. Listar todos\n");
    printf("Opcao: ");
    if (scanf("%d", &opcao) != 1)
    {
        printf("Entrada invalida.\n");
        limpar_buffer();
        return;
    }
    limpar_buffer();

    int encontrado = 0;

    if (opcao == 1)
    {
        int id_v;
        printf("ID do veiculo: ");
        if (scanf("%d", &id_v) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            return;
        }
        limpar_buffer();
        for (int i = 0; i < total_locacoes; i++)
        {
            if (locacoes[i].id_veiculo == id_v)
            {
                char nome_c[TAM_NOME] = "?";
                for (int j = 0; j < total_clientes; j++)
                {
                    if (clientes[j].id == locacoes[i].id_cliente)
                    {
                        strcpy(nome_c, clientes[j].nome);
                        break;
                    }
                }
                printf("L(%d) | V(%d) | Cliente: %s | Dias: %d | Total: R$%.2f | %s\n",
                       locacoes[i].id, locacoes[i].id_veiculo, nome_c,
                       locacoes[i].dias, locacoes[i].valor_total,
                       locacoes[i].ativa ? "ATIVA" : "DEVOLVIDA");
                encontrado = 1;
            }
        }
    }
    else if (opcao == 2)
    {
        int id_c;
        printf("ID do cliente: ");
        if (scanf("%d", &id_c) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            return;
        }
        limpar_buffer();
        for (int i = 0; i < total_locacoes; i++)
        {
            if (locacoes[i].id_cliente == id_c)
            {
                char placa_v[TAM_PLACA] = "?";
                for (int j = 0; j < total_veiculos; j++)
                {
                    if (veiculos[j].id == locacoes[i].id_veiculo)
                    {
                        strcpy(placa_v, veiculos[j].placa);
                        break;
                    }
                }
                printf("L(%d) | Placa: %s | C(%d) | Dias: %d | Total: R$%.2f | %s\n",
                       locacoes[i].id, placa_v, locacoes[i].id_cliente,
                       locacoes[i].dias, locacoes[i].valor_total,
                       locacoes[i].ativa ? "ATIVA" : "DEVOLVIDA");
                encontrado = 1;
            }
        }
    }
    else if (opcao == 3)
    {
        for (int i = 0; i < total_locacoes; i++)
        {
            printf("L(%d) | V(%d) | C(%d) | Dias: %d | Total: R$%.2f | %s\n",
                   locacoes[i].id, locacoes[i].id_veiculo, locacoes[i].id_cliente,
                   locacoes[i].dias, locacoes[i].valor_total,
                   locacoes[i].ativa ? "ATIVA" : "DEVOLVIDA");
            encontrado = 1;
        }
    }
    else
    {
        printf("Opcao invalida.\n");
        return;
    }
    if (!encontrado)
        printf("Nenhuma locacao encontrada.\n");
}

// Registra a devolução de um veículo, atualizando status e arquivos
int registrar_devolucao()
{
    printf("\n-- Registrar Devolucao --\n");
    int id_l;
    printf("ID da locacao: ");
    if (scanf("%d", &id_l) != 1)
    {
        printf("Entrada invalida.\n");
        limpar_buffer();
        return -1;
    }
    limpar_buffer();

    int idx = buscar_indice_locacao(id_l);
    if (idx == -1 || locacoes[idx].ativa == 0)
    {
        printf("Locacao nao encontrada ou ja encerrada.\n");
        return -1;
    }

    printf("\n--- Resumo da devolucao ---\n");
    printf("Locacao: L(%d)\n", locacoes[idx].id);
    printf("Veiculo: V(%d)\n", locacoes[idx].id_veiculo);
    printf("Cliente: C(%d)\n", locacoes[idx].id_cliente);
    printf("Dias contratados: %d\n", locacoes[idx].dias);
    printf("Valor total: R$%.2f\n", locacoes[idx].valor_total);

    if (!confirmar("Confirma a devolucao?"))
    {
        printf("Devolucao cancelada.\n");
        return -1;
    }

    char log_msg[300];
    snprintf(log_msg, sizeof(log_msg),
             "Devolucao registrada: L(%d) - Veiculo V(%d) | Cliente C(%d) | Dias: %d | Total: R$%.2f",
             locacoes[idx].id, locacoes[idx].id_veiculo,
             locacoes[idx].id_cliente, locacoes[idx].dias, locacoes[idx].valor_total);

    locacoes[idx].ativa = 0;
    if (atualizar_locacao_no_arquivo(locacoes[idx].id, &locacoes[idx]) != 0)
    {
        printf("Erro ao atualizar locacao no arquivo.\n");
        return -1;
    }

    // Libera o veículo
    for (int i = 0; i < total_veiculos; i++)
    {
        if (veiculos[i].id == locacoes[idx].id_veiculo)
        {
            veiculos[i].status = 0;
            if (atualizar_veiculo_no_arquivo(veiculos[i].id, &veiculos[i]) != 0)
            {
                printf("Erro ao atualizar veiculo no arquivo.\n");
                return -1;
            }
            break;
        }
    }

    gerar_relatorios_automaticos();
    printf("Devolucao registrada. Veiculo V(%d) disponivel novamente.\n",
           locacoes[idx].id_veiculo);

    escrever_log(log_msg);
    return 0;
}

// Menu do módulo de locação
void menu_locacao()
{
    int opcao;
    do
    {
        printf("\n=== LOCACAO ===\n");
        printf("1. Registrar aluguel\n2. Historico de locacoes\n3. Registrar devolucao\n4. Voltar\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        switch (opcao)
        {
        case 1:
            registrar_aluguel();
            break;
        case 2:
            historico_locacoes();
            break;
        case 3:
            registrar_devolucao();
            break;
        case 4:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opcao != 4);
}

// ============================================================
// RELATORIOS EM TELA
// ============================================================

// Exibe na tela a lista de veículos disponíveis
void relatorio_disponiveis()
{
    printf("\n-- Veiculos Disponiveis --\n");
    int count = 0;
    for (int i = 0; i < total_veiculos; i++)
    {
        if (veiculos[i].status == 0)
        {
            printf("V(%d) | %s | %s %s | Ano: %d | Diaria: R$%.2f\n",
                   veiculos[i].id, veiculos[i].placa, veiculos[i].marca,
                   veiculos[i].modelo, veiculos[i].ano, veiculos[i].diaria);
            count++;
        }
    }
    if (count == 0)
        printf("Nenhum veiculo disponivel no momento.\n");
}

// Exibe na tela a lista de locações ativas
void relatorio_locacoes_ativas()
{
    printf("\n-- Locacoes Ativas --\n");
    int count = 0;
    for (int i = 0; i < total_locacoes; i++)
    {
        if (locacoes[i].ativa == 1)
        {
            char placa_v[TAM_PLACA] = "?";
            char nome_c[TAM_NOME] = "?";
            for (int j = 0; j < total_veiculos; j++)
            {
                if (veiculos[j].id == locacoes[i].id_veiculo)
                {
                    strcpy(placa_v, veiculos[j].placa);
                    break;
                }
            }
            for (int j = 0; j < total_clientes; j++)
            {
                if (clientes[j].id == locacoes[i].id_cliente)
                {
                    strcpy(nome_c, clientes[j].nome);
                    break;
                }
            }
            printf("L(%d) | Veiculo: %s | Cliente: %s | Dias: %d | Total: R$%.2f\n",
                   locacoes[i].id, placa_v, nome_c, locacoes[i].dias, locacoes[i].valor_total);
            count++;
        }
    }
    if (count == 0)
        printf("Nenhuma locacao ativa no momento.\n");
}

// Menu de relatórios (apenas exibição em tela)
void menu_relatorios()
{
    int opcao;
    do
    {
        printf("\n=== RELATORIOS ===\n");
        printf("1. Veiculos disponiveis (tela)\n");
        printf("2. Locacoes ativas (tela)\n");
        printf("3. Voltar\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        switch (opcao)
        {
        case 1:
            relatorio_disponiveis();
            break;
        case 2:
            relatorio_locacoes_ativas();
            break;
        case 3:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opcao != 3);
}

// ============================================================
// MENU PRINCIPAL
// ============================================================

int main()
{
    // Tenta carregar os arquivos; se não existirem, cria vazios
    if (carregar_veiculos() != 0)
        salvar_veiculos();
    if (carregar_clientes() != 0)
        salvar_clientes();
    if (carregar_locacoes() != 0)
        salvar_locacoes();

    // SE NÃO HOUVER NENHUM DADO CARREGADO, GERA AS SIMULAÇÕES AUTOMATICAMENTE
    if (total_veiculos == 0 && total_clientes == 0 && total_locacoes == 0)
    {
        printf("Nenhum dado encontrado. Gerando dados de exemplo para demonstração...\n");
        gerar_dados_teste();
        printf("Dados de exemplo gerados com sucesso!\n");
    }

    // Registra no log a inicialização do sistema
    escrever_log("Sistema iniciado");

    // Gera os relatórios iniciais (arquivos .txt) com os dados carregados ou de teste
    gerar_relatorios_automaticos();

    int opcao;
    do
    {
        printf("\n=============================\n");
        printf("   LOCADORA DE VEICULOS\n");
        printf("=============================\n");
        printf("1. Veiculos\n");
        printf("2. Clientes\n");
        printf("3. Locacao\n");
        printf("4. Relatorios\n");
        printf("5. Sair\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1)
        {
            printf("Entrada invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();
        switch (opcao)
        {
        case 1:
            menu_veiculos();
            break;
        case 2:
            menu_clientes();
            break;
        case 3:
            menu_locacao();
            break;
        case 4:
            menu_relatorios();
            break;
        case 5:
            printf("Encerrando o sistema.\n");
            escrever_log("Sistema encerrado");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opcao != 5);

    return 0;
}