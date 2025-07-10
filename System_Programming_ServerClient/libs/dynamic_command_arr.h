#ifndef DYN_COMMAND_ARR_H

#define DYN_COMMAND_ARR_H
#include "../libs/command_queue.h"

#define INIT_CAPACITY 2


typedef struct{
    int capacity;
    int size;
    command** command_array; // hold pointer that points to a command struct.
} dynamic_cmd_array;


dynamic_cmd_array* dynamic_command_arr_init();

void dynamic_command_arr_add(dynamic_cmd_array* dyn_cmd_arr, command* cmd);
void dynamic_command_arr_delete(dynamic_cmd_array* dyn_cmd_arr, int index);
command* dynamic_command_arr_get(dynamic_cmd_array* dyn_cmd_arr, int index);
void dynamic_command_arr_destroy(dynamic_cmd_array* dyn_cmd_arr);

void dynamic_command_arr_sort(dynamic_cmd_array* dyn_cmd_arr);      // Utilised the stdlib qsort to sort the array in time stamp
int dynamic_command_arr_compare(const void* a, const void* b);      // Used in the stdlib qsort compare function.

#endif