

#include "../libs/command_queue.h"
#include "../libs/dynamic_queue_arr.h"




dyn_cmd_queue_arr* dyn_cmd_queue_arr_init(){
    dyn_cmd_queue_arr* dynamic_cmd_queue = malloc(sizeof(dyn_cmd_queue_arr));
    dynamic_cmd_queue->capacity = INIT_CAPACITY;
    dynamic_cmd_queue->size = 0;
    dynamic_cmd_queue->cmd_queue_arr = malloc(sizeof(void*) * INIT_CAPACITY);   
    return dynamic_cmd_queue;

}

void dyn_cmd_queue_arr_add(dyn_cmd_queue_arr* dynamic_cmd_queue, command_queue* cmd_queue){
    if (dynamic_cmd_queue->size == dynamic_cmd_queue->capacity){
        dynamic_cmd_queue->capacity *= 2;
        dynamic_cmd_queue->cmd_queue_arr = realloc(dynamic_cmd_queue->cmd_queue_arr,
                                             sizeof(void*) * dynamic_cmd_queue->capacity);
    }
    dynamic_cmd_queue->cmd_queue_arr[dynamic_cmd_queue->size] = cmd_queue;
    dynamic_cmd_queue->size++;

}

void dyn_cmd_queue_arr_delete(dyn_cmd_queue_arr* dynamic_cmd_queue, int index){
     if (index < 0 || index >= dynamic_cmd_queue->size){
        printf("Index out of bounds, Nothing Deleted\n");
        return;
    }
    free(dynamic_cmd_queue->cmd_queue_arr[index]);

    for (int i = index; i < dynamic_cmd_queue->size - 1; i++){
        dynamic_cmd_queue->cmd_queue_arr[i] = dynamic_cmd_queue->cmd_queue_arr[i+1];
    }
    dynamic_cmd_queue->size--;
}

command_queue* dyn_cmd_queue_arr_get(dyn_cmd_queue_arr* dynamic_cmd_queue, int index){
    if (index < 0 || index >= dynamic_cmd_queue->size){
        printf("Index out of bounds\n");
        return NULL;
    }   
    return dynamic_cmd_queue->cmd_queue_arr[index];
}

void dyn_cmd_queue_arr_destroy(dyn_cmd_queue_arr* dynamic_cmd_queue){
    for (int i = 0; i < dynamic_cmd_queue->size; i++){
        command_queue* cmd_q = dyn_cmd_queue_arr_get(dynamic_cmd_queue, i);
        free(cmd_q->username);
        command_queue_destroy(cmd_q);
    }
    free(dynamic_cmd_queue->cmd_queue_arr);
    free(dynamic_cmd_queue);
}

// int main(){
//     printf("%d", INIT_CAPACITY);
// }