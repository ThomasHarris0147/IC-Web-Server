#include <stdio.h>
#include <stdlib.h>
#include "deque.h"

struct deque* deque_create(int number){
    struct deque *d = malloc(sizeof(struct deque));
    d->end = 0;
    d->start = 0;
    d->size = 0;
    d->max_size = number;
    d->jobs = malloc(sizeof(long)*number);
    return d;
}

void deque_print(struct deque* deque){
    if (deque->size == 0) {
        printf("empty deque\n");
        return;
    }
    for(int i = 0; i < deque->size; i++){
        printf("%ld ", deque->jobs[(deque->start+i)%deque->max_size]);
    }
    printf("\n");
}

struct deque* deque_pop(struct deque* deque){
    if(deque->size == 0){
        printf("nothing to pop in deque\n");
        return deque;
    }
    deque->start = (deque->start+1)%deque->max_size;
    deque->size--;
    return deque;
}

int deque_checkpos(struct deque* deque, long pos){
    if (deque->size == 0) {
        printf("empty deque\n");
        return -1;
    }
    for(int i = 0; i < deque->size; i++){
        if (deque->jobs[(deque->start+i)%deque->max_size] == pos)
            return (deque->start+i)%deque->max_size;
    }
    return -1;
}

struct deque* deque_pushfront(struct deque* deque, long pos){
    int index;
    if ((index = deque_checkpos(deque, pos)) > -1){
        long temp = deque->jobs[deque->start];
        deque->jobs[deque->start] = deque->jobs[index];
        deque->jobs[index] = temp;
    } else {
        printf("deque doesnt have that index\n");
    }
    return deque;
}

struct deque* deque_add(struct deque* deque, long job){
    if(deque->size >= deque->max_size){
        printf("deque already full\n");
        return deque;
    }
    deque->jobs[deque->end] = job;
    deque->end = (deque->end+1)%deque->max_size;
    deque->size++;
    return deque;
}

void deque_free(struct deque* deque){
    free(deque->jobs);
    free(deque);
}