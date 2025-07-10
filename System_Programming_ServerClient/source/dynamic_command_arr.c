


#include "../libs/dynamic_command_arr.h"

// #include "command_queue.c"


dynamic_cmd_array* dynamic_command_arr_init(){
    dynamic_cmd_array* dyn_cmd_arr = malloc(sizeof(dynamic_cmd_array));
    dyn_cmd_arr->capacity = INIT_CAPACITY;
    dyn_cmd_arr->command_array = malloc(sizeof(void*) * INIT_CAPACITY);
    dyn_cmd_arr->size = 0;
    return dyn_cmd_arr;
}

void dynamic_command_arr_add(dynamic_cmd_array* dyn_cmd_arr, command* cmd){
    if (dyn_cmd_arr->size == dyn_cmd_arr->capacity){
        dyn_cmd_arr->capacity *= 2;
        dyn_cmd_arr->command_array = realloc(dyn_cmd_arr->command_array, sizeof(void*) * dyn_cmd_arr->capacity);
    }
    dyn_cmd_arr->command_array[dyn_cmd_arr->size] = cmd;
    dyn_cmd_arr->size++;

}

void dynamic_command_arr_delete(dynamic_cmd_array* dyn_cmd_arr, int index){
     if (index < 0 || index >= dyn_cmd_arr->size){
        printf("Index out of bounds, Nothing Deleted\n");
        return;
    }
    free(dyn_cmd_arr->command_array[index]->command_str);
    free(dyn_cmd_arr->command_array[index]);
    for (int i = index; i < dyn_cmd_arr->size - 1; i++){
        dyn_cmd_arr->command_array[i] = dyn_cmd_arr->command_array[i+1];
    }
    dyn_cmd_arr->size--;
}

command* dynamic_command_arr_get(dynamic_cmd_array* dyn_cmd_arr, int index){
    if (index < 0 || index >= dyn_cmd_arr->size){
        printf("Index out of bounds\n");
        return NULL;
    }   
    return dyn_cmd_arr->command_array[index];
}


int dynamic_command_arr_compare(const void* a, const void* b){
    /*
    -1 if a goes before b
    0 if a == b
    1 if a goes after b
    
    */
    command* cmd_a = *(command**) a;
    command* cmd_b = *(command**) b;

    struct timeval tv_a = cmd_a->tv;
    struct timeval tv_b = cmd_b->tv;

    if (tv_a.tv_sec < tv_b.tv_sec){
        return -1;
    }
    if (tv_a.tv_sec > tv_b.tv_sec){
        return 1;
    }

    // If equal compare micro seconds

    if (tv_a.tv_usec < tv_b.tv_usec){
        return -1;
    }
    if (tv_a.tv_usec > tv_b.tv_usec){
        return 1;
    }

    return 0;

    
}
void dynamic_command_arr_sort(dynamic_cmd_array* dyn_cmd_arr){
    qsort(dyn_cmd_arr->command_array, 
        dyn_cmd_arr->size, 
        sizeof(command*), 
        &dynamic_command_arr_compare);
}

void dynamic_command_arr_destroy(dynamic_cmd_array* dyn_cmd_arr){
    free(dyn_cmd_arr->command_array);
    free(dyn_cmd_arr);
}

// int main(){
//     command_queue* c_q = command_queue_init();
//     dynamic_cmd_array* cmd_arr = dynamic_command_arr_init();

//     command* cmd1 = make_new_command("hero");

    
//     command* cmd2 = make_new_command("abc");
//     command* cmd3 = make_new_command("jak");
//     command_queue_enqueue(c_q, cmd3);
//     command_queue_enqueue(c_q, cmd1);
//     command_queue_enqueue(c_q, cmd2);

//     dynamic_command_arr_add(cmd_arr, command_queue_dequeue(c_q));
//     dynamic_command_arr_add(cmd_arr, command_queue_dequeue(c_q));
//     dynamic_command_arr_add(cmd_arr, command_queue_dequeue(c_q));

//     for (int i = 0; i < cmd_arr->size; i++){
//         printf("%s\n", cmd_arr->command_array[i]->command_str);
//     }

//     dynamic_command_arr_sort(cmd_arr);
//     for (int i = 0; i < cmd_arr->size; i++){
//         printf("%s\n", cmd_arr->command_array[i]->command_str);
//     }
// }