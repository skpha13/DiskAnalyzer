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
#include <string.h>         /* For strcpy at assignments*/

//will delete this later and exclude unused libraries
#define FORMAT_LOG_ERROR "[ERROR] %d \n"

// Max number of elements of the pq
//  The first element of the array will be excluded
#define MAX_SIZE_PQ 101


typedef struct{
    daemon_file_t **tasks;   //pointer to an array pointers of tasks
    int size;               //number of tasks contained
}priority_queue;



/* Will return 0 on success and -1 on failure*/
int initiate_pq(priority_queue** pq){

    // Allocating memory for the pq
    *pq = (priority_queue*)malloc(sizeof(priority_queue));

    // Check if memory is allocated
    if(pq == NULL){
        return -1;
    }

    // Set the size to 0, there are no elements
    (*pq)->size = 0;

    // Allocating memory for the array
    (*pq)->tasks = (daemon_file_t**)malloc(MAX_SIZE_PQ * sizeof(daemon_file_t*));
   
    // Check if memory is allocated for the array
    if((*pq)->tasks == NULL){
        return -1;
    }
    
    for(int i = 0 ; i < MAX_SIZE_PQ ; i++){
        
        // Allocate every pointer of tasks
        (*pq)->tasks[i] = (daemon_file_t*)malloc(sizeof(daemon_file_t));
        
        // Check for errors 
        if((*pq)->tasks[i] == NULL){

            // If allocation fails for one,free all
            for(int j = 0 ; j < i ; j++)
                free((*pq)->tasks[j]);
            
            free((*pq)->tasks);
            // Return -1 for error
            return -1;
        }
    }


    // Success
    return 0;
}

////////////// HELPERS////////////////
void swap_task(daemon_file_t* a, daemon_file_t* b){
    daemon_file_t temp = *a;
    *a = *b;
    *b = temp;
}

int father(int node_index){
    node_index / 2;
}

int left_son(int node_index){
    return node_index * 2;
}

int right_son(int node_index){
    return node_index * 2 + 1;
}




/*
void sift(priority_queue* pq, int node){
    int son;
    do{
        son = 0;
        if(left_son(node) <= pq->size){
            son = left_son(node);
            if( right_son(node) <= pq->size && 
                pq->tasks[right_son(node)].priority > pq->tasks[left_son(node)].priority ){
                    son = right_son(node);
            }
            if(pq->tasks[son].priority <= pq->tasks[son].priority){
                son = 0;
            }
        }

        if(son){
            //swap_pq()
            node = son;
        }
    }while(son);
}
*/
///////////////////////////////////////

//Insert
//ExtractMin
//Top
int main(){
    // Create the priority queue of tasks
    priority_queue* pq;

    // Initiate pq
    if(initiate_pq(&pq) < 0){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

    daemon_file_t a = { .task_type = 1, 
                        .priority = 1,
                        .error = 0,
                        .task_id = 1,
                        .next_task_id = 2,
                        .path_to_analize = "AAA"};

    daemon_file_t b = { .task_type = 1, 
                            .priority = 2,
                            .error = 0,
                            .task_id = 2,
                            .next_task_id = 2,
                            .path_to_analize = "BBB"};
   
    daemon_file_t c = { .task_type = 1, 
                            .priority = 3,
                            .error = 0,
                            .task_id = 3,
                            .next_task_id = 2,
                            .path_to_analize = "CCC"};
    daemon_file_t* pta = &a;
    daemon_file_t* ptb = &b;
    daemon_file_t* ptc = &c;

    daemon_file_t** ptaa = &pta;

    *pq->tasks[1] = *pta;
    *pq->tasks[2] = *ptb;

    printf("PTA: %d PTB: %d\n",pq->tasks[1]->priority, pq->tasks[2]->priority);
    swap_task(pq->tasks[1], pq->tasks[2]);

    printf("PTA: %d PTB: %d\n",pq->tasks[1]->priority, pq->tasks[2]->priority);
}
