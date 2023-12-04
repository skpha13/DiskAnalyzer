#include "headers/IPC.h"
#include "headers/task_symbols.h"
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
#define THREAD_POOL_SIZE 5


int handle_prompt(daemon_file_t*  file){
	return -1;
}

int open_and_initialize_shm(daemon_file_t* com_file){
	openlog("Logs",LOG_PID,LOG_USER);

	/* WARNING: DE STERS  */shm_unlink(DAEMON_INPUT_FILE);
	int shared_memory = shm_open(DAEMON_INPUT_FILE, O_CREAT|O_RDWR|O_EXCL ,S_IRUSR|S_IWUSR);
	printf("shared_memory: %d\n",shared_memory);

	
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

	if(sem_init(&com_file->shell_continue, 0, 0)){
		perror(NULL);
		syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
		closelog();
		return EXIT_FAILURE;
	}
	
	com_file->error = 0;
	
	closelog();
	return 0;
}

int main(){
	
	pthread_t thread_pool[THREAD_POOL_SIZE];

	// opening shared memory and initialize 
	daemon_file_t *communication_file;
	open_and_initialize_shm(communication_file);

	openlog("Logs", LOG_PID, LOG_USER);
	while (1){
		/*IPC with da shell*/
		
		pthread_mutex_lock(&communication_file->acces_file);
		
		if(communication_file->task_id != 0){

			handle_prompt(communication_file);
			communication_file->task_id = 0;

			if(sem_post(&communication_file->shell_continue)){
				perror(NULL);
				syslog(LOG_INFO, FORMAT_LOG_ERROR, errno);
				closelog();
			}

		}
		pthread_mutex_unlock(&communication_file->acces_file);

		sleep(1);
		
	}
	closelog();
	
	return 1;
}