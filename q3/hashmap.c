#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 5
#define MAX_OPS 100

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

typedef struct HashMap {
    int size;
    Node **elements;
    pthread_mutex_t *locks;
} HashMap;

typedef enum OpType {
    INSERT,
    SEARCH,
    DELETE
} OpType;

typedef struct Operation {
    OpType type;
    char key[50];
    int value;
} Operation;

typedef struct ThreadArgs {
    HashMap *hm;
    int id;
    Operation op;
} ThreadArgs;

int hashFunction(const char *key, int size);
HashMap *hashmap_create(int size);
void hashmap_insert(HashMap *hm, const char *key,  int value);
int hashmap_search(HashMap *hm, const char *key);
bool hashmap_delete(HashMap *hm, const char *key);
void hashmap_destroy(HashMap *hm);

void *routine(void *arg);

int main(int argc, char *argv[]) {
    HashMap *hm = hashmap_create(10);
    pthread_t threads[MAX_OPS];
    Operation ops[MAX_OPS];
    int op_count = 0;

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return 1;
    } else {

        char op_type[10];
        char key[50];
        int value;

        
        while(fscanf(file, "%s %s %d", op_type, key, &value) >= 1) {
            if (strcmp(op_type, "insert") == 0) {
                ops[op_count].type = INSERT;
                strcpy(ops[op_count].key, key);
                ops[op_count].value = value;
            }
            if (strcmp(op_type, "search") == 0) {
                ops[op_count].type = SEARCH;
                strcpy(ops[op_count].key, key);   
            }
            if (strcmp(op_type, "delete") == 0) {
                ops[op_count].type = DELETE;
                strcpy(ops[op_count].key, key);
            }
            op_count++;
        }

        if (fclose(file) != 0) {
            perror("Erro ao fechar arquivo");
        }
    }

    ThreadArgs args[MAX_OPS];

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

    hm->elements = calloc(sizeof(Node*), hm->size);
    if (hm->elements == NULL) { free(hm); return NULL; }

    hm->locks = calloc(sizeof(pthread_mutex_t), hm->size);
    if (hm->locks == NULL) { free(hm->elements); free(hm); return NULL; }
    for (int i = 0; i < size; i++) {
        pthread_mutex_init(&hm->locks[i], NULL);
    }
    
    return hm;
}

void hashmap_insert(HashMap *hm, const char *key,  int value) {
    int id = hashFunction(key, hm->size);

    pthread_mutex_lock(&hm->locks[id]);
    // cria um novo node
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

    // insere no inicio da lista do bucket
    new_node->next = hm->elements[id];
    hm->elements[id] = new_node;
    pthread_mutex_unlock(&hm->locks[id]);
}

int hashmap_search(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

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
    return -1;
}

bool hashmap_delete(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

    pthread_mutex_lock(&hm->locks[id]);
    Node *prev = NULL;
    Node *cur = hm->elements[id];
    while(cur !=NULL) {
        if (strcmp(cur->key, key) == 0) {
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
    return false;
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

    for (int i = 0; i < hm->size; i++) {
        pthread_mutex_destroy(&hm->locks[i]);
    }

    free(hm->elements);
    free(hm->locks);
    free(hm);
}