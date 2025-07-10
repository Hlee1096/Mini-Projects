#include <stdlib.h>
#include <stdio.h>

#ifndef BROADCAST_DYN_ARR_H

#define BROADCAST_DYN_ARR_H

#define INIT_CAPACITY 2

typedef struct {
    int capacity; // maximum size
    int size; // current size
    int *array;
} dyn_array;

dyn_array *dyn_array_init();
void dyn_array_add(dyn_array *dyn, int value);
void dyn_array_delete(dyn_array *dyn, int index);
int dyn_array_get(dyn_array *dyn, int index);
void dyn_array_free(dyn_array *dyn);


#endif