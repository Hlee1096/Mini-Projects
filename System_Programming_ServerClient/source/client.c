#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>  
#include <string.h>
#include <sys/select.h>
#include "../libs/document.h"
#include "../libs/markdown.h"
#include "../libs/client_execution.h"


#define BASE_10 10
#define DEFAULT_SIZE (9)
#define BUFFER_SIZE (512)

char* make_username(const char* username_arg){
    int len = strlen(username_arg);
    char* username_ptr = malloc(sizeof(char) * (len +  2)); // for \n and null
    strcpy(username_ptr, username_arg);
    strcat(username_ptr, "\n");
    return username_ptr;
}

char* named_pipe_creator(const char* file_name, const int client_pid){
    int len = snprintf( NULL, 0, "%d", client_pid ) + 1;  // Plus 1 due to null terminator
    char* pid_str = malloc(sizeof(char) * len );
    snprintf(pid_str, len, "%d", client_pid);

    char* name = malloc(sizeof(char) * (len + DEFAULT_SIZE + 1));
    strcpy(name, file_name);
    strcat(name, pid_str);
    //name[len + DEFAULT_SIZE] = 0;
    free(pid_str);
    return name;
}

void send_signal(int server_pid){
    union sigval client_sig;
    client_sig.sival_int = getpid();
    sigqueue(server_pid, SIGRTMIN, client_sig);
    return;
}


int main(int argc, char** argv){
    if (argc != 3){
        printf("Incorrect Arguments\n");
        return -1;
    }

    int server_pid = strtol(argv[1], NULL, BASE_10);
    
    char* client_log = NULL;
  
    char* username = make_username(argv[2]);                // Username used to send to server first time.
    int client_role = -1;
    char* server_2_client = named_pipe_creator("FIFO_S2C_", getpid());
    char* client_2_server = named_pipe_creator("FIFO_C2S_", getpid());

    document* client_doc = markdown_init();
    fd_set readfds;
    struct timeval tv;
    int ret;
    char input_buffer[BUFFER_SIZE];

    // Initialsie the select timeout, both 0 for polling
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    int server_2_client_fd;
    int client_2_server_fd;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, (SIGRTMIN + 1));
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    int result_sig;

    send_signal(server_pid);

    
    // Initialises the select

    
    sigwait(&sigset, &result_sig);
    
    
    
    //server_2_client_fd = open(server_2_client, O_RDONLY | O_NONBLOCK);
    client_2_server_fd = open(client_2_server, O_WRONLY);
    server_2_client_fd = open(server_2_client, O_RDONLY);
    

    int flags = fcntl(server_2_client_fd, F_GETFL, 0);

    
    // document_receive(client_2_server_fd, server_2_client_fd, username, flags, &perms, client_doc);
    write(client_2_server_fd, username, strlen(username));
    free(username);

    fcntl(server_2_client_fd, F_SETFL, flags | O_NONBLOCK);
    // markdown_print(client_doc, stdout);
    
    int client_loop = 1;
    while (client_loop == 1){
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(server_2_client_fd, &readfds);

        ret = select(server_2_client_fd + 1, &readfds, NULL, NULL, &tv);
        if (ret > 0){
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                int bytes_read = read(STDIN_FILENO, input_buffer, BUFFER_SIZE - 1);
                if (bytes_read > 0) {
                    input_buffer[bytes_read] = '\0';
                    printf("Read STDIN: %s", input_buffer);
                    stdin_execution(&client_loop, client_2_server_fd, client_doc,
                                    input_buffer, &client_role, client_log);

                }

            }
            if (FD_ISSET(server_2_client_fd, &readfds)){
                int bytes_read = read(server_2_client_fd, input_buffer, BUFFER_SIZE - 1);
                if (bytes_read > 0) {
                    input_buffer[bytes_read] = '\0';
                    from_server_execution(&client_loop, client_doc, input_buffer,
                                            bytes_read, &client_role, &client_log);
                    
                    
                }
                if (bytes_read == 0){
                    break;
                }
            }
            
        }
        
    }
    printf("present_size = %d and size = %d\n", client_doc->present_size, client_doc->size);
    puts("Shutting Down Client!");
    
    close(server_2_client_fd);
    close(client_2_server_fd);
    free(server_2_client);
    free(client_2_server);
    markdown_free(client_doc);
    free(client_log);
}