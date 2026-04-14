#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARR_SIZE 10

// STRUCTS ========================== //

/* Argumentos passados para cada Thread */
typedef struct ThreadArgs {
    int *arr;
    int left;
    int right;
} ThreadArgs;

// DECLARAÇÃO DAS FUNÇÕES =========== //

void merge(int *arr, int l, int m, int r);
void merge_sort(int *arr, int l, int r);

void *routine (void *arg);

// VVVVVVVVVVVVV MAIN VVVVVVVVVVVVVVV //

int main (int argc, char *argv[]) {

    int input_arr[ARR_SIZE];
    srand(time(NULL));  // semente baseada no tempo
    for (int i = 0; i < ARR_SIZE; i++) {
        input_arr[i] = rand() % 1000 - 500;  // valores entre -500 e 499
    }

    printf("\nARRAY DESORDENADO:\n{ ");
    for (int i = 0; i < ARR_SIZE; i++) {
        if (i < ARR_SIZE - 1) printf("%d, ", input_arr[i]);
        else printf("%d", input_arr[i]);
    }
    printf(" }\n\n");

    merge_sort(input_arr, 0, ARR_SIZE - 1);

    printf("\nARRAY ORDENADO:\n{ ");
    for (int i = 0; i < ARR_SIZE; i++) {
        if (i < ARR_SIZE - 1) printf("%d, ", input_arr[i]);
        else printf("%d", input_arr[i]);
    }
    printf(" }\n\n");

    return 0;
}

// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV //


void merge(int *arr, int l, int m, int r) {
    
    int i, j, k;
    
    int n1 = m - l + 1;
    int n2 = r - m;

    int leftArr[n1], rightArr[n2];
    
    for (i = 0; i < n1; i++) leftArr[i] = arr[l + i];
    for (j = 0; j < n2; j++) rightArr[j]= arr[m + 1 + j];
    
    i = 0;
    j = 0;
    k = l;
    
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k] = leftArr[i];
            i++;
        } else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }
    
    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
    
};

// MERGE SORTE **COM THREADS** //

void merge_sort(int *arr, int l, int r) {

    pthread_t thread_left;
    ThreadArgs args_left;
    pthread_t thread_right;
    ThreadArgs args_right;

    if (l < r) {
        int mid = l + (r - l) / 2;

        args_left.arr = arr;
        args_left.left = l;
        args_left.right = mid;
        if (pthread_create(&thread_left, NULL, routine, &args_left) != 0) {
            perror("Falha na create da thread.\n");
        }
        
        args_right.arr = arr;
        args_right.left = mid + 1;
        args_right.right = r;
        if (pthread_create(&thread_right, NULL, routine, &args_right) != 0) {
            perror("Falha na create da thread.\n");
        }

        pthread_join(thread_left, NULL);
        pthread_join(thread_right, NULL);
        merge(arr, l, mid, r);
    }
};

void *routine (void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    printf("Thread ORDENANDO intervalo [%d, %d]\n",
        args->left, args->right);
    merge_sort(args->arr, args->left, args->right);
    printf("Thread CONCLUIU  intervalo [%d, %d]\n",
        args->left, args->right);
    return NULL;
};