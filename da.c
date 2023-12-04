#include<string.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include "headers/task_symbols.h"
#include "headers/IPC.h"

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
    if(is_digit(*c)){
        int priority=atoi(c);
        return (priority==LOW_PRIORITY||priority==NORMAL_PRIORITY||priority==HIGH_PRIORITY);
    }
    return 0;
}

int check_args(const char *arg, const char *option1, const char *option2){
	if(!strcmp(arg,option1) || !strcmp(arg,option2))
		return 1;
	return 0;
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

void add_task(const char* path,int priority){}

void suspend_task(int id){}

void resume_task(int id){}

void remove_task(int id){}

void info_task(int id){}

void list_tasks(){}

void print_done_tasks(int id){}




int main(int argc ,char** argv){


    if(argc==2){//doar da -l este correct sintactic aici
        if(check_args(argv[1], "-l", "--list")){
            list_tasks();
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
            resume_task(atoi(argv[2]));
            return 0;
        }
        if(check_args(argv[1],"-a", "--add")){
            add_task(argv[2],LOW_PRIORITY);
            return 0;
        }

        /*restul functiilor cand sunt 2 argumente aici*/
        
        if(check_args(argv[1],"-S", "--suspend")&&(is_natural(argv[2]))){
            suspend_task(atoi(argv[2]));
            return 0;
        }
        
        if(check_args(argv[1], "-r", "--remove")&&(is_natural(argv[2]))){
            remove_task(atoi(argv[2]));
            return 0;
        }
        
        if(check_args(argv[1], "-i", "--info")&&(is_natural(argv[2]))){
            info_task(atoi(argv[2]));
            return 0;
        }
        
        if(check_args(argv[1], "-p", "--print")&&(is_natural(argv[2]))){
            print_done_tasks(atoi(argv[2]));
            return 0;
        }
        
        
        incorrect_args();
        return -1;        
        
    }

    if(argc==5){
        if(check_args(argv[1], "-a", "--add")&&check_args(argv[3], "-p", "--priority")&&is_priority(argv[4])){
            int priority=atoi(argv[4]);
            add_task(argv[2],priority);
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
