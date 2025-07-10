#ifndef CLIENT_EXECUTION_H

#define CLIENT_EXECUTION_H
#include "../libs/markdown.h"
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE (512)

void stdin_execution(int* client_loop, int client_2_server_fd, document* doc, char* input_buffer, int* client_role, char* client_log);
void document_receive(const int client_2_server_fd, const int server_2_client_fd, char* username, int flags, int* perms, document* doc);
void from_server_execution(int* client_loop, document* doc, char* input_buffer, int byte_size, int* client_role, char** client_log);
int is_doc_command(const char* token);
#endif