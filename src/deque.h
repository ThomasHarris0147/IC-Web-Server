#ifndef DEQUE_H
#define DEQUE_H

struct deque{
    int start;
    int end;
    int size;
    int max_size;
    long *jobs;
};

struct deque* deque_create(int number);
void deque_print(struct deque* deque);
struct deque* deque_pop(struct deque* deque);
struct deque* deque_add(struct deque* deque, long job);
void deque_free(struct deque* deque);
struct deque* deque_pushfront(struct deque* deque, long pos);
int deque_checkpos(struct deque* deque, long pos);

#endif /* DEQUE_H */