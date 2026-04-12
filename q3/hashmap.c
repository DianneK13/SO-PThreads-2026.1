#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *key;
    int value;
    struct Node *next;
} Node;

typedef struct HashMap {
    int size;
    Node **elements;
} HashMap;

int hashFunction(const char *key, int size);
HashMap *hashmap_create(int size);
void hashmap_insert(HashMap *hm, const char *key,  int value);
int hashmap_search(HashMap *hm, const char *key);
void hashmap_delete(HashMap *hm, const char *key);
void hashmap_destroy(HashMap *hm);

int main() {

    return 0;
}


// CONSTRUCAO DA HASHMAP //

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
    return hm;
}

void hashmap_insert(HashMap *hm, const char *key,  int value) {
    int id = hashFunction(key, hm->size);

    // cria um novo node
    Node *new_node = malloc(sizeof(Node));
    if (!new_node) return;
    new_node->key = malloc(strlen(key) + 1);
    if (!new_node->key) { free(new_node); return; }
    strcpy(new_node->key, key);
    new_node->value = value;

    // insere no inicio da lista do bucket
    new_node->next = hm->elements[id];
    hm->elements[id] = new_node;
}

int hashmap_search(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

    Node *temp = hm->elements[id];
    while(temp != NULL) {
        if (strcmp(temp->key, key) == 0) return temp->value;
        temp = temp->next;
    }
    return -1;
}

void hashmap_delete(HashMap *hm, const char *key) {
    int id = hashFunction(key, hm->size);

    Node *prev = NULL;
    Node *cur = hm->elements[id];
    while(cur !=NULL) {
        if (strcmp(cur->key, key) == 0) {
            if (prev == NULL) hm->elements[id] = cur->next;
            else prev->next = cur->next;
            free(cur->key);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
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

    free(hm->elements);
    free(hm);
}