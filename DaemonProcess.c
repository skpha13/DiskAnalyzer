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
#include <errno.h>
#include <stdbool.h>
#include <linux/limits.h>


#define FORMAT_LOG_ERROR "[ERROR] %d \n"
#define MAX_NUMBER_OF_TASKS 500
#define MAX_NUMBER_OF_FOLDERS 800

typedef struct returnValues {
    // response_code = -1 if it failed, 0 if it succeeded, 1 if limit has reached
    int response_code;
    int numberOfFolders;
    int numberOfFiles;
    long size;
    float percentage;
} returnValues;

struct output {
    char path[PATH_MAX];
    struct returnValues data;
    float percentage;
};

typedef struct task_info {
    struct output returnOutput[MAX_NUMBER_OF_FOLDERS];
    struct returnValues running_info;
    int priority;
    int deleted;
    int suspended;
    int is_done;//daca is_done =1 atunci done, daca =2 atunci a aparut o eroare
    int running;// pentru a permte userului sa ii dea suspend cat timp vrea
} task_info;


typedef struct task_info_and_path{
    task_info * task;
    char path[PATH_MAX]; 
} task_info_and_path;


typedef struct thread_arg{
int thread_number;
char * path_to_analize;
int priority;
} thread_arg;


float calculatePercent(long size, long fullSize);
struct returnValues * folderAnalysis(task_info_and_path* arguments);


priority_queue* pq;

char * thread_buffers[MAX_NUMBER_OF_TASKS];
struct returnValues thread_responses[MAX_NUMBER_OF_TASKS];
task_info_and_path task_infos[MAX_NUMBER_OF_TASKS];
pthread_t task_threads[MAX_NUMBER_OF_TASKS];

//functie de analiza la rezultatul unui thread, copiat din Filesystem.c
void analyzeOutput(struct output returnOutput[], int pathSizeOfParent,char * buffer) {
    int numberOfFolders = returnOutput[0].data.numberOfFolders;
    int i = 1;

    while (numberOfFolders) {
        int counter = 0;
        for (; i<MAX_NUMBER_OF_FOLDERS; i++) {
            buffer+=sprintf(buffer,"|-%s/ %.2f%%\t%.1fMB\n",
                   returnOutput[i].path + pathSizeOfParent,
                   calculatePercent(returnOutput[i].data.size, returnOutput[0].data.size),
                   returnOutput[i].data.size / 1e6);

            counter += returnOutput[i].data.numberOfFolders;
            if (counter < 1) {
                i++;
                break;
            }

            counter--;
        }

        if (numberOfFolders > 1) buffer+=sprintf(buffer,"|\n");
        numberOfFolders--;
    }
}

//copie aproape perfecta de la functia main in Filesystem.c
void* thread_func(void* args){
    thread_arg * arguments =args;
    int index = arguments->thread_number;
    
    task_info_and_path* task = task_infos+index;
    task->task=malloc(sizeof(task_info));
    thread_buffers[index]=malloc(1e7);
    for(int i=0; i<MAX_NUMBER_OF_FOLDERS; i++) {
        task->task->returnOutput[i].data.response_code = 0;
        task->task->returnOutput[i].data.size = 0;
        task->task->returnOutput[i].data.numberOfFolders = 0;
        task->task->returnOutput[i].data.numberOfFiles = 0;
        task->task->returnOutput[i].percentage = 0;
    }
    task->task->returnOutput[0].percentage = 100;
    task->task->running_info.percentage = 0;

    strcpy(task->path,arguments->path_to_analize);
    task->task->is_done=0;
    task->task->running=1;
    task->task->suspended=1;
    task->task->priority=arguments->priority;
    task->task->deleted=0;
    struct returnValues *ret = folderAnalysis(task);
    if (ret->response_code == -1) {
        sprintf(thread_buffers[index],"Failed to get information about directory");
        task->task->is_done=2;
        return NULL;
    }


    thread_buffers[index]+=sprintf(thread_buffers[index],"%s/ %.2f%%\t%.1fMB\n|\n",task->task->returnOutput[0].path, 100.0f, task->task->returnOutput[0].data.size / 1e6);
    int pathSizeOfParent = strlen(task->task->returnOutput[0].path);
    analyzeOutput(task->task->returnOutput, pathSizeOfParent,thread_buffers[index]);
    task->task->is_done=1;
    return NULL;

}
bool isValidPath(const char* path) {
    return opendir(path) !=NULL;
}



void add_task(daemon_file_t * file){
    if(!isValidPath(file->path_to_analize)){
        file->error=INVALID_PATH;
        return;
    }
    if(file->next_task_id==MAX_NUMBER_OF_TASKS){
        file->error=TOO_MANY_TASKS;
        return;
    }
    int task_number=file->next_task_id;
    file->next_task_id++;
    thread_arg* args= malloc(sizeof(thread_arg));
    args->path_to_analize= malloc(PATH_MAX+1);
    strcpy(args->path_to_analize,file->path_to_analize);
    file->path_to_analize[0]=0;
    args->thread_number=task_number;
    args->priority=file->priority;
    pthread_create(task_threads+task_number,NULL,thread_func,args);

}



