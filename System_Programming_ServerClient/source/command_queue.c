#include "../libs/command_queue.h"


command* make_new_command(char* command_str, char* username, int role){
    command* cmd = malloc(sizeof(command));
    char* new_command_str = strdup(command_str);
    cmd->command_str = new_command_str;
    cmd->username = username;
    cmd->role = role;
    cmd->next = NULL;
    gettimeofday(&cmd->tv, NULL);
    return cmd;

}

command_queue* command_queue_init(){
   command_queue* c_queue = malloc(sizeof(command_queue));
   c_queue->head = NULL;
   c_queue->tail = NULL;
   c_queue->username = NULL;
   c_queue->size = 0;
   c_queue->disconnected = 0;
   c_queue->role = -1;
   pthread_mutex_init(&c_queue->queue_lock, NULL);
   return c_queue;
}

void command_queue_enqueue(command_queue* cmd_queue, command* cmd){
    //pthread_mutex_lock(&cmd_queue->queue_lock);
    cmd_queue->size++;
    if (cmd_queue->head == NULL){
        cmd_queue->head = cmd;
        cmd_queue->tail = cmd;
        //pthread_mutex_unlock(&cmd_queue->queue_lock);
        return;
    }
    cmd_queue->tail->next = cmd;
    cmd_queue->tail = cmd;
    //pthread_mutex_unlock(&cmd_queue->queue_lock);
}


command* command_queue_dequeue(command_queue* cmd_queue){
    //pthread_mutex_lock(&cmd_queue->queue_lock);
    
    if (cmd_queue->head == NULL){
        printf("Command Queue is Empty\n");
        // pthread_mutex_unlock(&cmd_queue->queue_lock);
        return NULL;
    }
    command* temp = cmd_queue->head;
    cmd_queue->head = cmd_queue->head->next;
    if (cmd_queue->head == NULL){
        cmd_queue->tail = NULL;
       
    }
    cmd_queue->size--;
    //pthread_mutex_unlock(&cmd_queue->queue_lock);
    return temp;
}

void command_queue_destroy(command_queue* cmd_queue){
    pthread_mutex_destroy(&cmd_queue->queue_lock);
    free(cmd_queue);
}

// int main(){
//     command_queue* c_q = command_queue_init();
//     command* c1 = make_new_command("herobrene", "KILL THEM ALL");
//     command* c2 = make_new_command("ng", "KILL ALL");
//     command* c3 = make_new_command("ab", "2");
//     command_queue_enqueue(c_q, c1);
//     command_queue_enqueue(c_q, c2);
//     command_queue_enqueue(c_q, c3);

  
//     while (c_q->head != NULL){
//         printf("%s\n", command_queue_dequeue(c_q)->command_str);
//     }
 
// }