#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 6

//Declaracao dos ponteiros necessarios na funcao executada pelas threads
int* tamanhoMatriz;
int** matriz;

//Alocar matriz
int** criarMatriz(int tamanho) {
    int **matriz = malloc(tamanho * sizeof(int*));
    if (matriz == NULL) {
        printf("Falha na alocacao da matriz.\n");
        return NULL;
    }

    for (int i=0; i<tamanho; i++) {
        matriz[i] = malloc(tamanho * sizeof(int));
        if (matriz[i] == NULL) {
            for (int j=0; j<i; j++) {
                free(matriz[j]);
            }
            free(matriz);
            return NULL;
        }
    }
    return matriz;
}

//Liberar matriz
void freeMatriz(int **matriz, int tamanho) {
    for (int i=0; i<tamanho; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

//Verificar linhas
void* verificaLinhas(void* tid) {
    int threadId = (*(int*) tid);
    printf("[%d] Thread %d iniciou a verificacao de suas linhas\n", threadId, threadId);

    /*
    A threads de indice impar sao responsaveis pela verificacao das linhas. Quanto maior o numero
    de threads, menor o trabalho que cada uma faz individualmente. As threads impares sao entao numeradas
    sequencialmente com base em seus indices, e recebem blocos de mesmo tamanho de linhas a serem verificadas.
    Ex: threadId 1 -> threadLinhaId 0
        threadId 2 -> threadLinhaId 1
        threadId 3 -> threadLinhaId 2
    */
    int n = *tamanhoMatriz;
    int qntThreadsLinhas = NUM_THREADS/2;
    int threadLinhaId = threadId/2;
    int comeco = (threadLinhaId*n)/qntThreadsLinhas;
    int fim = ((threadLinhaId+1)*n)/qntThreadsLinhas;

    //Salva a soma da primeira linha
    int sum = 0;
    for(int j=0; j<*tamanhoMatriz; j++) {
        sum += matriz[0][j];
    }

    //Verifica se o resultado salvo difere da soma de alguma outra linha
    for(int i=comeco; i<fim; i++) {
        int newSum = 0;
        for(int j=0; j<*tamanhoMatriz; j++) {
            newSum += matriz[i][j];
        }
        if(newSum != sum) {
            printf("[%d] Thread %d finalizou a verificacao de suas linhas\n", threadId, threadId);
            return NULL;
        }
    }

    /*
    Retorna o resultado, caso todas as linhas tenham a mesma soma, para ser comparado posteriormente aos
    resultados das colunas e das diagonais
    */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    printf("[%d] Thread %d finalizou a verificacao de suas linhas\n", threadId, threadId);

    return (void*) result;
}

//Verificar colunas
void* verificaColunas(void* tid) {
    int threadId = (*(int*) tid);
    printf("[%d] Thread %d iniciou a verificacao de suas colunas\n", threadId, threadId);

    /*
    A threads de indice par > 0 sao responsaveis pela verificacao das colunas. Quanto maior o numero
    de threads, menor o trabalho que cada uma faz individualmente. As threads pares sao entao numeradas
    sequencialmente com base em seus indices, e recebem blocos de mesmo tamanho de colunas a serem verificadas.
    Ex: threadId 2 -> threadColunaId 0
        threadId 4 -> threadColunaId 1
        threadId 6 -> threadColunaId 2
    Ademais, se NUM_THREADS == 2, entao a threads responsavel pela colunas devera verificar todas, como
    consta na linha 318.
    */
    int n = *tamanhoMatriz;
    int comeco, fim;
    if(NUM_THREADS == 2) {
        comeco = 0;
        fim = n;
    }
    else {
        int qntThreadsColunas = (NUM_THREADS-1)/2;
        int threadColunaId = (threadId/2)-1;
        comeco = (threadColunaId*n)/qntThreadsColunas;
        fim = ((threadColunaId+1)*n)/qntThreadsColunas;
    }

    //Salva a soma da primeira coluna
    int sum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        sum += matriz[i][0];
    }

    //Verifica se o resultado salvo difere da soma de alguma outra coluna
    for(int j=comeco; j<fim; j++) {
        int newSum = 0;
        for(int i=0; i<*tamanhoMatriz; i++) {
            newSum += matriz[i][j];
        }
        if(newSum != sum) {
            printf("[%d] Thread %d finalizou a verificacao de suas colunas\n", threadId, threadId);
            return NULL;
        }
    }

    /*
    Retorna o resultado, caso todas as colunas tenham a mesma soma, para ser comparado posteriormente aos
    resultados das linhas e das diagonais
    */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    printf("[%d] Thread %d finalizou a verificacao de suas colunas\n", threadId, threadId);

    return (void*) result;
}

//Verificar diagonais
void* verificaDiagonais(void* tid) {
    int threadId = (*(int*) tid);
    printf("[%d] Thread %d iniciou a verificacao de suas diagonais\n", threadId, threadId);

    //Salva resultado da diagonal principal
    int sum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        sum += matriz[i][i];
    }

    //Salva resultado da diagonal secundaria
    int newSum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        newSum += matriz[i][*tamanhoMatriz - 1 - i];
    }

    //Verifica se sao diferentes
    if(sum != newSum) {
        printf("[%d] Thread %d finalizou a verificacao de suas diagonais\n", threadId, threadId);
        return NULL;
    }

    /*
    Retorna o resultado, caso as somas sejam iguais, para ser comparado posteriormente aos
    resultados das linhas e das colunas
    */
    int* result = malloc(sizeof(int));
    *result = sum;
    
    printf("[%d] Thread %d finalizou a verificacao de suas diagonais\n", threadId, threadId);

    return (void*) result;
}

