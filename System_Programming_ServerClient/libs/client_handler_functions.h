#ifndef CLIENT_HANDLER_FUNC_H

#define CLIENT_HANDLER_FUNC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <pthread.h>
#include <sys/select.h>
#include "../libs/document.h"
#include "../libs/markdown.h"
#include "../libs/pthread_list.h"
#include "../libs/command_queue.h"
#include "../libs/dynamic_command_arr.h"
#include "../libs/dynamic_queue_arr.h"
#include "../libs/broadcast_dyn_arr.h"

#define DEFAULT_SIZE (9)  // Size of FIFO_C2S_ and FIFO_S2C_
#define PIPE_BUFFER_SIZE (512)

typedef struct{
    int client_pid;
    document* doc;
//   pthread_dynamic_list* thread_list;
    command_queue* cmd_queue;
    thread_struct* thread_struct_ptr;
    dyn_array* server_2_client_fd;
    pthread_mutex_t* document_lock_ptr;
    pthread_mutex_t* active_clients_lock_ptr;
    pthread_mutex_t* broadcast_lock_ptr;
    

    // unsigned int* broadcast_flag_ptr;
    unsigned int* active_clients_ptr;
    //pthread_t* thread;
    //dyn_cmd_queue_arr* dynamic_cmd_queue_arr;
} client_handler_args;


void check_username(const int read_fd, const int write_fd, int* client_role, char** username);
char* check_roles(const char* username);
void* client_handler(void* arg);
char* named_pipe_creator(const char* file_name, int client_pid);
void make_named_pipes(int client_pid, int* server_client_fd);
void version_size_payload_sender(document* doc, const int write_fd);
void send_document(document* doc, const int write_fd);
void document_transmission(document* doc, const int write_fd, pthread_mutex_t* document_lock_ptr);
int check_ascii (char* array, int len);
int valid_commands(char* command);
int valid_command_format(char* input, int len);

#endif