
#include "../libs/client_handler_functions.h"
#include "../libs/server_execution.h"


void* client_handler(void* arg){    // thread handle per client
    
    client_handler_args* thread_args = (client_handler_args*) arg;
    int client_pid = thread_args->client_pid;
    document* doc = thread_args->doc;
    thread_struct* thread_struct_ptr = thread_args->thread_struct_ptr;
    command_queue* cmd_queue = thread_args->cmd_queue;
    pthread_mutex_t* document_lock_ptr = thread_args->document_lock_ptr;
    pthread_mutex_t* broadcast_lock_ptr = thread_args->broadcast_lock_ptr;
    pthread_mutex_t* active_clients_lock_ptr = thread_args->active_clients_lock_ptr;
    dyn_array* server_2_client_fd_arr = thread_args->server_2_client_fd;
    
    unsigned int* active_clients_ptr = thread_args->active_clients_ptr;
    
    free(arg);

    
    
    int server_client_fd[2];        // 0 will be client to server, 1 will be server to client
    server_client_fd[0] = -1;
    server_client_fd[1] = -1;
    make_named_pipes(client_pid, server_client_fd);
    int flags = fcntl(server_client_fd[0], F_GETFL);

 
    // Initialises the select
    fd_set readfds;
    struct timeval tv;
    char input_buffer[PIPE_BUFFER_SIZE];

    unsigned added_fd_flag = 0;
    char* client_username = NULL;           
    int client_role = -1;                               // -1 Role unassigned, 0 - read only, 1 write.

    
    check_username(server_client_fd[0], server_client_fd[1], &client_role, &client_username);

    if (client_role != -1){
        document_transmission(doc, server_client_fd[1], document_lock_ptr);
        fcntl(server_client_fd[0], F_SETFL ,flags | O_NONBLOCK);
        cmd_queue->username = client_username;
        cmd_queue->role = client_role;
        pthread_mutex_lock(broadcast_lock_ptr);
        dyn_array_add(server_2_client_fd_arr, server_client_fd[1]);
        pthread_mutex_unlock(broadcast_lock_ptr);
        added_fd_flag = 1;
    }
    

    // Initialsie the select timeout, both 0 for polling
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    printf("cmd qu role %d\n", cmd_queue->role);
    while (1 && client_role != -1){
        FD_ZERO(&readfds);
        FD_SET(server_client_fd[0], &readfds);
        select(server_client_fd[0] + 1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(server_client_fd[0], &readfds)) {
            int bytes_read = read(server_client_fd[0], input_buffer, PIPE_BUFFER_SIZE - 1);
            // printf("bytes read: %d\n", bytes_read);
            if (bytes_read > 0) {
                input_buffer[bytes_read] = '\0';
                
                if (strcmp(input_buffer, "DISCONNECT\n") == 0){
                    break;
                }
                command* new_cmd = make_new_command(input_buffer, cmd_queue->username, cmd_queue->role);
               
                
                pthread_mutex_lock(&cmd_queue->queue_lock);
                command_queue_enqueue(cmd_queue, new_cmd);
                pthread_mutex_unlock(&cmd_queue->queue_lock);
                
            }
            if (bytes_read == 0){
                break;
            }
        }

        
      
    }
    printf("Closing\n");
    if (client_role == -1){
        puts("waiting 1 second");
        sleep(1);
    }
    

    pthread_mutex_lock(active_clients_lock_ptr);
    (*active_clients_ptr)--;
    pthread_mutex_unlock(active_clients_lock_ptr);

    if (added_fd_flag == 1){
        pthread_mutex_lock(broadcast_lock_ptr);
        for (int i = 0; i < server_2_client_fd_arr->size; i++){
            if (dyn_array_get(server_2_client_fd_arr, i) == server_client_fd[1]){
                dyn_array_delete(server_2_client_fd_arr, i);
                break;
            }
        }
        pthread_mutex_unlock(broadcast_lock_ptr);
    }
    
    cmd_queue->disconnected = 1;
    thread_struct_ptr->disconneted = 1;
    char* s2c = named_pipe_creator("FIFO_S2C_", client_pid);
    char* c2s = named_pipe_creator("FIFO_C2S_", client_pid);
    unlink(s2c);
    unlink(c2s);
    free(s2c);
    free(c2s);
    close(server_client_fd[0]);
    close(server_client_fd[1]);

    printf("done\n");
    return NULL;

}