//Caso NUM_THREADS == 1
void* oneThread(void* tid) {
    //Caso haja apenas uma so thread, ela tera que fazer todo o trabalho das tres funcoes acima

    int threadId = (*(int*) tid);
    printf("[%d] Thread %d iniciou a verificacao das linhas, colunas e diagonais\n", threadId, threadId);

    //Verificacao das diagonais
    int sum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        sum += matriz[i][i];
    }

    int newSum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        newSum += matriz[i][*tamanhoMatriz - 1 - i];
    }

    if(sum != newSum) {
        printf("[%d] Thread %d finalizou a verificacao de suas diagonais\n", threadId, threadId);
        return NULL;
    }
    
    //Verificacao das linhas
    for(int i=0; i<*tamanhoMatriz; i++) {
        int sumRow = 0;
        for(int j=0; j<*tamanhoMatriz; j++) {
            sumRow += matriz[i][j];
        }
        if(sumRow != sum) {
            printf("[%d] Thread %d finalizou a verificacao de suas linhas e diagonais\n", threadId, threadId);
            return NULL;
        }
    }

    //Verificacao das colunas
    for(int j=0; j<*tamanhoMatriz; j++) {
        int sumColumns = 0;
        for(int i=0; i<*tamanhoMatriz; i++) {
            sumColumns += matriz[i][j];
        }
        if(sumColumns != sum) {
            printf("[%d] Thread %d finalizou a verificacao de suas linhas, colunas e diagonais\n", threadId, threadId);
            return NULL;
        }
    }
    
    //Retorna uma flag caso todas as somas sejam iguais
    int* ok = malloc(sizeof(int));
    *ok = 1;

    printf("[%d] Thread %d finalizou a verificacao de suas linhas, colunas e diagonais\n", threadId, threadId);

    return ok;
}

//Caso NUM_THREADS == 2
void* twoThreads(void* tid) {
    /*
    Caso haja apenas duas threads, a que realizara esta funcao tera que fazer o trabalho de
    verificaDiagonais() e de verificaLinhas().
    */

    int threadId = (*(int*) tid);
    printf("[%d] Thread %d iniciou a verificacao das linhas e diagonais\n", threadId, threadId);

    //Verificacao das diagonais
    int sum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        sum += matriz[i][i];
    }

    int newSum = 0;
    for(int i=0; i<*tamanhoMatriz; i++) {
        newSum += matriz[i][*tamanhoMatriz - 1 - i];
    }

    if(sum != newSum) {
        printf("[%d] Thread %d finalizou a verificacao de suas diagonais\n", threadId, threadId);
        return NULL;
    }
    
    //Verificacao das linhas
    for(int i=0; i<*tamanhoMatriz; i++) {
        int sumRow = 0;
        for(int j=0; j<*tamanhoMatriz; j++) {
            sumRow += matriz[i][j];
        }
        if(sumRow != sum) {
            printf("[%d] Thread %d finalizou a verificacao de suas linhas e diagonais\n", threadId, threadId);
            return NULL;
        }
    }

    /*
    Retorna o resultado, caso as somas sejam iguais, para ser comparado posteriormente ao
    resultado das colunas
    */
    int* result = malloc(sizeof(int));
    *result = sum;

    printf("[%d] Thread %d finalizou a verificacao de suas linhas e diagonais\n", threadId, threadId);

    return (void*) result;
}

