#include "./IPC.h"
#include "./task_symbols.h"
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdio.h>

#define FORMAT_LOG_ERROR "[ERROR] %d \n"
#define MAX_SIZE_PQ 100

typedef struct{
    daemon_file_t *tasks;
    int size;
}priority_queue;

int create(priority_queue* pq){
    // Allocating memory for the pq
    pq = (priority_queue*)malloc(sizeof(priority_queue));

    // Check if memory is allocated
    if(pq == NULL){
        perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

    pq->size = 0;
    // Allocating memory for the array
    pq->tasks = (daemon_file_t*)malloc(MAX_SIZE_PQ * sizeof(priority_queue));

    // Check if memory is allocated for the array
    if(pq->tasks == NULL){
        perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }
}
//Insert
//ExtractMin
//Top
int main(){
    priority_queue* pq;
    create(pq);

}
