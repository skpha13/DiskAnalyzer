#include <stdio.h>
#include "headers/priority_queue.h"

int main(){
    priority_queue* pq;

    if(initiate_pq(&pq) < 0)
        return 1;

    int n = 10;
    task_struct** ts = (task_struct**)malloc(n * (sizeof(task_struct*)));
    for(int i = 0 ; i < n ; i++){
        ts[i] = (task_struct*)malloc(sizeof(task_struct));
        /*atribuie id*/
        ts[i]->task_id = i;
    
    }
    /*ii dau lu 0 la caterinca typeu DELETED sa vedem ce face*/
    ts[0]->task_type = 4;

    /*afiseaza id-urile taskurilor*/
    for(int i = 0 ; i < n ; i++){
        /*atribuie prioritatea*/
        ts[i]->priority = i % 3 + 1 ;
        insert_pq(pq,ts[i]);
        printf("%d ",ts[i]->task_id);
    }
    printf("\n");

    /*afiseaza prioritatea*/
    for(int i = 0 ; i < n ; i++){
        printf("%d ",ts[i]->priority);
    }

    printf("\n");
    
    task_struct* ret = suspend_task(pq,0);
    
    if(ret != NULL)
        printf("%d\n", ret->priority);


}