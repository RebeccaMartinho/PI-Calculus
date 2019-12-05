#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#define TIME 5000
#define PROCESSES 2

typedef struct {
    int init[PROCESSES]; //Intervalo de inicio dos processos
    int end[PROCESSES];    //Intervalo de fim dos processos
    double seconds;
} DATE;

DATE date;
double pi = 0.0;
pthread_mutex_t pi_lock;

void *calculatePI (void *arguments) {
    int processNumber = (*((int *) arguments));
    double piAux = 0.0;
    pi = 0.0;
    
    for(int i = date.init[processNumber]; i < date.end[processNumber]; i++) {
        piAux += date.seconds / (2*i + 1);
        date.seconds *= -1; 
    }
    
    //trava acesso a esta parte do código, altera pi, e destrava
    pthread_mutex_lock (&pi_lock);
    pi += piAux;
    pthread_mutex_unlock (&pi_lock);
    
    return (NULL);
}

void createThreads(int *processNumber) {
    pthread_t thread1, thread2;
    void *returnValue;
    
    pthread_mutex_init (&pi_lock, NULL); // Inicializa a variável mutex
    
    // cria e executa duas threads 
    if(pthread_create(&thread1, NULL , calculatePI, processNumber) || pthread_create(&thread2,NULL, calculatePI, processNumber)) {
        fprintf (stderr, "Error!\n");
        exit (1);
    }
    
    /* Join espera as threads terminarem, o retorno é armazenado em valorRetorno */
    if(pthread_join (thread1, &returnValue) || pthread_join (thread2, &returnValue)) {
        fprintf (stderr, "Error!\n");
        exit (1);
    }
}

int main () {  
    int readWrite[2];  // readWrite[0]-> leitura - readWrite[1]-> escrita
    pid_t pidDoPai = getpid();
    date.seconds = 4.0;
    clock_t initialTime, endTime;
    double time_spent;

    initialTime = clock(); // tempo de inicio

    if (pipe(readWrite) < 0){   //cria pipe
        fprintf(stderr, "Error!\n"); 
        return -1; 
    }

   //Criando processos
    int processNumber = -1;
    for (int n = 0; n < PROCESSES; n++) { //Criando processos 
        if(getpid() == pidDoPai) {  //se for o processo pai
            fork(); //cria processo filho
            processNumber++; //incrementa o numero do processo
            
            if(processNumber == 0) {
                date.init[processNumber] = 0;
                date.end[processNumber] = TIME/PROCESSES;
            } else {
                date.init[processNumber] = date.end[processNumber-1];
                date.end[processNumber] = date.init[processNumber] + TIME/PROCESSES;
            }
        }
    }
    
    if (getpid() == pidDoPai) { // Processo pai 
        double piSon = 0.0;
        processNumber = 0;
        close(readWrite[1]); //fecha pipe de escrita
        
        for (int n = 0; n < PROCESSES; n++) {
            read(readWrite[0], &piSon, __SIZEOF_DOUBLE__); //lê dados no pipe
            pi += piSon;
        }
        
        close(readWrite[0]); //fecha pipe de leitura
        endTime = clock();  // tempo de fim
        time_spent = ((double) (endTime - initialTime)) / CLOCKS_PER_SEC * 1000; // tempos total em milissegundos
        
        printf("\n\nValor estimado de pi = %1.50f\n\n" , pi);
        printf("Tempo gasto: %lf ms\n", time_spent);
    } else { //processo filho
        createThreads(&processNumber);
        close(readWrite[0]); //fecha pipe de leitura
        write(readWrite[1], &pi, __SIZEOF_DOUBLE__); //escreve o valor de pi no pipe
        close(readWrite[1]); //fecha pipe de escrita
    }
    return 0;
}


