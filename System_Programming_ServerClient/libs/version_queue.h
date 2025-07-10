#ifndef VERSION_QUEUE_H

#define VERSION_QUEUE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct version_node version_node;


struct version_node{
    char* command;          // Dynamic
    char* username;         // Dynamic
    char* success_reject;
    char* reason;           // Only if above is reject
    version_node* next;
};

typedef struct{
    version_node* head;
    version_node* tail;
}version_queue;

version_queue* version_queue_init();
void version_queue_enqueue(version_queue* version_q, version_node* version_n);
version_node* version_queue_dequeue(version_queue* version_q);
void version_queue_destroy(version_queue* version_queue);
version_node* make_new_version_node();



#endif
