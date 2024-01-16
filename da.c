#include<string.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include "headers/task_symbols.h"
#include "headers/IPC.h"
#include<pthread.h>
#include<errno.h>
#include<semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include<unistd.h>
#include <stdbool.h>
#include <dirent.h>

int is_digit(const char c){
    switch(c){
    	case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        return 1;
        default:
        return 0;
    }
}

int is_natural(const char* c){
    if(!*c) return 0;//handle empty string

    while (*c)
    {
        if(!is_digit(*c)) return 0;
        c++;  
    }
    return 1;

}

int is_priority(const char* c){
    if(is_natural(c)){
        int priority=atoi(c);
        return priority==LOW_PRIORITY || priority==NORMAL_PRIORITY || priority==HIGH_PRIORITY;
    }
    return 0;
}

int check_args(const char *arg, const char *option1, const char *option2){
	return !strcmp(arg,option1) || !strcmp(arg,option2);
	
}

bool isValidPath(const char* path) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        return false;
    }

    return true;
}

void incorrect_args(){
	printf(
	"Unrecognized command\n"
	"Try 'da --help' for more information.\n"
	);
}

void help(){
	printf(
	"\nUsage: da OPTION... [DIR]...\n"
	"Analyze the space occupied by the directory at [DIR].\n\n"
	"   -a, --add analyze a new directory path for disk usage\n"
	"   -p, --priority set priority for the new analysis (1,2 or 3) (works only with -a argument)\n"
	"   -S, --suspend <id> suspend task with <id>\n"
	"   -R, --resume <id> resume task with <id>\n"
	"   -r, --remove <id> remove the analysis with the given <id>\n"
	"   -i, --info <id> print status about the analysis with <id> (pending, progress,...)\n"
	"   -l, --list list all analysis tasks, with their ID and the corresponding root\n"
	"   -p, --print <id> print analysis report for those tasks that are \"done\"\n"
	"   -h, --help display this help\n\n"
	);
}


/*regula: o comunicare cu daemonul poate sa adauge un singur task*/
void add_task(const char* path,int priority,daemon_file_t* daemon_input){
    
    pthread_mutex_lock(&daemon_input->shell_wait);//I am ahead of all disk analyzer shells
    pthread_mutex_lock(&daemon_input->acces_file);//I have write permissions to the shared file btwn daemon
    daemon_input->task_type=ADD_TASK;
    strcpy(daemon_input->path_to_analize,path);
    daemon_input->priority=priority;
    pthread_mutex_unlock(&daemon_input->acces_file);// I let the daemon have acces to the file
    
    sem_wait(&daemon_input->shell_continue);// I wait for the daemon to finish in order to continue
    if(daemon_input->error==TOO_MANY_TASKS){
        printf("[error] Too many analisis jobs are being done, please remove some or wait...\n");
        daemon_input->error=0;
    }
    else if(daemon_input->error==INVALID_PATH){
        printf("[error] The path specified is non existent or wrongly formatted\n");
        daemon_input->error=0;
    
    }
    else{
        printf("Added task with id %d\n",daemon_input->next_task_id-1);
    }
    pthread_mutex_unlock(&daemon_input->shell_wait);
}

void suspend_task(int id,daemon_file_t* daemon_input){
    //to do la final : dc taskul este deja terminat, sa fie avertizat utilizatorul
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);
    daemon_input->task_type=SUSPEND_TASK;
    daemon_input->task_id=id;
    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    if(daemon_input->error==TASK_UNFOUND){
        printf("[error] Task with job id %d was not given to the daemon to analyze\n",id);
        daemon_input->error=0;
    }
    else{
        printf("Suspended task with id %d\n",id);
    }
    pthread_mutex_unlock(&daemon_input->shell_wait);
}


void resume_task(int id,daemon_file_t* daemon_input){
    //to do la final: dc este deja in rulare, sa fie avertizat in shell
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);
    daemon_input->task_type=RESUME_TASK;
    daemon_input->task_id=id;
    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    if(daemon_input->error==TASK_UNFOUND){
        daemon_input->error=0;
        printf("[error] Task with job id %d was not given to the daemon to analyze\n",id);
    }
    else{
        printf("Resumed task with id %d\n", id);
    }
    pthread_mutex_unlock(&daemon_input->shell_wait);
}

void remove_task(int id,daemon_file_t* daemon_input){
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);
    daemon_input->task_type=REMOVE_TASK;
    daemon_input->task_id=id;
    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    if(daemon_input->error==TASK_UNFOUND){
        daemon_input->error=0;
        printf("[error] Task with job id %d does not exist to analyze\n",id);
    }
    else{
        printf("Removed task with id %d\n",id);
    }
    pthread_mutex_unlock(&daemon_input->shell_wait);
}

