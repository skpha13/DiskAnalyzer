#include <stdlib.h>      // For free function
#include "task_struct.h"

/* Max number of elements of the pq
   The first element of the array will be excluded */
#define MAX_SIZE_PQ 101

//Apology letter for the horrors below

/* A priority_queue of task_struct*,
   Didn't use the actual objects to avoid
   using operator= on swaps and copies of 
   objects ,and easier object passing
*/

/* The contents of the pq must be allocated 
   with malloc and inserted by that pointer
*/


typedef struct{
    task_struct **tasks;   //pointer to an array pointers of tasks
    int size;               //number of tasks contained
}priority_queue;


//METHODS

/* Will return 0 on success and -1 on failure*/
int initiate_pq(priority_queue** pq);

/* Inserts an element into the pq */
void insert_pq(priority_queue* pq, task_struct* task);

/* Pops the root and repairs the pq property*/
void pop_pq(priority_queue* pq);

/* Returns the first max unmarked element */
task_struct* top_pq(priority_queue* pq);

/* Frees the memory used*/
void free_pq(priority_queue* pq);


//HELPERS
void swap_task(task_struct* a, task_struct* b);
int father_pq(int node_index);
int left_son_pq(int node_index);
int right_son_pq(int node_index);
void sift_pq(priority_queue* pq, int node);
void percolate_pq(priority_queue* pq, int node);
void lazy_deletion_pq(priority_queue* pq);


/////////////DEFINITIONS/////////////
void swap_task(task_struct* a, task_struct* b){
    task_struct temp = *a;
    *a = *b;
    *b = temp;
}

int father_pq(int node_index){
    return node_index / 2;
}

int left_son_pq(int node_index){
    return node_index * 2;
}

int right_son_pq(int node_index){
    return node_index * 2 + 1;
}

/* Used in the pop_pq function to lower the last element
    after it was swaped with the root*/
void sift_pq(priority_queue* pq, int node){
    int son;
    do{
        son = 0;
        if(left_son_pq(node) <= pq->size){
            son = left_son_pq(node);
            if( right_son_pq(node) <= pq->size && 
                pq->tasks[right_son_pq(node)]->priority > pq->tasks[left_son_pq(node)]->priority ){
                    son = right_son_pq(node);
            }
            if(pq->tasks[son]->priority <= pq->tasks[node]->priority){
                son = 0;
            }
        }

        if(son){
            swap_task(pq->tasks[son], pq->tasks[node]);
            node = son;
        }
    }while(son);
}

/* Used in the insert_pq function to lift an element up*/
void percolate_pq(priority_queue* pq, int node){
    task_struct* key = pq->tasks[node];

    while((node > 1) && (key->priority >  pq->tasks[father_pq(node)]->priority)){
        pq->tasks[node] = pq->tasks[father_pq(node)]; 
        node = father_pq(node);
    }
    pq->tasks[node] = key;
}


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
    (*pq)->tasks = (task_struct**)malloc(MAX_SIZE_PQ * sizeof(task_struct*));
   
    // Check if memory is allocated for the array
    if((*pq)->tasks == NULL){
        return -1;
    }
    
    for(int i = 0 ; i < MAX_SIZE_PQ ; i++){
        
        // Allocate every pointer of tasks
        (*pq)->tasks[i] = (task_struct*)malloc(sizeof(task_struct));
        
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

void insert_pq(priority_queue* pq, task_struct* task){
    pq->size = pq->size + 1;
    pq->tasks[pq->size] = task;
    percolate_pq(pq, pq->size);
}

/* Deletes the marked elements when they
   get on top*/
void lazy_deletion_pq(priority_queue* pq){

    // task_type 4 means the task was deleted
    while(pq->tasks[1]->task_type == 4 && pq->size > 0){

        if(pq->size == 1){
            pq->size = pq->size - 1;
            return;
        }
        // Swap first and last elements
        swap_task(pq->tasks[1], pq->tasks[pq->size]);
        
        // Update size
        pq->size = pq->size - 1;

        // Fix the pq property
        sift_pq(pq,1);      

    }
}

void pop_pq(priority_queue* pq){
    // if there are no elements,do nothing
    if(pq->size <= 0)
        return;
    
    lazy_deletion_pq(pq);
    
    if(pq->size == 1){
        pq->size = pq->size - 1;
        return;
    }
    // Swap first and last elements
    swap_task(pq->tasks[1], pq->tasks[pq->size]);
        
    // Update size
    pq->size = pq->size - 1;

    // Fix the pq property
    sift_pq(pq,1);      
    
}   

task_struct* top_pq(priority_queue* pq){

    lazy_deletion_pq(pq);

    // Return NULL if there are no elements
    if(pq->size <= 0)
        return  NULL;
    // After deleting the marked nodes,return the top
    return pq->tasks[1];

}


void free_pq(priority_queue* pq){
    
    // Frees every single pointer of the array
    for(int i = 0 ; i < MAX_SIZE_PQ ; i++)   
            free(pq->tasks[i]);

    // Frees the pointer to the array
    free(pq->tasks);

    // Frees the priority queue
    free(pq);
}
