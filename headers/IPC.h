#include<semaphore.h>
#include<pthread.h>

typedef struct{
    int task_type;
    int priority;
    int error;
    int task_id;
    int next_task_id;
    char  path_to_analize[1000000];
    pthread_mutex_t shell_wait, acces_file;
    sem_t shell_continue;
} daemon_file_t;


#define DAEMON_INPUT_FILE "/DiskAnalyzerInput"


