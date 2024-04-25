#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        if(q->size < MAX_QUEUE_SIZE)
        {
                q->proc[q->size] = proc;
                q->size ++;
        }
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if(q->size > 0){
                uint32_t Max = q->proc[0]->priority;
                int position = 0;
                for(int i = 1; i < q->size; i++){
                        if(q->proc[i]->priority > Max){
                                Max = q->proc[i]->priority;
                                position = i;
                        }
                }
                struct pcb_t* res = q->proc[position];
                q->proc[position] = NULL;
                for(int i = position; i < q->size; i++){
                        q->proc[i] = q->proc[i+1];
                }
                q->size--;
                return res;
        }else{return NULL;}
}

