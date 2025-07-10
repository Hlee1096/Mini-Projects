


#ifndef DYN_QUEUE_ARR_H
#define DYN_QUEUE_ARR_H

#include "../libs/command_queue.h"

#define INIT_CAPACITY 2
// This will hold an array of pointers which point to a queue for each thread to handle.

typedef struct{
    command_queue** cmd_queue_arr;
    int capacity;
    int size;

} dyn_cmd_queue_arr;

dyn_cmd_queue_arr* dyn_cmd_queue_arr_init();

void dyn_cmd_queue_arr_add(dyn_cmd_queue_arr* dynamic_cmd_queue, command_queue* cmd_queue);
void dyn_cmd_queue_arr_delete(dyn_cmd_queue_arr* dynamic_cmd_queue, int index);
void dyn_cmd_queue_arr_destroy(dyn_cmd_queue_arr* dynamic_cmd_queue);
command_queue* dyn_cmd_queue_arr_get(dyn_cmd_queue_arr* dynamic_cmd_queue, int index);

#endif