<<<<<<< HEAD
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
/*handle prompt sterge TOT dupa ce l-a citit, filozofia este ca dc am primit mezajul este doar pt mine*/
/*de ex readPath(c) si dupa deletePath(c)*/

int handle_prompt(daemon_file_t*  file){
	
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
	com_file->error = 0;
	com_file->task_type=0;
	com_file->next_task_id=0;
	com_file->task_id=0;
	*p_daemon_file=com_file;
	return EXIT_SUCCESS;
}

int main(){
	
	pthread_t thread_pool[THREAD_POOL_SIZE];

	// opening shared memory and initialize 
	daemon_file_t *communication_file;
	int status=open_and_initialize_shm(&communication_file);
	if(status==EXIT_FAILURE) 
		return EXIT_FAILURE;
	
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
	
	return 1;
}
=======
#include <stdio.h>
#include <unistd.h>	//for fork()
#include <stdlib.h>	//for exit()
#include <sys/stat.h>	//for umask
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>


// Message added in syslog later
#define LOGGING  "Start logging my task = %d\n"

int main(){
	pid_t pid;
	int x_fd;

	/* Fork off the parent process */
	pid = fork();
	
	/* Error */
	if(pid < 0)
		exit(EXIT_FAILURE);

	/* Let parent terminate */
	if(pid > 0)
		exit(EXIT_SUCCESS);

	/* On success: The child process becomes session leader and process group leader */
	if(setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Fork off the second time */
	pid = fork();

	/* Error */
	if(pid < 0)
		exit(EXIT_FAILURE);

	/* Let parent terminate */
	if(pid > 0)
		exit(EXIT_SUCCESS);

	/* Allow read, write, execute permission for the file's owner only, created by daemon */
	umask(077);


	/* Change the working directory to the root directory
	 * if the current directory is on some mounted file system,
	 * so daemon process will not let the mounted file system to unmount */
	chdir("/");

	/* Close all open file descriptors */
	for(x_fd = sysconf(_SC_OPEN_MAX) ; x_fd >= 0 ; x_fd--)
		close(x_fd);



	/* Logging Erros/Info in the syslog system */

	// log into syslog a message every second	

	 int count = 0;
	openlog("Logs", LOG_PID, LOG_USER);
	while(1){
		sleep(1);
	//	syslog(LOG_INFO, LOGGING, count++);
	}
	closelog();

	
	
	return 1;
}
>>>>>>> 34a4cf9fda4386951101d2ce7c9f4705a14dda89