void remove_task(daemon_file_t * file){
    int task_num=file->task_id;
    if(file->next_task_id>=task_num||task_infos[task_num].task->deleted){
        file->error=TASK_UNFOUND;
        return;
    }

    pthread_cancel(task_threads[task_num]);
    free(thread_buffers[task_num]);
    free(task_infos[task_num].task);
    thread_buffers[task_num]=NULL;
    suspend_task(pq,task_num);
    task_infos[task_num].task->is_done=1;
    task_infos[task_num].task->deleted=1;

}

void suspend_task_daemon(daemon_file_t * file){
    int task_num=file->task_id;
    if(file->next_task_id>=task_num||task_infos[task_num].task->deleted){
        file->error=TASK_UNFOUND;
        return;
    }
    suspend_task(pq,task_num);
    task_infos[task_num].task->running=0;
    task_infos[task_num].task->suspended=1;
    

}

void resume_task(daemon_file_t * file){
    int task_num=file->task_id;
    if(file->next_task_id>=task_num||task_infos[task_num].task->deleted){
        file->error=TASK_UNFOUND;
        return;
    }
    if(task_infos[task_num].task->running==0&&!task_infos[task_num].task->is_done){
        task_infos[task_num].task->running=1;
        task_struct * t =malloc(sizeof(task_struct));
        t->deleted=0;
        t->task_id=task_num;
        t->priority=task_infos[task_num].task->priority;
        insert_pq(pq,t);
    }
}


void prompt_task_info(daemon_file_t * file){
    int task_num=file->task_id;
    if(file->next_task_id>=task_num||task_infos[task_num].task->deleted){
        file->error=TASK_UNFOUND;
        return;
    }
    if(task_infos[task_num].task->is_done==0){
        sprintf(file->path_to_analize,"In Progress,%.2f%% done\n",task_infos[task_num].task->running_info.percentage);
    }
    else{
        sprintf(file->path_to_analize,"Done\n");
    }
}


void prompt_report(daemon_file_t * file){
   int task_num=file->task_id;
    if(file->next_task_id>=task_num||task_infos[task_num].task->deleted){
        file->error=TASK_UNFOUND;
        return;
    }
    if(task_infos[task_num].task->is_done==0){
        file->error=TASK_NOT_DONE;
        return;
    } 
    sprintf(file->path_to_analize,thread_buffers[task_num]);
}

void list_tasks(daemon_file_t * file){
    char * c =file->path_to_analize;
    c+=sprintf(c,"ID PRI Path Done Status Details\n");
    for(int i=0;i<file->next_task_id;i++){
        if(!task_infos[i].task->deleted){
            char * status;
            if(task_infos[i].task->is_done){
                status="Finished";
            }
            else{
                status="Pending";
            }
            c+=sprintf(c,"%d %d %s %.2f%% Done %s %d files %d folders\n",i,
            
            task_infos[i].task->priority,task_infos[i].path,c,task_infos[i].task->running_info.numberOfFiles,
            
            task_infos[i].task->running_info.numberOfFolders);
        }
    }
}

/*handle prompt sterge TOT dupa ce l-a citit, filozofia este ca dc am primit mezajul este doar pt mine*/
/*de ex readPath(c) si dupa deletePath(c)*/
void handle_prompt(daemon_file_t*  file){
    if (file->task_type == ADD_TASK) {
        add_task(file);
    }

    if(file->task_type== REMOVE_TASK){
        remove_task(file);
    }
    if(file->task_type==SUSPEND_TASK){
        suspend_task_daemon(file);
    }
    if(file->task_type==RESUME_TASK){
        resume_task(file);
    }
    if(file->task_type==PROMPT_TASK_INFO){
        prompt_task_info(file);
    }
    if(file->task_type==PROMPT_REPORT){
        prompt_report(file);
    }
    if(file->task_type==LIST_TASKS){
        list_tasks(file);
    }
    
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
	task_struct* task_running = malloc(sizeof(task_struct));	
    task_running->task_id = -1;

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
	if(status==EXIT_FAILURE){
		return EXIT_FAILURE;

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
			
			
		}
		pthread_mutex_unlock(&communication_file->acces_file);

        task_struct* pq_task = top_pq(pq);
        if (pq_task != NULL) {
            int task_id = pq_task->task_id;
            int task_priority = pq_task->priority;

            if (task_running->task_id == -1 ||
                task_infos[task_running->task_id].task->is_done == 1 ||
                task_infos[task_running->task_id].task->running == 0)  
            {
                task_running = pq_task;
                task_infos[task_id].task->suspended = 0;

                pop_pq(pq);
            }
            else if (task_priority > task_running->priority) 
            {
                task_infos[task_running->task_id].task->suspended = 1;
                task_infos[task_id].task->suspended = 0;

                insert_pq(pq, task_running);

                pop_pq(pq);
                task_running = pq_task;
            }
        }

		sleep(1);
	}

	return 1;
}

