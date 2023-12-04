#include "headers/IPC.h"
#include "headers/task_symbols.h"
#include<pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#define THREAD_POOL_SIZE 5

int main(){
pthread_t thread_pool[THREAD_POOL_SIZE];


}