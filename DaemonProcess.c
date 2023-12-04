#include "headers/IPC.h"
#include "headers/task_symbols.h"
#include<pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include<unistd.h>
#include<errno.h>
#include<syslog.h>
#include<stdlib.h>
#include<semaphore.h>
#define FORMAT_LOG_ERROR "[ERROR] %d \n"
#define THREAD_POOL_SIZE 5
#define SHARED_MEMORY_NAME "/DiskAnalizerSHM"


int main(){
pthread_t thread_pool[THREAD_POOL_SIZE];
// opening shared memory and initialize 
openlog("Logs",LOG_PID,LOG_USER);
int shared_memory=shm_open(SHARED_MEMORY_NAME,O_CREAT | O_RDWR|O_EXCL , S_IRUSR | S_IWUSR );
if(shared_memory<0){
	perror(NULL);
	syslog(LOG_INFO,FORMAT_LOG_ERROR,errno);
	closelog();
	return EXIT_FAILURE;
}

size_t shared_memory_size=sizeof(daemon_file_t);

if ( ftruncate ( shared_memory, shared_memory_size ) == -1) {
        perror ( NULL );
        syslog(LOG_INFO,FORMAT_LOG_ERROR,errno);
        shm_unlink(SHARED_MEMORY_NAME);
		closelog();
		return EXIT_FAILURE;
    }

daemon_file_t* communication_file=mmap(0,shared_memory_size,PROT_READ|PROT_WRITE,MAP_SHARED,shared_memory,0);

	

/*to do: add instruction queue*/
while (1)
{
	sleep(1);

}



}