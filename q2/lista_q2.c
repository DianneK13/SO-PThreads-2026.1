#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define LINES 7 //quantidade de linhas da tabela
#define THREADS 3 //quantidade de threads (consequentemente o número de arquivos também)
#define ATUALIZACOES 3 //quantidades de atualizações por arquivo (todos os arquivos tem que ter a mesma)

typedef struct inputData{
  FILE *fptr; //ponteiro pra arquivo
  char input[30]; //nome do arquivo de entrada
  int threadID; //id da thread
}inputData;

pthread_mutex_t mutex[LINES]; //um mutex por linha da tabela, pra garantir que duas threads não vão mexer na mesma linha ao mesmo tempo
pthread_mutex_t mutexPrint; //mutex pra cada thread ter a sua vez de mexer no cursor pra fazer seus prints

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

      pthread_mutex_lock(&mutex[numLinha-1]); //caso comece a modificar uma linha, bloqueia ela
      pthread_mutex_lock(&mutexPrint);//caso uma thread começe a mexer no cursor pra realizar os prints, impede outras de fazerem o mesmo

      printf("\e[u"); //garante que o cursor sempre comece no início da tabela quando a thread começar sua atividade
      
      fscanf((*(inputData*)args).fptr, " %30[^\n]", info); //pega nova informação

      printf("\e[%dB", numLinha); //mover cursor pra linha que vai ser modificada (B desce numLinha vezes)

      printf("\e[0;4%d;1m%s\e[0m",numLinha-1, info); //atualizar linha com a cor correta
      printf(" modificado mais recentemente por thread %d\n", (*(inputData*)args).threadID); //indicar thread q modificou
      
      pthread_mutex_unlock(&mutexPrint);
      pthread_mutex_unlock(&mutex[numLinha-1]); //desbloqueia linha

      sleep(2); //espera dois segundos pras mudanças serem visíveis
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

  //inicializar mutexes
  for(i=0; i<LINES; i++){
    pthread_mutex_init(&mutex[i], NULL);
  }
  pthread_mutex_init(&mutexPrint, NULL);

  //printar tabela
  printf("\e[0;40;1mAQT123 Istambul    17:45\e[0m\n");
  printf("\e[0;41;1mXYZ001 Nice        17:00\e[0m\n");
  printf("\e[0;42;1mABC789 Moscou      18:20\e[0m\n");
  printf("\e[0;43;1mFFF305 Estocolmo   18:35\e[0m\n");
  printf("\e[0;44;1mQRS111 Madri       18:45\e[0m\n");
  printf("\e[0;45;1mDEF321 Berlim      19:00\e[0m\n");
  printf("\e[0;46;1mGHI456 St. Petsbu. 19:15\e[0m\n");
  sleep(2);

  printf("\e[%dA", LINES + 1); //move o cursor pra cima da tabela
  printf("\e[s"); //salva a posição
  
  //criar as threads 
  for(i=0; i<THREADS; i++){
    args[i]->threadID = i;
    if(pthread_create(&threads[i], NULL, &updateChart, args[i]) != 0){
      printf("erro no thread create\n");
      return 1;
    }
  }

  //aguardar as threads acabarem
  for(i=0; i<THREADS; i++){
    if(pthread_join(threads[i], NULL) != 0){
      printf("erro no thread join\n");
      return 1;
    }
    printf("\e[u");
    printf("\e[%dB", LINES+3+i);  //mover cursor pra baixo da tabela pros últimos prints
    printf("thread %d finalizada\n", i);
  }
  
  //desalocar mutexes
  for(i=0; i<LINES; i++){
    pthread_mutex_destroy(&mutex[i]);
  }
  pthread_mutex_destroy(&mutexPrint);

  //free na memória alocada 
  for(i=0; i<THREADS; i++){
    free(args[i]);
  }

  return 0;
}
