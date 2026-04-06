/*
COMANDO P/ CONVERTER IMAGEM:
magick INPUT.PNG -compress none questao1_og.ppm
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 6

//Declaracao dos ponteiros necessarios na funcao executada pelas threads
int *pontLinhas, *pontColunas;
int **redMat, **blueMat, **greenMat, **greyMat; 

//Alocar matriz
int** matriz(int qntLinhas, int qntColunas) {
    int **matriz = malloc(qntLinhas * sizeof(int*));
    if (matriz == NULL) {
        printf("Falha na alocacao da matriz.\n");
        return NULL;
    }

    for (int i=0; i<qntLinhas; i++) {
        matriz[i] = malloc(qntColunas * sizeof(int));
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
void freeMatriz(int **matriz, int qntLinhas) {
    for (int i=0; i<qntLinhas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

//Adquirir tons de cinza
int getGreyScale(int red, int green, int blue) {
    int greyScale = (((red*30) + (green*59) + (blue*11))/100); 
    return greyScale;
}

//Normalizar tons
int normalize(int cor, int rangeCor) {
    if(cor<0) cor=0;
    else if(cor>rangeCor) cor=rangeCor;
    return cor;
}

//Tarefa da thread
void *routine(void *tid) {
    /*
    As threads possuem indices, que as identificam. Visando evitar condicao de corrida,
    cada thread inicia tratando todas as colunas das linhas que possuem o mesmo indice i
    que a thread. Depois, tratam todos os pixels da linha de indice i+NUM_THREADS, ate que
    toda a imagem seja convertida. Por exemplo, para 6 threads, a thread 0 tratara todas as 
    colunas das linhas 0, 6, 12, 18...
    */

    int i, j;
    int threadId = (*(int*) tid);

    for(i=threadId; i<*pontLinhas; i=i+NUM_THREADS) {
        for(j=0; j<*pontColunas; j++) {
            greyMat[i][j] = getGreyScale(redMat[i][j], greenMat[i][j], blueMat[i][j]);
        }
    }
    free(tid);

    return NULL;
}

int main() {
    //Carregar arquivo na memoria
    FILE* ptrRead = fopen("questao1_og.ppm", "r");
    if (ptrRead == NULL) {
        printf("Arquivo 'questao1_og.ppm' nao encontrado.\n");
        return 1;
    }

    //Obter os dados da imagem
    int qntLinhas, qntColunas, rangeCor;
    fscanf(ptrRead, " %*s %d %d %d", &qntColunas, &qntLinhas, &rangeCor);
    pontLinhas = malloc(sizeof(int));
    *pontLinhas = qntLinhas;
    pontColunas = malloc(sizeof(int));
    *pontColunas = qntColunas;
    
    //Criar uma matriz para cada canal de cor
    redMat = matriz(qntLinhas, qntColunas);
    if (redMat == NULL) {
        printf("Falha na alocacao da matriz vermelha.\n");
        free(pontLinhas);
        free(pontColunas);
        fclose(ptrRead);
        return 3;
    }

    greenMat = matriz(qntLinhas, qntColunas);
    if (greenMat == NULL) {
        printf("Falha na alocacao da matriz vermelha.\n");
        free(pontLinhas);
        free(pontColunas);
        freeMatriz(redMat, qntLinhas);
        fclose(ptrRead);
        return 3;
    }

    blueMat = matriz(qntLinhas, qntColunas);
    if (blueMat == NULL) {
        printf("Falha na alocacao da matriz vermelha.\n");
        free(pontLinhas);
        free(pontColunas);
        freeMatriz(redMat, qntLinhas);
        freeMatriz(greenMat, qntLinhas);
        fclose(ptrRead);
        return 3;
    }

    //Ler cor de cada canal
    for(int i=0; i<qntLinhas; i++) {
        for(int j=0; j<qntColunas; j++) {
            int red, green, blue;
            fscanf(ptrRead, " %d %d %d", &red, &green, &blue);

            red = normalize(red, rangeCor);
            green = normalize(green, rangeCor);
            blue = normalize(blue, rangeCor);

            redMat[i][j] = red;
            greenMat[i][j] = green;
            blueMat[i][j] = blue;
        }
    }
    
    //Criar matriz em grey scale
    greyMat = matriz(qntLinhas, qntColunas);
    if (greyMat == NULL) {
        printf("Falha na alocacao da matriz cinza.\n");
        free(pontLinhas);
        free(pontColunas);
        freeMatriz(redMat, qntLinhas);
        freeMatriz(greenMat, qntLinhas);
        freeMatriz(blueMat, qntLinhas);
        fclose(ptrRead);
        return 3;
    }

    //Declaracao do vetor de threads
    pthread_t threads[NUM_THREADS];

    //Criacao das threads e assign do indice de cada uma
    for(int i=0; i<NUM_THREADS; i++) {
        int* index = malloc(sizeof(int));
        *index = i;

        if(pthread_create(&threads[i], NULL, &routine, index) != 0) {
            return 1;
        }
    }

    //Espera o trabalho de cada uma finalizar para dar join
    for(int i=0; i<NUM_THREADS; i++) {
        if(pthread_join(threads[i], NULL) != 0) {
            return 1;
        }
    }
    
    //Criacao do arquivo da imagem convertida
    FILE* ptrWrite = fopen("questao1_converted.ppm", "w");
    if (ptrWrite == NULL) {
        printf("Nao foi possivel criar o arquivo 'questao1_converted.ppm'.\n");
        free(pontLinhas);
        free(pontColunas);
        freeMatriz(redMat, qntLinhas);
        freeMatriz(greenMat, qntLinhas);
        freeMatriz(blueMat, qntLinhas);
        freeMatriz(greyMat, qntLinhas);
        fclose(ptrRead);
        return 2;
    }

    //Escrita dos valores de cada pixel na imagem
    fprintf(ptrWrite, "P3\n");
    fprintf(ptrWrite, "%d %d\n", qntColunas, qntLinhas);
    fprintf(ptrWrite, "%d\n", rangeCor);

    for(int i=0; i<qntLinhas; i++) {
        for(int j=0; j<qntColunas; j++) {
            fprintf(ptrWrite, "%d %d %d\n", greyMat[i][j], greyMat[i][j], greyMat[i][j]);
        }
    }

    //Fechar arquivos, frees e encerrar programa
    free(pontLinhas);
    free(pontColunas);
    freeMatriz(redMat, qntLinhas);
    freeMatriz(greenMat, qntLinhas);
    freeMatriz(blueMat, qntLinhas);
    freeMatriz(greyMat, qntLinhas);
    fclose(ptrRead);
    fclose(ptrWrite);
    return 0;
}

/*
POSSIVEIS MELHORIAS:

-Ao finalizar o codigo, notei que a criacao de uma unica matriz tridimensional poderia
substituir o trabalho de guardar os tons da imagem, feito por 3 diferentes matrizes no
codigo atual. Entretanto, isso impediria o reaproveitamento de algumas das funcoes usadas,
alem de resultar na invalidacao dos valores de retorno e dos argumentos de outras funcoes.

-Ao inves de passar somente o indice de cada thread como argumento, uma struct que contivesse,
alem do indice, as informacoes dos ponteiros declarados no inicio do codigo poderia deixar este
mais limpo. Porem, novamente, a estrutura do codigo necessitaria ser bastante alterada, o que 
potencialmente resultaria no surgimento de novos bugs.
*/