struct returnValues * folderAnalysis(task_info_and_path* arguments) {
    task_info_and_path* get_union_taskInfo_path = arguments;
    struct task_info * taskInfo = get_union_taskInfo_path->task; 
    const char* path = get_union_taskInfo_path->path;
    static int indexOutput=0;
    static bool limitReached = false;
    static float totalPercentage = 0;
    struct returnValues *ret = malloc(sizeof(returnValues));

    while (taskInfo->suspended == true) {
        sleep(0.2);
    }


    ret->response_code = 0;
    ret->numberOfFolders = 0;
    ret->size = 0;
    ret->numberOfFiles = 0;

    if (limitReached == true) {
        ret->response_code = 1;
        return ret;
    }

    if (indexOutput >= MAX_NUMBER_OF_FOLDERS-2) {
        taskInfo->returnOutput[indexOutput].data.size = 0;
        taskInfo->returnOutput[indexOutput].data.response_code = 1;
        taskInfo->returnOutput[indexOutput].data.numberOfFolders = 0;
        taskInfo->returnOutput[indexOutput].data.numberOfFiles = 0;

        char temp[strlen(path) + strlen(taskInfo->returnOutput[0].path) + 1];
        strcpy(temp, taskInfo->returnOutput[0].path);
        strcat(temp, "...");
        strcpy(taskInfo->returnOutput[indexOutput].path,temp);

        limitReached = true;
        return ret;
    }

    

    DIR *bfs_traversal = opendir(path);

    if (bfs_traversal == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int numberFolders = 0;
    struct dirent* bfs;
    struct stat st;
    while ((bfs = readdir(bfs_traversal)) != NULL) {
        if (bfs) {
            if (strcmp(bfs->d_name, ".") == 0 || strcmp(bfs->d_name, "..") == 0 || bfs->d_name[0] == '.')
                continue;

            char temp[strlen(path) + strlen(bfs->d_name) + 1];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, bfs->d_name);
            if (stat(temp, &st) == 0 && S_ISDIR(st.st_mode)) {
                numberFolders++;
            }
        }
    }

    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory\n");
        ret->response_code = -1;
        return ret;
    }

    int saveIndex = indexOutput;
    strcpy(taskInfo->returnOutput[saveIndex].path, path);

    struct dirent* dp;
    struct stat sb;

    while ((dp = readdir(dir)) != NULL) {
        errno = 0;

        if (dp) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || dp->d_name[0] == '.')
                continue;

            char temp[strlen(path) + strlen(dp->d_name) + 1];
            strcpy(temp, path);
            strcat(temp, "/");
            strcat(temp, dp->d_name);

            if (stat(temp, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    ret->numberOfFolders++;
                    taskInfo->running_info.numberOfFolders ++;
                    indexOutput++;
                    
                    task_info_and_path* union_taskInfo_path=malloc(sizeof(task_info_and_path));
                    union_taskInfo_path->task=get_union_taskInfo_path->task;
                    if (indexOutput < MAX_NUMBER_OF_FOLDERS-2)
                        union_taskInfo_path->task->returnOutput[indexOutput].percentage = taskInfo->returnOutput[saveIndex].percentage / numberFolders;
                    
                    strcpy(union_taskInfo_path->path, temp);

                    struct returnValues *ret_subdir = folderAnalysis(union_taskInfo_path);

                    free(union_taskInfo_path);

                    if (ret_subdir->response_code == -1) {
                        ret->response_code = -1;
                        closedir(dir);
                        free(ret_subdir);
                        taskInfo->returnOutput[saveIndex].data = *ret;
                        return ret;
                    }

                    if (ret_subdir->response_code == 1) {
                        ret->numberOfFolders--;
                    }

                    ret->size += ret_subdir->size;
                    ret->numberOfFiles += ret_subdir->numberOfFiles;

                    free(ret_subdir);

                } else {
                    taskInfo->running_info.size += sb.st_size;
                    taskInfo->running_info.numberOfFiles++;

                    ret->size += sb.st_size;
                    ret->numberOfFiles++;
                }
            } else {
                perror("Failed to get file stats\n");
                ret->response_code = -1;
                closedir(dir);
                taskInfo->returnOutput[saveIndex].data = *ret;
                return ret;
            }
        } else {
            if (errno == 0) {
                closedir(dir);
                taskInfo->returnOutput[saveIndex].data = *ret;
                return ret;
            }

            closedir(dir);
            ret->response_code = -1;
            taskInfo->returnOutput[saveIndex].data = *ret;
            return ret;
        }
    }

    taskInfo->returnOutput[saveIndex].data = *ret;
    if (taskInfo->returnOutput[saveIndex].data.numberOfFolders == 0)
        taskInfo->running_info.percentage += taskInfo->returnOutput[saveIndex].percentage;
    //printf("%.2f%%\n",taskInfo->running_info.percentage);

    closedir(dir);
    return ret;
}

float calculatePercent(long size, long fullSize) {
    return (size * 100) / fullSize;
}

