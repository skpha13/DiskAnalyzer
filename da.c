#include<string.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include "headers/task_symbols.h"
#include "headers/IPC.h"

int is_digit(const char c){
    switch(c){
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
    if(is_digit(c)){
        int priority=atoi(c);
        return (priority==LOW_PRIORITY||priority==NORMAL_PRIORITY||priority==HIGH_PRIORITY);
    }
    return 0;
}

void suspend_task(int id);

void resume_task(int id);

void add_task(const char* path,int priority){
    
}

void help();

void list_tasks();
int main(int argc ,char** argv){

    if(argc==1) {
        help();
        return -1;
    }

    if(argc==2){//doar da -l este correct sintactic aici
        if(!strcmp("-l",argv[1])||strcmp("--list",argv[1])){
            list_tasks();
            return 0;
        }
        help();
        return -1;

    }

    if(argc==3){ //doar da -S <id>, da -R <id>, etc... sunt prompturi corecte aici
        if((!strcmp(argv[1],"-R")||strcmp(argv[1],"--resume"))&&(is_natural(argv[2]))){
            resume_task(atoi(argv[2]));
            return 0;
        }
        if((!strcmp(argv[1],"-a")||!strcmp(argv[1],"-add"))){
            add_task(argv[2],LOW_PRIORITY);
            return 0;
        }

        /*restul functiilor cand sunt 2 argumente aici*/
    }

    if(argc==4){
        help();
        return -1;

    }

    if(argc==5){
        if((!strcmp(argv[1],"-a")||!strcmp(argv[1],"-add"))&&(!strcmp(argv[3],"-p")||strcmp(argv[3],"--priority"))&&is_priority(argv[4])){
            int priority=atoi(argv[4]);
            add_task(argv[2],priority);
            return 0;
        }
    }

    help();
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