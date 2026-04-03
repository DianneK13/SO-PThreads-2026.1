#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LINES 7 //quantidade de linhas da tabela
#define THREADS 3 //quantidade de threads
#define ATUALIZACOES 3 //quantidades de atualizações por arquivo

typedef struct inputData{
  FILE *fptr; //ponteiro pra arquivo
  char input[30]; //nome do arquivo de entrada
}inputData;

char chart[LINES][30]; //tabela das informações dos trens (Array de strings)
pthread_mutex_t mutex[7]; //um mutex por linha da tabela

void printarTabela(){
  printf("\e[0;40;1m%s\e[0m\n", chart[0]);
  printf("\e[0;41;1m%s\e[0m\n", chart[1]);
  printf("\e[0;42;1m%s\e[0m\n", chart[2]);
  printf("\e[0;43;1m%s\e[0m\n", chart[3]);
  printf("\e[0;44;1m%s\e[0m\n", chart[4]);
  printf("\e[0;45;1m%s\e[0m\n", chart[5]);
  printf("\e[0;46;1m%s\e[0m\n", chart[6]);
  printf("\n");
}

//atualiza e printa a tabela com as informações novas
void *updateChart(void *args){
  (*(inputData*)args).fptr = fopen((*(inputData*)args).input, "r"); //acessa o arquivo de entrada

  if((*(inputData*)args).fptr == NULL){
    printf("arquivo não encontrado\n");
  }
  else{
    int numLinha;
    char info[30];

    for(int j=0; j<ATUALIZACOES; j++){ //fazer isso até ter lido o arquivo todo
      fscanf((*(inputData*)args).fptr, "%d", &numLinha); //verifica qual é a linha que vai ser alterada
      numLinha -= 1; //subtrair 1 pra ficar com o index correto

      pthread_mutex_lock(&mutex[numLinha]); //caso comece a modificar uma linha, bloqueia ela
      
      fscanf((*(inputData*)args).fptr, " %30[^\n]", info); //pega nova informação
      strcpy(chart[numLinha], info); //atualiza tabela
      printf("LINHA MODIFICADA: %d\n", numLinha + 1);
      printf("NOVA INFORMAÇÃO: %s\n", info);
      printarTabela();
      sleep(2); //espera dois segundos até que a linha possa ser modificada de novo
      
      pthread_mutex_unlock(&mutex[numLinha]); //desbloqueia linha

    }
  }
  pthread_exit(NULL);
}

int main(int argc, int *argv[]){
  pthread_t threads[THREADS]; //array de threads. Cada thread é responsável por um arquivo
  inputData *args[THREADS]; //argumentos que cada thread precisará pra tratar seus arquivos

  //inicializar argumentos
  int i;
  for(i=0; i<THREADS; i++){
    args[i] = (inputData*) malloc(sizeof(inputData));
    if(args[i] == NULL){
      printf("erro no malloc\n");
    }
  }
  strcpy(args[0]->input, "Q2_arquivo1.txt");
  strcpy(args[1]->input, "Q2_arquivo2.txt");
  strcpy(args[2]->input, "Q2_arquivo3.txt");

  //inicializar tabela
  strcpy(chart[0], "AQT123 Istambul    17:45");
  strcpy(chart[1], "XYZ001 Nice        17:00");
  strcpy(chart[2], "ABC789 Moscou      18:20");
  strcpy(chart[3], "FFF305 Estocolmo   18:35");
  strcpy(chart[4], "QRS111 Madri       18:45");
  strcpy(chart[5], "DEF321 Berlim      19:00");
  strcpy(chart[6], "GHI456 St. Petsbu. 19:15");

  //inicializar mutexes
  pthread_mutex_init(&mutex[0], NULL);
  pthread_mutex_init(&mutex[1], NULL);
  pthread_mutex_init(&mutex[2], NULL);
  pthread_mutex_init(&mutex[3], NULL);
  pthread_mutex_init(&mutex[4], NULL);
  pthread_mutex_init(&mutex[5], NULL);
  pthread_mutex_init(&mutex[6], NULL);
  
  printf("TABELA INICIAL:\n");
  printarTabela();

  //criar as threads 
  for(i=0; i<THREADS; i++){
    printf("criando thread %d...\n", i);
    if(pthread_create(&threads[i], NULL, &updateChart, args[i]) != 0){
      printf("erro no thread create\n");
      return 1;
    }
    printf("thread %d criada\n", i);
  }

  //aguardar as threads acabarem
  for(i=0; i<THREADS; i++){
    if(pthread_join(threads[i], NULL) != 0){
      printf("erro no thread join\n");
      return 1;
    }
    printf("thread %d finalizada\n", i);
  }
  
  //desalocar mutexes
  pthread_mutex_destroy(&mutex[0]);
  pthread_mutex_destroy(&mutex[1]);
  pthread_mutex_destroy(&mutex[2]);
  pthread_mutex_destroy(&mutex[3]);
  pthread_mutex_destroy(&mutex[4]);
  pthread_mutex_destroy(&mutex[5]);
  pthread_mutex_destroy(&mutex[6]);

  //free na memória alocada 
  for(i=0; i<THREADS; i++){
    free(args[i]);
  }

  return 0;
}

/*
ANSI
  exemplo: printf("\e[0;40;1mTEXTO\e[0m\n");
  sempre tem o \e[0; no inicio
  segundo número: código da cor
    (30 a 37 muda cor do texto, 40 a 47 muda a cor do fundo)
  terceiro número: modos (itálico, sublinhado, etc. 1 é o default)
  "m" antes do texto
  \e[0m depois do texto é pra resetar as coisa pro default
*/