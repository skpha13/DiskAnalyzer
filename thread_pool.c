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

        //execute thread  here
        
        //aici cred ca trebuie un switch cu toate comenzile 
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

    //submit tasks here

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

    //free_pq(pq);
    return 0;
}