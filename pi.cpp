#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

#define TIME 5000
#define PROCESSES 2

typedef struct {
    int init[PROCESSES]; 
    int end[PROCESSES];   
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
    
    
    pthread_mutex_lock (&pi_lock);
    pi += piAux;
    pthread_mutex_unlock (&pi_lock);
    
    return (NULL);
}

void createThreads(int *processNumber) {
    pthread_t thread1, thread2;
    void *returnValue;
    
    pthread_mutex_init (&pi_lock, NULL);
    
     
    if(pthread_create(&thread1, NULL , calculatePI, processNumber) || pthread_create(&thread2,NULL, calculatePI, processNumber)) {
        fprintf (stderr, "Error!\n");
        exit (1);
    }
    
   
    if(pthread_join (thread1, &returnValue) || pthread_join (thread2, &returnValue)) {
        fprintf (stderr, "Error!\n");
        exit (1);
    }
}

int main () {  
    int readWrite[2];  
    pid_t pidDoPai = getpid();
    date.seconds = 4.0;
    clock_t initialTime, endTime;
    double time_spent;

    initialTime = clock(); 

    if (pipe(readWrite) < 0){   
        fprintf(stderr, "Error!\n"); 
        return -1; 
    }

   
    int processNumber = -1;
    for (int n = 0; n < PROCESSES; n++) { 
        if(getpid() == pidDoPai) { 
            fork(); 
            processNumber++;
            
            if(processNumber == 0) {
                date.init[processNumber] = 0;
                date.end[processNumber] = TIME/PROCESSES;
            } else {
                date.init[processNumber] = date.end[processNumber-1];
                date.end[processNumber] = date.init[processNumber] + TIME/PROCESSES;
            }
        }
    }
    
    if (getpid() == pidDoPai) { 
        double piSon = 0.0;
        processNumber = 0;
        close(readWrite[1]);
        
        for (int n = 0; n < PROCESSES; n++) {
            read(readWrite[0], &piSon, __SIZEOF_DOUBLE__); 
            pi += piSon;
        }
        
        close(readWrite[0]);
        endTime = clock();  
        time_spent = ((double) (endTime - initialTime)) / CLOCKS_PER_SEC * 1000; 
        
        printf("\n\nValor estimado de pi = %1.50f\n\n" , pi);
        printf("Tempo gasto: %lf ms\n", time_spent);
    } else {
        createThreads(&processNumber);
        close(readWrite[0]); 
        write(readWrite[1], &pi, __SIZEOF_DOUBLE__); 
        close(readWrite[1]); 
    }
    return 0;
}


