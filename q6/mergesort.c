/*
 * Merge Sort Concorrente - Questão 6
 * 
 * DESCRIÇÃO GERAL: 
 * 
 * Esta é a implementação de uma versão concorrente do algoritmo de ordenação Merge
 * Sort. Vimos a versão clássica deste na cadeira de Algoritmos e Estruturas de Dados
 * no período 2025.2. Tal como feito na q3, primeiro o alg. foi implementado de forma
 * manual para depois se aplicar a concorrência através de threads. 
 *  
 * DESCRIÇÕES DE IMPLEMENTAÇÃO:
 * 
 * - Criação das Threads: cada vez que o algoritmo divide o array, uma nova thread é
 *   criada para tratar de ordenar essa sub-seção de esquerda e direita. Sendo assim,
 *   threads são criadas até chegar no caso base. Isso fica claro quando o output
 *   mostra que uma thread está ordenando um intervalo [x,y] e, mais na frente, este
 *   mesmo intervalo aparece como concluído.
 * - Crescimento de Threads: a criação é exponencial - cada nível de recursão dobra
 *   o número de threads ativas. Para arrays pequenos (como ARR_SIZE = 10) isso é
 *   manejável, maaas, para arrays grandes, esta abordagem seria substituída por um
 *   threshold(limite). Noutras palavras, abaixo de certo tamanho (32~64), o merge sort 
 *   roda sequencialmente.
 * - Melhoria possível: implementar esse threshold para evitar criação excessiva de
 *   threads em sub-arrays pequenos, reduzindo o overhead de criação/destruição.
 * - "Sem mutex?": Como cada thread atua em sub-seções desanexadas, a exclusão mútua
 *   já é natural, então não houve necessidade da implementação de locks.
 * 
 * Compilar: gcc -g -Wall -pthread mergesort.c -o mergesort
 * Uso: ./mergesort
 * 
 * TESTE DISPONÍVEL:
 * 
 * Foi implementada a criação de random arrays com um tamanho hard coded `ARR_SIZE` -
 * que pode ser alterado pelo programador a depender do que deseja testar - através do
 * `srand(time(NULL))` para gerar a semente baseada no tempo.
 * 
 * Formato do output:
 * 
 * Primeiro o programa informa qual foi o array gerado (DESORDENADO) e mostra o que a
 * thread de cada intervalo está fazendo (ordenando ou se concluiu) para enfim mostrar 
 * o resultado final. Abaixo um exemplo com um ARR_SIZE = 2:
 * 
 * ARRAY DESORDENADO:
 * { -164, -441 }
 *
 * Thread ORDENANDO intervalo [0, 0]
 * Thread CONCLUIU  intervalo [0, 0]
 * Thread ORDENANDO intervalo [1, 1]
 * Thread CONCLUIU  intervalo [1, 1]
 * 
 * ARRAY ORDENADO:
 * { -441, -164 }
 * 
 * OBS: NÃO TESTE UM ARR_SIZE COM ORDEM DE GRANDEZA MAIOR DO QUE 10^4!! TRAVOU TUDO AQUI
 */

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
    
    // copia os dois lados para arrays temporários antes de comparar.
    // necessário pois o merge escreve no mesmo array que lê
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

/* 
 * As Threads foram separadas em esquerda e direita, bem como seus argumentos.
 * Cada uma gera, no máximo, duas filhas até chegar no caso base, quando começam
 * a retornar para seus pais.
 */

void merge_sort(int *arr, int l, int r) {

    pthread_t thread_left;
    ThreadArgs args_left;
    pthread_t thread_right;
    ThreadArgs args_right;

    if (l < r) {
        int mid = l + (r - l) / 2;

        // Preenche argumentos da thread da esquerda
        args_left.arr = arr;
        args_left.left = l;
        args_left.right = mid;
        if (pthread_create(&thread_left, NULL, routine, &args_left) != 0) {
            perror("Falha na create da thread.\n");
        }
        
        // Preenche argumentos da thread da direita
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