#include "headers/IPC.h"
#include "headers/task_symbols.h"
#include "headers/priority_queue.h"
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
#include <string.h>
#include <dirent.h>

#define FORMAT_LOG_ERROR "[ERROR] %d \n"
#define THREAD_POOL_SIZE 5
/*handle prompt sterge TOT dupa ce l-a citit, filozofia este ca dc am primit mezajul este doar pt mine*/
/*de ex readPath(c) si dupa deletePath(c)*/

int handle_prompt(daemon_file_t*  file){
    if (file->task_type == ADD_TASK) {
        
    }

    if(file->task_type== REMOVE_TASK){

    }
    return -1;
}


// ================= THREAD POOL =================
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
// ================= END =================

// ================= DISK ANALYSIS =================
struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded
    int response_code;
    int numberOfFolders;
    int numberOfFiles;
    long size;
};

struct output {
    char path[PATH_MAX];
    struct returnValues data;
};

// TODO: how to estimate current progress
struct task_info {
    int suspend_index;
    struct output returnOutput[PATH_MAX];
    struct returnValues running_info;
} taskInfo;

int indexOutput = 0;

void * folderAnalysis(const char* path) {
    struct returnValues* ret = malloc(sizeof(2 * sizeof(int) + sizeof(long long)));
    ret->numberOfFolders = 500;
    ret->numberOfFiles = 100;
    ret->size = 123021;
    ret->response_code = 0;

    return ret;
}
// ================= END =================

int open_and_initialize_shm(daemon_file_t** p_daemon_file){
	openlog("Logs",LOG_PID,LOG_USER);
	daemon_file_t * com_file;
	shm_unlink(DAEMON_INPUT_FILE);
	//shared memory is accesible by all users
	int shared_memory = shm_open(DAEMON_INPUT_FILE,O_CREAT | O_RDWR|O_EXCL , S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP);


	
	if(shared_memory < 0){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}

	size_t shared_memory_size = sizeof(daemon_file_t);

	if (ftruncate (shared_memory, shared_memory_size) == -1) {
			perror (NULL);
			syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
			shm_unlink(DAEMON_INPUT_FILE);
			closelog();
			return EXIT_FAILURE;
		}

	com_file = mmap(0, shared_memory_size, PROT_READ|PROT_WRITE, MAP_SHARED, shared_memory, 0);

	//daemon initialises sync devices
	if(pthread_mutex_init(&com_file->acces_file, NULL)){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}

	if(pthread_mutex_init(&com_file->shell_wait, NULL)){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}	

	if(sem_init(&com_file->shell_continue, 1, 0)){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}
	com_file->error = 0;
	com_file->task_type=0;
	com_file->next_task_id=0;
	com_file->task_id=0;
	*p_daemon_file=com_file;
	return EXIT_SUCCESS;
}

int main(){
	
	pthread_t thread_pool[THREAD_POOL_SIZE];

    // Initiate pq
    if(initiate_pq(&pq) < 0){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    }

	// opening shared memory and initialize 
	daemon_file_t *communication_file;
	int status=open_and_initialize_shm(&communication_file);
	if(status==EXIT_FAILURE) 
		return EXIT_FAILURE;

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

    // Create threads
    for(int i = 0 ; i < THREAD_POOL_SIZE ; i++){
        if(pthread_create(&thread_pool[i], NULL, &startThread, NULL) != 0){
            perror(NULL);
            syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
            closelog();
            return EXIT_FAILURE;
        }
    }
	
	while (1){
		/*IPC with da.c shell*/
	
		pthread_mutex_lock(&communication_file->acces_file);
		
		if(communication_file->task_type != 0){

			handle_prompt(communication_file);
			communication_file->task_type= 0;

			if(sem_post(&communication_file->shell_continue)){
				perror(NULL);
				syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
				closelog();
			}
			
			fflush(0);
		}
		pthread_mutex_unlock(&communication_file->acces_file);

		sleep(1);
		
	}

    // TODO: join threads

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

	return 1;
}