int main() {
    printf("\nComecando programa...\n");

    //Carregar arquivo na memoria
    FILE* ptrRead = fopen("questao5.txt", "r");
    if (ptrRead == NULL) {
        printf("Arquivo 'questao5.txt' nao encontrado.\n");
        return 1;
    }
    else printf("Carregando matriz na memoria\n");

    //Obter tamanho da matriz quadrada
    tamanhoMatriz = malloc(sizeof(int));
    int tamanho = 1;
    char c;
    while (fscanf(ptrRead, "%c", &c) == 1 && c != '\n') {
        if (c == ' ') tamanho++;
    }
    rewind(ptrRead);
    *tamanhoMatriz = tamanho;
    printf("O tamanho da matriz quadrada e %d!\n", tamanho);

    //Criar variavel da matriz
    matriz = criarMatriz(tamanho);
    if (matriz == NULL) {
        printf("Falha na alocacao da matriz.\n");
        free(tamanhoMatriz);
        fclose(ptrRead);
        return 3;
    }

    //Ler cada numero
    printf("Lendo matriz\n");
    for(int i=0; i<tamanho; i++) {
        for(int j=0; j<tamanho; j++) {
            fscanf(ptrRead, "%d", &matriz[i][j]);
        }
    }

    //Criacao das threads e assign do indice de cada uma
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    for(int i=0; i<NUM_THREADS; i++) {
        ids[i] = i;
    }

    //Alocacao do trabalho das threads dependendo do NUM_THREADS
    switch (NUM_THREADS) {
    //Uma só thread terá que fazer todo o trabalho
    case 1:
        if(pthread_create(&threads[0], NULL, &oneThread, &ids[0]) != 0) {
            return 1;
        }
        break;

    //A primeira thread verifica as linhas e as diagonais, enquanto a segunda verifica as colunas
    case 2:
        if(pthread_create(&threads[0], NULL, &twoThreads, &ids[0]) != 0) {
            return 1;
        }
        if(pthread_create(&threads[1], NULL, &verificaColunas, &ids[1]) != 0) {
            return 1;
        }
        break;

    /*
    A primeira thread verifica as diagonais, as threads de indice impar verificam as linhas e as pares, as colunas.
    A quantidade de linhas e colunas verificada por cada thread é *inversamente proporcional* ao NUM_THREADS
    */
    default:
        //Primeira thread
        if(pthread_create(&threads[0], NULL, &verificaDiagonais, &ids[0]) != 0) {
            return 1;
        }

        //Demais threads
        for(int i=1; i<NUM_THREADS; i++) {
            //Threads impares
            if(i%2 == 1) {
                if(pthread_create(&threads[i], NULL, &verificaLinhas, &ids[i]) != 0) {
                    return 1;
                }
            }

            //Threads pares
            else if(i%2 == 0) {
                if(pthread_create(&threads[i], NULL, &verificaColunas, &ids[i]) != 0) {
                    return 1;
                }
            }
        }
        break;
    }

    //Espera o trabalho de cada uma finalizar para dar join
    int* result[NUM_THREADS] = {0};
    for(int i=0; i<NUM_THREADS; i++) {
        if(pthread_join(threads[i], (void**) &result[i]) != 0) {
            return 1;
        }
    }

    //Logica de decisao do quadrado magico
    switch (NUM_THREADS) {
    //Para NUM_THREADS == 1
    case 1:
        if(result[0] != NULL && *result[0] == 1) printf("Sim, a matriz E um quadrado magico!!!\n");
        else printf("Nao, a matriz NAO E um quadrado magico!!!\n");
        break;

    //Para NUM_THREADS == 2    
    case 2:
        if(result[0] != NULL && result[1] != NULL && *result[0] == *result[1]) {
            printf("Sim, a matriz E um quadrado magico!!!\n");
        }
        else printf("Nao, a matriz NAO E um quadrado magico!!!\n");
        break;
    //Para NUM_THREADS >= 3
    default:
        int valid = 1;

        //Verifica se algum ponteiro result eh NULL
        for(int i=0; i<NUM_THREADS; i++) {
            if(result[i] == NULL) {
                valid = 0;
                break;
            }
        }

        //Verifica se todos os ponteiros result tem mesmo valor
        for(int i=0; i<NUM_THREADS && valid == 1; i++) {
            for(int j=0; j<i; j++) {
                if(*result[j] != *result[i]) {
                    valid = 0;
                    break;
                }
            }
        }

        if(valid == 1) printf("Sim, a matriz E um quadrado magico!!!\n");
        else printf("Nao, a matriz NAO E um quadrado magico!!!\n");
        break;
    }

    //Fechar arquivos, frees e encerrar programa
    for(int i=0; i<NUM_THREADS; i++) {
        if(result[i] != NULL) free(result[i]);
    }
    free(tamanhoMatriz);
    freeMatriz(matriz, tamanho);
    fclose(ptrRead);
    printf("Terminando programa...\n");
    return 0;
}

/*
LOGICA DE FUNCIONAMENTO PARA N THREADS:

1 - diagonais, linhas e colunas
2 - diagonais, linhas
    colunas
3 - diagonais
    linhas
    colunas
4 - diagonais
    linhas
    colunas
    linhas
5 - diagonais
    linhas
    colunas
    linhas
    colunas
...
*/