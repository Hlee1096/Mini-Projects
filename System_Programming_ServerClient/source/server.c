#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h> 
#include <pthread.h>
#include <time.h>
#include "../libs/markdown.h"
#include "../libs/pthread_list.h"
#include "../libs/client_handler_functions.h"
#include "../libs/version_queue.h"
#include "../libs/server_execution.h"
#include "../libs/command_queue.h"
#include "../libs/dynamic_command_arr.h"
#include "../libs/dynamic_queue_arr.h"
#include "../libs/broadcast_dyn_arr.h"


#define BASE_10 10
#define BUFFER_SIZE 10

static volatile int client_signal_pid = -1;                 // Variable to keep track of new client's pid
static volatile int new_client_flag = 0;                    // Flag to keep track if new client joins.         
#define UNUSED(x) (void)(x)


// sigaction handler
void sigaction_handler_funct(int sig, siginfo_t *info, void *ucontext){
    UNUSED(sig);
    UNUSED(ucontext);
    new_client_flag = 1;
    client_signal_pid = info->si_pid;
}


int main(int argc, char** argv){
    if (argc != 2){
        printf("Incorrect Arguments\n");
        return -1;
    }
    document* server_document = markdown_init();
    pthread_dynamic_list* pthread_list = pthread_list_init();                       // Array that holds threads_structs
    dyn_cmd_queue_arr* server_command_queue_array = dyn_cmd_queue_arr_init();       // Array holds all the queues that each thread will have
    dynamic_cmd_array* server_command_array = dynamic_command_arr_init();           // Array holds all the commands
    version_queue_array* server_version_q_arr = version_queue_array_init();         // Array holds the logs.
    version_queue_array_add(server_version_q_arr, version_queue_init());
    dyn_array* server_2_client_fd = dyn_array_init();                               // Array holds all the client's fd

    pthread_mutex_t document_lock = PTHREAD_MUTEX_INITIALIZER;                      // Accessing documents will require this lock.
    pthread_mutex_t active_clients_lock = PTHREAD_MUTEX_INITIALIZER;                 // Accessing documents will require this lock.
    pthread_mutex_t broadcast_lock = PTHREAD_MUTEX_INITIALIZER;                      // Accessing documents will require this lock.
    unsigned int active_clients = 0;
   

    // Initialises the select
    fd_set readfds;
    struct timeval tv;
    int ret;
    char buffer[BUFFER_SIZE];

    // Initialsie the select timeout, both 0 for polling
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    struct sigaction signal_handler = {0};
    signal_handler.sa_flags = SA_SIGINFO;
    signal_handler.sa_sigaction = sigaction_handler_funct;

    sigaction(SIGRTMIN, &signal_handler, NULL);


    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGRTMIN);
    sigaddset(&signal_set, (SIGRTMIN+1));
    
    int time_interval = strtol(argv[1], NULL, BASE_10);
    clock_t t1, t2;

    printf("Server PID: %d\n", getpid());
   

    t1 = clock();
    int server_loop = 1;
    while (server_loop) {
        t2 = clock();
        if (timediff(t1, t2) >= time_interval){
            editing_mode(server_document, &document_lock, server_2_client_fd,
                        &broadcast_lock, server_version_q_arr, server_command_array,
                        server_command_queue_array);
            t1 = clock();
        }


        if (new_client_flag == 1){

            sigprocmask(SIG_BLOCK, &signal_set, NULL);
            new_client_flag = 0;
            client_handler_args* pthread_args = malloc(sizeof(client_handler_args));
            pthread_args->client_pid = client_signal_pid;
            pthread_args->doc = server_document;
            
            pthread_args->cmd_queue = command_queue_init();
            thread_struct* thread_struct_ptr = pthread_list_add_new(pthread_list);
            thread_struct_ptr->disconneted = 0;
            pthread_args->thread_struct_ptr = thread_struct_ptr;
            pthread_args->document_lock_ptr = &document_lock;
            pthread_args->broadcast_lock_ptr = &broadcast_lock;
            pthread_args->active_clients_lock_ptr = &active_clients_lock; 
            // pthread_args->broadcast_flag_ptr = &broadcast_flag;
            pthread_args->active_clients_ptr = &active_clients;
            pthread_args->server_2_client_fd = server_2_client_fd;
            
            printf("THREAD LIST SIZE %d\n", pthread_list->size);

            dyn_cmd_queue_arr_add(server_command_queue_array, pthread_args->cmd_queue);
            pthread_create(&thread_struct_ptr->thread, NULL, &client_handler, (void*) pthread_args);
            
            pthread_mutex_lock(&active_clients_lock);
            active_clients++;
            pthread_mutex_unlock(&active_clients_lock);

            client_signal_pid = -1;

            sigprocmask(SIG_UNBLOCK, &signal_set, NULL);
        }
        

        // ReInitialises the bit set.
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        if (ret > 0){
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0';
                    server_execute(buffer, bytes_read ,server_document, &server_loop, &document_lock,
                                    &active_clients_lock, &active_clients, server_version_q_arr);
                    
                }

            }
        }
    }


   
    editing_mode(server_document, &document_lock, server_2_client_fd,
                        &broadcast_lock, server_version_q_arr, server_command_array,
                        server_command_queue_array);                                   
    free_pthread_list(pthread_list);                        
    version_queue_array_destroy(server_version_q_arr);
    teardown(server_document);
    markdown_free(server_document);
    dynamic_command_arr_destroy(server_command_array);      
    dyn_array_free(server_2_client_fd);

    dyn_cmd_queue_arr_destroy(server_command_queue_array);


}
