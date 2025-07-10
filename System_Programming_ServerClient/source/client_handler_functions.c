#include "../libs/client_handler_functions.h"


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

void make_named_pipes(int client_pid, int* server_client_fd){
    printf("called in named piped\n");
    char* server_2_client = named_pipe_creator("FIFO_S2C_", client_pid);
    char* client_2_server = named_pipe_creator("FIFO_C2S_", client_pid);

    int server_2_client_fd;
    int client_2_server_fd;

    if (access(server_2_client, F_OK) == 0){
        unlink(server_2_client);
    }
    if (access(client_2_server, F_OK) == 0){
        unlink(client_2_server);
    }

    int perm = 0666;                        // everyone has read write perms.    
    mkfifo(server_2_client, perm);
    mkfifo(client_2_server, perm);
    
    kill(client_pid, (SIGRTMIN + 1));
    // client_2_server_fd = open(client_2_server, O_RDONLY | O_NONBLOCK);
    client_2_server_fd = open(client_2_server, O_RDONLY);
    server_2_client_fd = open(server_2_client, O_WRONLY);
    
    
    
    

    server_client_fd[0] = client_2_server_fd;
    server_client_fd[1] = server_2_client_fd;
    //printf("read: %d and write: %d\n", server_client_fd[0], server_client_fd[1]);

    free(server_2_client);
    free(client_2_server);
    printf("exiting in named piped\n");
}

char* check_roles(const char* username){
    FILE* text_file = fopen("roles.txt", "r");
   

    char* stripped_username = malloc(sizeof(char) * strlen(username) + 1);
    strcpy(stripped_username, username);
    stripped_username[strlen(username) - 1] = '\0';
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    char* token;
    char* saveptr;
    char* search = " ";
    while ((read = getline(&line, &len, text_file)) != -1) {
        token = strtok_r(line, search, &saveptr);
        if (strcmp(token, stripped_username) == 0){
            token = strtok_r(NULL, search, &saveptr);
            if (strcmp(token, "read\n") == 0){
                free(stripped_username);
                fclose(text_file);
                free(line);
                return "read\n";
            }
            else if (strcmp(token, "write\n") == 0){
                free(stripped_username);
                fclose(text_file);
                free(line);
                return "write\n";
            }
        }
        
    }
    free(stripped_username);
    free(line);
    fclose(text_file);
    return "none";
}

void check_username(const int read_fd, const int write_fd, int* client_role, char** username){
    int bytes_read = 0;
    
    char input_buffer[PIPE_BUFFER_SIZE];
    bytes_read = read(read_fd, input_buffer, PIPE_BUFFER_SIZE - 1);
    input_buffer[bytes_read] = '\0';


    char* malloced_username = malloc(sizeof(char) * bytes_read + 1);
    strcpy(malloced_username, input_buffer);
    malloced_username[bytes_read - 1] = '\0';
    *username = malloced_username;


    char* file_perm = check_roles(input_buffer);
    //printf("file ferm in check username %s", file_perm);
    if (strcmp(file_perm, "none") == 0){
        write(write_fd, "Reject UNAUTHORISED", strlen("Reject UNAUTHORISED"));
        *client_role = -1;
        return;
    }
    else if (strcmp(file_perm, "write\n") == 0){
        write(write_fd, file_perm, strlen(file_perm));
        *client_role = 1;
        return;
    }
    else if (strcmp(file_perm, "read\n") == 0){
        write(write_fd, file_perm, strlen(file_perm));
        *client_role = 0;
        return;
    }
}



void version_size_payload_sender(document* doc, const int write_fd){
    int len = snprintf( NULL, 0, "%ld", doc->version) + 1 + 1;   // Plus one for null and plus one for '\n'
    char* version = malloc(sizeof(char) * len);
    snprintf(version, len, "%lu", doc->version);
    version[len-1] = '\0';
    version[len - 2] = '\n';
    write(write_fd, version, strlen(version));
    free(version);

    len = snprintf( NULL, 0, "%d", doc->present_size) + 1 + 1;
    char* size = malloc(sizeof(char) * len);
    snprintf(size, len, "%d", doc->present_size);
    size[len - 1] = '\0';
    size[len - 2] = '\n';
    write(write_fd, size, strlen(size));
    free(size);

}

void send_document(document* doc, const int write_fd){
    chunk* cursor = doc->present;
    while (cursor != NULL){
        write(write_fd, &(cursor->character), sizeof(char));
        cursor = cursor->next;
    }

}

void document_transmission(document* doc, const int write_fd, pthread_mutex_t* document_lock_ptr){
    pthread_mutex_lock(document_lock_ptr);

    version_size_payload_sender(doc, write_fd);

    send_document(doc, write_fd);

    pthread_mutex_unlock(document_lock_ptr);
}

int check_ascii (char* array, int len) {        // returns 0 if there is non ascii and 1 if none non ascii
    for (int i = 0; i < len; i++) {
        unsigned char c = array[i];
        if (c > 127 || c < 32){
            return 0;
        }
        
    }
    return 1;
}

int valid_commands(char* command){
    char *saveptr;
    char* token = strtok_r(command, " ", &saveptr);
    if (strcmp(token, "INSERT") == 0){
        return 1;
    }
    else if (strcmp(token, "DEL") == 0){
        return 1;
    }
    else if (strcmp(token, "NEWLINE") == 0){
        return 1;
    }
    else if (strcmp(token, "HEADING") == 0){
        return 1;
    }
    else if (strcmp(token, "BOLD") == 0){
        return 1;
    }
    else if (strcmp(token, "ITALIC") == 0){
        return 1;
    }
    else if (strcmp(token, "BLOCKQUOTE") == 0){
        return 1;
    }
    else if (strcmp(token, "ORDERED_LIST") == 0){
        return 1;
    }
    else if (strcmp(token, "UNORDERED_LIST") == 0){
        return 1;
    }
    else if (strcmp(token, "CODE") == 0){
        return 1;
    }
    else if (strcmp(token, "HORIZONTAL_RULE") == 0){
        return 1;
    }
    else if (strcmp(token, "LINK") == 0){
        return 1;
    }
    else {
        return 0;
    }
    
}
int valid_command_format(char* input, int len){ // returns 1 if valid and 0 if not
    if (check_ascii(input, len - 1) == 0){      // -1 due to manually checking if final character is new line.
        return 0;
    }
    if (len > 256){
        return 0;
    }
    if (input[len - 1] != '\n'){
        return 0;
    }
    if (valid_commands(input) == 0){
        return 0;
    }
    return 1;
}