#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_CABINES 3
#define NUM_CARROS 10
#define LIVRE 0
#define OCUPADO 1

//cada carro (thread) possui seu número (usado nos prints e na hora de decidir pra qual cabine ele vai)
typedef struct carro{
  pthread_t thread;
  int num;
}carro;

pthread_mutex_t mutex[NUM_CABINES]; //um mutex por cabine pra garantir que só um carro entre
pthread_cond_t cond[NUM_CABINES]; //condição enviada aos carros quando a cabine i estiver livre
int estadoCabines[NUM_CABINES]; //estado de cada cabine (criada pra fazer o while do wait)

void *pagarPedagio(void *arg){
  int num = *(int*)arg; //número do carro
  int cabine = num % NUM_CABINES; //usar número do carro pra decidir pra qual cabine ele vai
  
  pthread_mutex_lock(&mutex[cabine]);  //bloqueia o mutex antes de fazer as alterações

  while(estadoCabines[cabine] == OCUPADO){
    pthread_cond_wait(&cond[cabine], &mutex[cabine]);   //esperar sua respectiva cabine ser liberada
    //wait é equivalente a dar unlock no mutex, esperar o sinal de outra thread e depois dar lock novamente
  }

  //se chegou aqui, significa que a cabine ta livre.
  printf("carro %d entrou na cabine %d\n", num, cabine);       
  estadoCabines[cabine] = OCUPADO; 

  sleep(3);  //espera um tempo pra simular o pagamento                       

  estadoCabines[cabine] = LIVRE;

  printf("carro %d pagou o pedagio e liberou a cabine %d\n", num, cabine);

  pthread_cond_broadcast(&cond[cabine]); //avisa os outros carros aguardando a mesma cabine
  pthread_mutex_unlock(&mutex[cabine]);  //libera cabine  

  pthread_exit(NULL);
}

int main(int argc, int *argv[]){
  carro carros[NUM_CARROS];
  int i;

  //inicializar mutexes, variáveis condicionais e o array do estado das cabines
  for(i=0; i<NUM_CABINES; i++){
    estadoCabines[i] = LIVRE;
    pthread_mutex_init(&mutex[i], NULL);
    pthread_cond_init(&cond[i], NULL);
  }
   
  //inicializar threads
  for(i=0; i<NUM_CARROS; i++){
    carros[i].num = i; //armazenar número do carro pra ser passado como argumento

    if(pthread_create(&carros[i].thread, NULL, &pagarPedagio, &carros[i].num) != 0){ 
      printf("erro na criação\n");
      return 1;
    }
  }

  //aguardar threads acabarem
  for(i=0; i<NUM_CARROS; i++){
    if(pthread_join(carros[i].thread, NULL) != 0){
      printf("erro no join\n");
      return 1;
    }
  }

  //desalocar mutexes e variáveis condicionais
  for(i=0; i<NUM_CABINES; i++){
    pthread_mutex_destroy(&mutex[i]);
    pthread_cond_destroy(&cond[i]);
  }

  return 0;
}