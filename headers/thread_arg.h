#include "priority_queue.h"
#include "hash.h"


typedef struct {
	priority_queue *pq;
	task_holder *h;
}thread_arg;
