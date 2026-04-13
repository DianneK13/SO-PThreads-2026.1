#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 5

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

int hashFunction(const char *key, int size);
HashMap *hashmap_create(int size);
void hashmap_insert(HashMap *hm, const char *key,  int value);
int hashmap_search(HashMap *hm, const char *key);
void hashmap_delete(HashMap *hm, const char *key);
void hashmap_destroy(HashMap *hm);

void *routine(void *arg) {
    HashMap *hm = (HashMap *)arg;

    //..

}

int main() {
    HashMap *hm = hashmap_create(10);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, routine, hm) != 0) {
            perror("Failed to create thread.\n");
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread.\n");
        }
    }

    hashmap_destroy(hm);
    return 0;
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

void hashmap_delete(HashMap *hm, const char *key) {
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
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    pthread_mutex_unlock(&hm->locks[id]);
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