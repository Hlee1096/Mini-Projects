
#include "../libs/version_queue_array.h"





version_queue_array* version_queue_array_init(){
    version_queue_array* version_q_arr = malloc(sizeof(version_queue_array));
    version_q_arr->capacity = INIT_CAPACITY;
    version_q_arr->size = 0;
    version_q_arr->version_queue_arr = malloc(sizeof(void*) * INIT_CAPACITY);
    version_q_arr->current_version = 0;
    return version_q_arr;
}

void version_queue_array_add(version_queue_array* version_q_arr, version_queue* version_q){
    if (version_q_arr->size == version_q_arr->capacity){
        version_q_arr->capacity *= 2;
        version_q_arr->version_queue_arr = realloc(version_q_arr->version_queue_arr,
                                                    sizeof(void*) * version_q_arr->capacity);
    }
    version_q_arr->version_queue_arr[version_q_arr->size] = version_q;
    version_q_arr->size++;
}

version_queue* version_queue_array_get(version_queue_array* version_q_arr, int index){
    if ((index < 0 && -index > version_q_arr->size) || (index > 0 && index >= version_q_arr->size))
        return NULL;
    // we support Python-like negative indexing
    return version_q_arr->version_queue_arr[index];
}
void version_queue_array_destroy(version_queue_array* version_q_arr){
    for (int i = 0; i < version_q_arr->size; i++){
        version_queue_destroy(version_q_arr->version_queue_arr[i]);
    }
    free(version_q_arr->version_queue_arr);
    free(version_q_arr);
}