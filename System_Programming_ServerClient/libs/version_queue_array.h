
#ifndef VERSION_QUEUE_ARRAY_H
#define VERSION_QUEUE_ARRAY_H

#include "../libs/version_queue.h"
#define INIT_CAPACITY 2


typedef struct{
    version_queue** version_queue_arr;
    int size;
    int capacity;
    int current_version;
}version_queue_array;

version_queue_array* version_queue_array_init();

void version_queue_array_add(version_queue_array* version_q_arr, version_queue* version_q);

version_queue* version_queue_array_get(version_queue_array* version_q_arr, int index);
void version_queue_array_destroy(version_queue_array* version_q_arr);

#endif