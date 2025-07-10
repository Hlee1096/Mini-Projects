#ifndef SERVER_EXEC_H

#define SERVER_EXEC_H

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
#include <time.h>
#include "../libs/document.h"
#include "../libs/markdown.h"
#include "../libs/pthread_list.h"
#include "../libs/version_queue.h"
#include "../libs/version_queue_array.h"
#include "../libs/dynamic_queue_arr.h"
#include "../libs/broadcast_dyn_arr.h"
#include "../libs/dynamic_command_arr.h"


int client_handler_execute(const int write_fd, int* username_flag, 
                            document* doc, char* input_buffer);                                                     // Handles command from client.


void server_execute(char* input_buffer, int bytes_read, document* doc, int* server_loop,
     pthread_mutex_t* document_lock, pthread_mutex_t* active_clients_lock, unsigned int* active_clients,
     version_queue_array* server_version_q_arr);






void editing_mode(document* doc, pthread_mutex_t* document_lock_ptr,
                    dyn_array* server_2_client_fd, pthread_mutex_t* broadcast_lock,
                    version_queue_array* server_version_q_arr, dynamic_cmd_array* server_command_array,
                    dyn_cmd_queue_arr* server_command_queue_array);

void teardown(document* doc);
long timediff(clock_t t1, clock_t t2);

int command_execute(document* doc, pthread_mutex_t* document_lock_ptr, command* cmd);
#endif
