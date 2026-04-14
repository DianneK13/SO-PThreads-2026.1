/*
 * Hashmap Concorrente - Questão 3
 * 
 * DESCRIÇÃO GERAL: 
 * 
 * Esta é uma implementação de uma Hash Map, a versão concorrente da clássica 
 * Hash Table, vista por nós durante a cadeira Algoritimos e Estruturas de Dados 
 * período passado, 2025.2. Sendo assim, foi primeiro feita a implementação manual
 * de uma Hash Table para então aplicar a concorrência através de threads.
 * 
 * DESCRIÇÕES DE IMPLEMENTAÇÃO:
 * 
 * - Quantidade de Threads: Foi escolhido fazer uma implementação robusta em que
 *   cada operação da entrada recebe uma thread dedicada. Se o arquivo possuir 25
 *   operações, serão criadas 25 threads.
 * - Algoritmo de Hashing: Decidiu-se pela implementação de um algoritmo um pouco
 *   mais seguro do que o clássico mod. A hashing_function soma o valor ASCII de
 *   cada caractere da key, que é um char*, e então retira o % size para adquirir
 *   o id do bucket.
 * - Fine-Grained-Locking: Falando em buckets, cada um deles recebe um mutex 
 *   dedicado para evitar deadlocks durante colisões, que certamente ocorrerão.
 * 
 * Compilar: gcc -g -Wall -pthread hashmap.c -o hashmap
 * Uso: ./hashmap <arquivo_de_entrada>
 * 
 * Formato do arquivo de entrada:
 *   insert <chave> <valor>
 *   search <chave>
 *   delete <chave>
 * 
 * Testes disponíveis: TesteCase1.txt a TesteCase5.txt
 * 
 * Case 1: Teste incial das instruções
 * Case 2: Aumenta o nível da água aumentando a quantidade de instruções
 * Case 3: Testa várias instruções de uma mesma key
 * Case 4: Testa o comportamento do sis ao buscar ou deletar elementos inexistentes
 * Case 5: Testa várias instruções querendo mexer no mesmo bucket
 */

#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 5
#define MAX_OPS 100

// STRUCTS ========================== //

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

typedef struct HashMap {
    int size;               // num de buckets
    Node **elements;
    pthread_mutex_t *locks; // um mutex por bucket
} HashMap;

/* Tipos de op suportados pelo arquivo de entrada */
typedef enum OpType {
    INSERT,
    SEARCH,
    DELETE
} OpType;

/* Representa uma op lida do arquivo de entrada */
typedef struct Operation {
    OpType type;
    char key[50];
    int value;
} Operation;

/* Argumentos passados para cada Thread */
typedef struct ThreadArgs {
    HashMap *hm;
    int id;
    Operation op;
} ThreadArgs;

// ================================== //

// DECLARAÇÃO DAS FUNÇÕES =========== //

int hashFunction(const char *key, int size);
HashMap *hashmap_create(int size);
void hashmap_insert(HashMap *hm, const char *key,  int value);
int hashmap_search(HashMap *hm, const char *key);
bool hashmap_delete(HashMap *hm, const char *key);
void hashmap_destroy(HashMap *hm);

void *routine(void *arg);

// ================================== //

// VVVVVVVVVVVVV MAIN VVVVVVVVVVVVVVV //

int main(int argc, char *argv[]) {
    HashMap *hm = hashmap_create(10);
    pthread_t threads[MAX_OPS];
    Operation ops[MAX_OPS];
    int op_count = 0;

    // arq de entrada passado como arg: ./hashmap <arquivo>
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return 1;
    } else {

        char op_type[10];
        char key[50];
        int value;

        // cada linha do arq vira uma Operation no array ops[]
        // fscanf retorna EOF quando o arquivo termina
        while(fscanf(file, "%s %s %d", op_type, key, &value) >= 1) {
            // INSERT
            if (strcmp(op_type, "insert") == 0) {
                ops[op_count].type = INSERT;
                strcpy(ops[op_count].key, key);
                ops[op_count].value = value;
            }
            // SEARCH
            if (strcmp(op_type, "search") == 0) {
                ops[op_count].type = SEARCH;
                strcpy(ops[op_count].key, key);   
            }
            // DELETE
            if (strcmp(op_type, "delete") == 0) {
                ops[op_count].type = DELETE;
                strcpy(ops[op_count].key, key);
            }
            op_count++; /*usado para o num de threads*/
        }

        if (fclose(file) != 0) {
            perror("Erro ao fechar arquivo");
        }
    }

    ThreadArgs args[MAX_OPS];

    // cada op recebe uma thread dedicada
    // args[] é alocado em stack - o join garante a segurança
    // já que as threads terminam antes da main retornar
    for (int i = 0; i < op_count; i++) {
        args[i].hm = hm;
        args[i].id = i;
        args[i].op = ops[i];
        if (pthread_create(&threads[i], NULL, routine, &args[i]) != 0) {
            perror("Falha na create da thread.\n");
        }
    }
    for (int i = 0; i < op_count; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Falha no join da thread.\n");
        }
    }

    hashmap_destroy(hm);
    return 0;
}

// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV //

// ROTINA DE FUNCIONALIDADES DE INSERT, SEARCH E DELETE DA HASHMAP //

void *routine(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;

    switch (args->op.type) {
        case INSERT: {
            hashmap_insert(args->hm, args->op.key, args->op.value);
            printf("Thread [%d] inseriu [%s -> %d].\n", 
                args->id,
                args->op.key,
                args->op.value);
                break;
        }

        case SEARCH: {
            int result = hashmap_search(args->hm, args->op.key);
            if (result != -1) {
                printf("Thread [%d] achou [%s -> %d].\n",
                    args->id,
                    args->op.key,
                    result);
            } else {
                printf("Thread [%d] nao encontrou [%s].\n", 
                    args->id,
                    args->op.key);
            }
            break;
        }

        case DELETE: {
            if (hashmap_delete(args->hm, args->op.key)) {
                printf("Thread [%d] deletou [%s].\n", 
                    args->id, args->op.key);
            } else {
                printf("Thread [%d] nao encontrou [%s] para deletar.\n",
                    args->id, args->op.key);
            }
            break;
        }

        default:
            return NULL;
    }
    return NULL;
}

// CONSTRUCAO DA HASHMAP **COM THREADS**//

int hashFunction(const char *key, int size) {
    // soma dps valores ASCII da key garante dist uniforme entre buckets
    int sum = 0;
    for (int i = 0; i < strlen(key); i++) {
        sum += (int)key[i];
    }
    return sum % size;
};

HashMap *hashmap_create(int size) {
    HashMap *hm = malloc(sizeof(HashMap));
    if (hm == NULL) return NULL;

    hm->size = size;

    // calloc zera a mem, ent todos os buckets iniciam como NULL
    hm->elements = calloc(sizeof(Node*), hm->size);
    if (hm->elements == NULL) { free(hm); return NULL; }

    // um mutex por bucket permite ops paralelas em buckets distintos
    hm->locks = calloc(sizeof(pthread_mutex_t), hm->size);
    if (hm->locks == NULL) { free(hm->elements); free(hm); return NULL; }
    for (int i = 0; i < size; i++) {
        pthread_mutex_init(&hm->locks[i], NULL);
    }
    
    return hm;
}

void hashmap_insert(HashMap *hm, const char *key,  int value) {
    int id = hashFunction(key, hm->size);

    // trava apenas o bucket afetado - outras threads operam livremente
    pthread_mutex_lock(&hm->locks[id]);

    Node *new_node = malloc(sizeof(Node));
    if (!new_node) { 
        pthread_mutex_unlock(&hm->locks[id]);
        return; 
    }

    new_node->key = malloc(strlen(key) + 1);
    if (!new_node->key) { 
        free(new_node);
        pthread_mutex_unlock(&hm->locks[id]);
        return;
    }
    strcpy(new_node->key, key);
    new_node->value = value;

    // inserção no inicio da lista do bucket, sem necssidade de percorrer
    new_node->next = hm->elements[id];
    hm->elements[id] = new_node;
    pthread_mutex_unlock(&hm->locks[id]);
}

int hashmap_search(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

    // leitura também precisa de lock - outra pode estar deletando
    pthread_mutex_lock(&hm->locks[id]);
    Node *temp = hm->elements[id];
    while(temp != NULL) {
        if (strcmp(temp->key, key) == 0) {
            pthread_mutex_unlock(&hm->locks[id]);
            return temp->value;
        }
        temp = temp->next;
    }
    pthread_mutex_unlock(&hm->locks[id]);
    return -1;   /*chave não encontrada*/
}

bool hashmap_delete(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

    pthread_mutex_lock(&hm->locks[id]);
    Node *prev = NULL;
    Node *cur = hm->elements[id];
    while(cur !=NULL) {
        if (strcmp(cur->key, key) == 0) {
            // reconecta a lista antes de liberar node
            if (prev == NULL) hm->elements[id] = cur->next;
            else prev->next = cur->next;
            free(cur->key);
            free(cur);
            pthread_mutex_unlock(&hm->locks[id]);
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    pthread_mutex_unlock(&hm->locks[id]);
    return false;  /*chave não encontrada*/
}

void hashmap_destroy(HashMap *hm) {
    if (!hm) return;

    for (int i = 0; i < hm->size; i++) {
        Node *cur = hm->elements[i];
        while (cur) {
            Node *next = cur->next;
            free(cur->key);
            free(cur);
            cur = next;
        }
    }

    // mutex deve ser destruído antes de liberar o array
    for (int i = 0; i < hm->size; i++) {
        pthread_mutex_destroy(&hm->locks[i]);
    }

    free(hm->elements);
    free(hm->locks);
    free(hm);
}