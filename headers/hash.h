#include<string.h>
#include<stdio.h>

#define MAX_SIZE_PATH 10000
#define NR_STRINGS 10000  //got seg fault on bigger
#define INVALID_ID 0
#define PATH_ALRD_REMOVED -1
#define NO_EMPTY_IDS -2
#define PATH_ALRD_INCLUDED -3


typedef struct task_holder{
	char *id_path[MAX_SIZE_PATH];
	int index, need_searching, backup_index, backup[1000];
}task_holder;



void initiate_holder(task_holder *h){
	for(int i=0;i<NR_STRINGS;i++)
		h->id_path[i]=NULL;
	h->index=0;
	h->need_searching=0;
	h->backup_index=-1;
	for(int i=0;i<1000;i++)
		h->backup[i]=-1;
}

int make_bigger(task_holder *h){
	
	for(int i=0;i<NR_STRINGS;i++){
		if(!h->id_path[i]){
			if(h->backup_index<1000 && h->backup[h->backup_index]==-1)
				h->backup[h->backup_index++]=i;
			else
				if(h->backup_index >= 1000)
					break;		
		}			
	}
	return h->backup_index;
	
}

int check_path(task_holder *h, const char *path, int *index){

	for(int i=0;i<NR_STRINGS;i++){
		if(h->id_path[i] && !strcmp(h->id_path[i],path)){
			*index=i;
			return PATH_ALRD_INCLUDED;
		}
	}
	return -1;
	
}

int add_id(task_holder *h, char *path){

	if(!h->need_searching){
		
		h->id_path[h->index++]=strdup(path);
		
		if(h->index >= NR_STRINGS)
			h->need_searching = 1;
		return h->index-1;
	}else{
		if(h->backup_index==-1)
			if(make_bigger(h)!=-1){
				h->id_path[h->backup[h->backup_index]]=strdup(path);
				h->backup[h->backup_index--]=-1;
				return h->backup_index+1;
			}else
				return NO_EMPTY_IDS;
	}	
}


int remove_id(task_holder *h, int id){

	if(NR_STRINGS<=id || id<0)
		return INVALID_ID;
	if(!h->id_path[id])
		return PATH_ALRD_REMOVED;
	
	h->id_path[id]=NULL;	
	return 1;

}


void print(task_holder *h){
	for(int i=0;i<NR_STRINGS;i++)
		if(h->id_path[i])
			printf("%d  %s\n",i,h->id_path[i]);
}







