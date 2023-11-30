#include<string.h>
#include<pthread.h>
#include<stdio.h>

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

    while (!c)
    {
        if(!is_digit(*c)) return 0;
        c++;
        
    }

    return 1;

}

int suspend(int id);



void help();

void listall();
int main(int argc ,char** argv){

    if(argc==1) {
        help();
        return 0;
    }

    if(argc<=2){//doar da -l este correct sintactic aici
        if(!strcmp("-l",argv[1])||strcmp("--list",argv[1])){
            listall();
            return 0;
        }
        help();
        return 0;

    }

    if(argc<=3){ //doar da -S <id>, da -R <id>, etc... sunt prompturi corecte aici
        
    }



}


/*
--help display usage message  T
-a, --add analyze a new directory path for disk usage L
-p, --priority set priority for the new analysis (works only with -a argument) T
-S, --suspend <id> suspend task with <id> T
-R, --resume <id> resume task with <id> L
-r, --remove <id> remove the analysis with the given <id> T
-i, --info <id> print status about the analysis with <id> (pending, progress, d  T
-l, --list list all analysis tasks, with their ID and the corresponding root p L
-p, --print <id> print analysis report for those tasks that are "done" T

*/