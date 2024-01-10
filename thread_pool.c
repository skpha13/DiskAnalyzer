#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>

#include "headers/priority_queue.h"

#define THREAD_POOL_SIZE 5
#define FORMAT_LOG_ERROR "[ERROR] %d \n"


pthread_mutex_t mutex_pq;
pthread_cond_t cond_pq;
priority_queue* pq;

void submit_task(task_struct* task){
    pthread_mutex_lock(&mutex_pq);
    
    insert_pq(pq,task);
    
    pthread_mutex_unlock(&mutex_pq);

    // Signal that a task was inserted
    pthread_cond_signal(&cond_pq);
}



void test(){
    printf("cox\n");
}
/// ADI,IN LOC DE TEST ITI PUI FUNCTIA
/// IN TASK AI: id,priority,deletede
/// CRED CA ARGUMENTELE FUNCITIEI LE LUIA
/// DIN CEVA HASH DUPA ID, nu sunt sigur
void execute_task(task_struct* task){
    test();
}


void* startThread(void* args){

    while(1){
        task_struct* task;

        pthread_mutex_lock(&mutex_pq);

        while(pq->size == 0){
            // Wait to receive a task
            pthread_cond_wait(&cond_pq, &mutex_pq);
        }

        // Take the next task
        task = top_pq(pq);
        pop_pq(pq);

        pthread_mutex_unlock(&mutex_pq);

        // Acually do the task
        execute_task(task);

    }

}

int main(){
    pthread_t thread_pool[THREAD_POOL_SIZE];
    
    // Initiate the mutex
    if(pthread_mutex_init(&mutex_pq, NULL)) {
        perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

    // Initiate the cond
    if(pthread_cond_init(&cond_pq,NULL)){
        perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

    // Initiate pq
    if(initiate_pq(&pq) < 0){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

    // Create threads
    for(int i = 0 ; i < THREAD_POOL_SIZE ; i++){
        if(pthread_create(&thread_pool[i], NULL, &startThread, NULL) != 0){
            perror(NULL);
            syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
            closelog();
            return EXIT_FAILURE;
        }
    }

// ************SUBMIT TASKS****************

    /// ASTA O SA MEARGA IN WHILE TRUE DIN MAIN
    /// DUPA CE SE CREEAZA TASKUL
    for(int i = 0 ; i < 2 ; i++){

        // CAND CREEZI UN TASK TREBUIE NEAPARAT CU MALLOC
        task_struct* t = (task_struct*)malloc(sizeof(task_struct));
        /// niste valori irelevante pt test
        t->priority = i % 3 + 1;
        t->task_id = i + 100;
        t->deleted = 0;
        submit_task(t);
    }
// *************************************

    // Join threads
    for(int i = 0 ; i < THREAD_POOL_SIZE ; i++){
        if(pthread_join(thread_pool[i], NULL) != 0){
            perror(NULL);
            syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
            closelog();
            return EXIT_FAILURE;
        }
    }

    // Destroy the mutex and the condional
    pthread_mutex_destroy(&mutex_pq);
    pthread_cond_destroy(&cond_pq);

    free_pq(pq);
    return 0;
}