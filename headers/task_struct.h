
/*USE MALLOC WHEN ALLOCATING task_struct* */

typedef struct{
    int priority;
    int task_id;
    int deleted;    //1->deleted, 0->still there
}task_struct;