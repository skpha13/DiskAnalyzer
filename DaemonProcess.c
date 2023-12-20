#include "headers/IPC.h"
#include "headers/task_symbols.h"
#include "headers/thread_arg.h"
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
#include <stdbool.h>
#include <dirent.h>

#define FORMAT_LOG_ERROR "[ERROR] %d \n"
#define THREAD_POOL_SIZE 5
/*handle prompt sterge TOT dupa ce l-a citit, filozofia este ca dc am primit mezajul este doar pt mine*/
/*de ex readPath(c) si dupa deletePath(c)*/

pthread_mutex_t mutex_pq;
//pthread_cond_t cond_pq;

bool isValidPath(const char* path) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        return false;
    }

    return true;
}

void add_task(priority_queue *pq, daemon_file_t* file, task_holder *h){

    int i = 0;
    int index = check_path(h, file->path_to_analize, &i);
    if(index != PATH_ALRD_INCLUDED)
    	index = add_id(h,file->path_to_analize);
    char *priority;
    switch(file->priority){
    	case 1:
    		priority = "low";
    		break;
    	case 2:
    		priority = "normal";
    		break;
    	case 3:
    		priority = "high";
    		break;
    	
    }
    
    char res[1500000];
    
    switch(index){
    	case NO_EMPTY_IDS:
    		file->error=TOO_MANY_TASKS;
    		break;
    	case PATH_ALRD_INCLUDED:
    		snprintf(res,1500000,"Directory ’%s’ is already included in analysis with ID ’%d’",file->path_to_analize, i);
    		break;
    	default : 
    		file->task_id = index;
    		task_struct *ts = (task_struct*)malloc(sizeof(task_struct));
    		ts->task_type = ADD_TASK;
    		ts->priority = file->priority;
    		ts->task_id = index; 
	    	insert_pq(pq,ts);
	    	snprintf(res,1500000,"Created analysis task with ID ’%d’ for ’%s’ and priority ’%s’.",index,file->path_to_analize,priority);
    		break;
    }
    strcpy(file->path_to_analize,res);
       	
}

void suspend_task(int id,daemon_file_t* file){

}


void resume_task(int id,daemon_file_t* file){

}

void remove_task(int id,daemon_file_t* file){

}

void info_task(int id,daemon_file_t* file){

}

void list_tasks(daemon_file_t* file){

}

 // task must be done 
void print_done_task(int id, daemon_file_t* file){

}    



int handle_prompt(daemon_file_t*  file, priority_queue *pq, task_holder *h){

	switch(file->task_type){
		case ADD_TASK:
			if(isValidPath(file->path_to_analize))
				add_task(pq,file,h);
			else
				file->error = INVALID_PATH;
			print(h);
			printf("1\n");
			break;
		case SUSPEND_TASK:
			printf("2\n");
			break;
		case RESUME_TASK:
			printf("3\n");
			break;
		case REMOVE_TASK:
			printf("4\n");
			break;
		case PROMPT_TASK_INFO:
			printf("5\n");
			break;
		case LIST_TASKS:
			printf("6\n");
			break;	
		case PROMPT_REPORT:
			printf("Te pup\n");	
			break;
	
	
	}
	
	
	return -1;
}

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
	
	
	if(pthread_mutex_init(&mutex_pq, NULL)) {
        perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    	}

	/*
    	// Initiate the cond
    	if(pthread_cond_init(&cond_pq,NULL)){
        	perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
    	}*/
	
	com_file->error = 0;
	com_file->task_type=0;
	com_file->task_id=0;
	*p_daemon_file=com_file;
	return EXIT_SUCCESS;
}

void functia_lui_Adi( /* parametrii */){

}

void* startThread(void* args){

    priority_queue *pq = ((thread_arg*)args)->pq;
    task_holder *h = ((thread_arg*)args)->h;
    while(1){
        task_struct* task;

        pthread_mutex_lock(&mutex_pq);

	
        while(pq->size == 0){
            // Wait to receive a task
            sleep(1);
            
            //nu cred ca mai e nevoie de urmatoarea linie, functia aia permite altor threaduri sa isi continue rularea
            //cu speranta ca pq->size va creste, dar noi avem deja thread-ul principal care ruleaza incontinuu si el face modificarile
            //in pq 
          
            //pthread_cond_wait(&cond_pq, &mutex_pq);
            
        }
        // Take the next task
        task = top_pq(pq);
        char *path = h->id_path[task->task_id];
        pop_pq(pq);

        pthread_mutex_unlock(&mutex_pq);

        //execute thread  here
        
        //aici vom apela functia lui Adi in care parcurgem recursiv directorul
        functia_lui_Adi(/* path, restul de arg daca mai sunt */);
        

    }

}

int main(){
	
	pthread_t thread_pool[THREAD_POOL_SIZE];

	// Create the priority queue of tasks
	priority_queue* pq;

	task_holder h;
	// Initiate pq
	if(initiate_pq(&pq) < 0){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}
    
    	// Initiate task_holder
    	initiate_holder(&h);


	// opening shared memory and initialize 
	daemon_file_t *communication_file;
	int status=open_and_initialize_shm(&communication_file);
	if(status==EXIT_FAILURE) 
		return EXIT_FAILURE;
	
	thread_arg t_arg;
	t_arg.pq=pq;
	t_arg.h=&h;
	for(int i = 0 ; i < THREAD_POOL_SIZE ; i++){
		if(pthread_create(&thread_pool[i], NULL, &startThread, &t_arg) != 0){
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

			handle_prompt(communication_file, pq, &h);
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
	
	return 1;
}
