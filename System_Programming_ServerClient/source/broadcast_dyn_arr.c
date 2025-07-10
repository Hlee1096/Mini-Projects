

#include "../libs/broadcast_dyn_arr.h"


dyn_array *dyn_array_init(){
    dyn_array *dyn = malloc(sizeof(dyn_array));
    dyn->capacity = INIT_CAPACITY; // set initial capacity
    dyn->size = 0;
    dyn->array = malloc(sizeof(int) * INIT_CAPACITY); // allocate memory for actual array
    return dyn;
}

void dyn_array_add(dyn_array *dyn, int value){
    // capacity reached, needs to reallocate memory
    if (dyn->size == dyn->capacity) {
        dyn->capacity *= 2;
        // we double the current capacity every time
        // we need to expand the array
        dyn->array = realloc(dyn->array, sizeof(int) * dyn->capacity);
    }
    dyn->array[dyn->size] = value;
    dyn->size++;
}

void dyn_array_delete(dyn_array *dyn, int index)
{
    //shifts dyn[index + 1:] to the left
    for (int i = index; i < dyn->size - 1; i++)
        dyn->array[i] = dyn->array[i + 1];
    dyn->size--;
}

int dyn_array_get(dyn_array *dyn, int index){
    // return -1 for out-of-bound accesses
    if ((index < 0 && -index > dyn->size) || (index > 0 && index >= dyn->size)){
        puts("INDEX OUT OF BOUNDS");
        return -1;
    }
        
   
    return dyn->array[index];
}

void dyn_array_free(dyn_array *dyn){
    free(dyn->array); // free the internal array first
    free(dyn); // free the array struct itself
}
