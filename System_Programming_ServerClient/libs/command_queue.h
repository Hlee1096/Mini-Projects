
#ifndef COMMAND_QUEUE_H

#define COMMAND_QUEUE_H
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct command command;



struct command{
    char* username;
    char* command_str;              // DYNAMIC
    struct timeval tv; 
    int role;         
    command* next;
};

typedef struct{
    command* head;
    command* tail;
    pthread_mutex_t queue_lock;
    char* username;             // DYNAMIC???
    int size;
    unsigned int disconnected;
    int role;
} command_queue;

command* make_new_command(char* command_str, char* username, int role);

command_queue* command_queue_init();
void command_queue_enqueue(command_queue* cmd_queue, command* cmd);
command* command_queue_dequeue(command_queue* cmd_queue);
void command_queue_destroy(command_queue* cmd_queue); // Detstorys commands

#endif