void info_task(int id,daemon_file_t* daemon_input){
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);
    daemon_input->task_type=PROMPT_TASK_INFO;
    daemon_input->task_id=id;
    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    if(daemon_input->error==TASK_UNFOUND){
        daemon_input->error=0;
        printf("[error] Task with job id %d was not given to the daemon to analyze\n",id);
    
    }
    else{
        puts(daemon_input->path_to_analize);//prin conventie informatia se pune aici
        
    }
    daemon_input->path_to_analize[0]=0;//stergem stringul
    
    pthread_mutex_unlock(&daemon_input->shell_wait);
}

void list_tasks(daemon_file_t* daemon_input){
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);

    daemon_input->task_type=LIST_TASKS;

    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    puts(daemon_input->path_to_analize);

    daemon_input->path_to_analize[0]=0;
    
    pthread_mutex_unlock(&daemon_input->shell_wait);
}

 // task must be done 
void print_done_task(int id,daemon_file_t*daemon_input){
    pthread_mutex_lock(&daemon_input->shell_wait);
    pthread_mutex_lock(&daemon_input->acces_file);
    daemon_input->task_type=PROMPT_REPORT;  
    daemon_input->task_id=id;
    pthread_mutex_unlock(&daemon_input->acces_file);
    sem_wait(&daemon_input->shell_continue);
    if(daemon_input->error==TASK_NOT_DONE){
        daemon_input->error=0;
        printf("[error] Task with id %d isn't done\n",id);
    }
    else if(daemon_input->error==TASK_UNFOUND){
        daemon_input->error=0;
        printf("[error] Task with id %d does not exist to analyze\n",id);

    }
    else{
        puts(daemon_input->path_to_analize);
        
    }
    daemon_input->path_to_analize[0]=0;
    pthread_mutex_unlock(&daemon_input->shell_wait);
}    



int get_shmared_memory(daemon_file_t ** daemon_file){
    int shared_memory=shm_open(DAEMON_INPUT_FILE,O_RDWR,S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP);
    if(shared_memory<0){
        printf("[error] errno:%d\n",errno);
        perror(NULL);
        return EXIT_FAILURE;
    }

    size_t shared_memory_size=sizeof(daemon_file_t);


    if ( ftruncate( shared_memory, shared_memory_size ) == -1) {
	perror ( NULL );
    printf("[error] errno:%d\n",errno);
    return EXIT_FAILURE;
    }

    daemon_file_t *com_file;
    com_file=mmap(0,shared_memory_size,PROT_READ|PROT_WRITE,MAP_SHARED,shared_memory,0);
    if(com_file==MAP_FAILED){
        perror(NULL);
        printf("[error] errno:%d\n",errno);
        return EXIT_FAILURE;
    }
    *daemon_file=com_file;
    return EXIT_SUCCESS;
}

int main(int argc ,char** argv){
    daemon_file_t * communication_file;
    int status=get_shmared_memory(&communication_file);
    if(status==EXIT_FAILURE) 
        return EXIT_FAILURE;

    if(argc==2){
        if(check_args(argv[1], "-l", "--list")){
            list_tasks(communication_file);
            return 0;
        }
        
        if(check_args(argv[1], "-h", "--help")){
            help();
            return 0;
        }
        
        incorrect_args();
        return -1;

    }

    if(argc==3){ //doar da -S <id>, da -R <id>, etc... sunt prompturi corecte aici
        if(check_args(argv[1], "-R", "--resume")&&(is_natural(argv[2]))){
            resume_task(atoi(argv[2]),communication_file);
            return 0;
        }
        if(check_args(argv[1],"-a", "--add")){
            add_task(argv[2],LOW_PRIORITY,communication_file);
            return 0;
        }

       
        
        if(check_args(argv[1],"-S", "--suspend")&&(is_natural(argv[2]))){
            suspend_task(atoi(argv[2]),communication_file);
            return 0;
        }
        
        if(check_args(argv[1], "-r", "--remove")&&(is_natural(argv[2]))){
            remove_task(atoi(argv[2]),communication_file);
            return 0;
        }
        
        if(check_args(argv[1], "-i", "--info")&&(is_natural(argv[2]))){
            info_task(atoi(argv[2]),communication_file);
            return 0;
        }
        
        if(check_args(argv[1], "-p", "--print")&&(is_natural(argv[2]))){
            print_done_task(atoi(argv[2]),communication_file);
            return 0;
        }
        
        
        incorrect_args();
        return -1;        
        
    }

    if(argc==5){
        if(check_args(argv[1], "-a", "--add")&&check_args(argv[3], "-p", "--priority")&&is_priority(argv[4])){
            int priority=atoi(argv[4]);
            add_task(argv[2],priority,communication_file);
            return 0;
        }
    }


    incorrect_args();
    return -1;
}


/*
--help display usage message  T
-a, --add analyze a new directory path for disk usage L
-p, --priority set priority for the new analysis (works only with -a argument) L
-S, --suspend <id> suspend task with <id> T
-R, --resume <id> resume task with <id> L
-r, --remove <id> remove the analysis with the given <id> T
-i, --info <id> print status about the analysis with <id> (pending, progress, done)  T
-l, --list list all analysis tasks, with their ID and the corresponding root p L
-p, --print <id> print analysis report for those tasks that are "done" T
*/
