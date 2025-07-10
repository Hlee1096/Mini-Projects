
#ifndef PTHREAD_LIST_H
#define PTHREAD_LIST_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define INIT_CAPACITY 2

typedef struct thread_struct thread_struct;


/*
Linked list to hole threads.
*/
struct thread_struct{
    pthread_t thread;
    unsigned int disconneted;
    thread_struct* next;
};

typedef struct{
    thread_struct* head;
    thread_struct* tail;
    int size;
} pthread_dynamic_list;



pthread_dynamic_list* pthread_list_init();


thread_struct* pthread_list_add_new(pthread_dynamic_list* pthread_list);



void free_pthread_list(pthread_dynamic_list* pthread_list);

#endif
