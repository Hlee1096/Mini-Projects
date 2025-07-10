

#include "../libs/client_execution.h"



void client_log_appender(char** client_log, const char* broadcast_string){
    //printf("appending: %s\n", broadcast_string);
    if (*client_log == NULL){
        *client_log = strdup(broadcast_string);
    }
    else{
        size_t string_length = strlen(broadcast_string);
        size_t current_client_log_len = strlen(*client_log);
        char* temp_str = malloc(sizeof(char) * (string_length + current_client_log_len + 1));
        strcpy(temp_str, *client_log);
        strcat(temp_str, broadcast_string);
        free(*client_log);
        *client_log = temp_str;
    }
   
}

void document_receive(const int client_2_server_fd, const int server_2_client_fd, char* username, int flags, int* perms, document* doc){
    write(client_2_server_fd, username, strlen(username));
    free(username);
    char buffer[BUFFER_SIZE];

    int bytes_read;
    bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read > 0){
        buffer[bytes_read] = '\0';
    }
   
    if (strcmp(buffer, "Reject UNAUTHORISED") == 0){
        *perms = -1;
        printf("not allowed\n");
        return;
    }
    if(strcmp(buffer, "read\n") == 0){
        *perms = 0;
        bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0){
            buffer[bytes_read] = '\0';
        }
        doc->version = atoi(buffer);

        bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0){
            buffer[bytes_read] = '\0';
        }
        doc->present_size = atoi(buffer);
        unsigned int count = 0;
        while (count < doc->present_size){
            bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0){
                buffer[bytes_read] = '\0';
            }
            markdown_insert(doc, doc->version, count, buffer);
            count++;
        }


    }
     if(strcmp(buffer, "write\n") == 0){
        *perms = 1;
        bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0){
            buffer[bytes_read] = '\0';
        }
        doc->version = atoi(buffer);

        bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0){
            buffer[bytes_read] = '\0';
        }
        doc->present_size = atoi(buffer);
        unsigned int count = 0;
        while (count < doc->present_size){
            bytes_read = read(server_2_client_fd, buffer, BUFFER_SIZE - 1);
            if (bytes_read > 0){
                buffer[bytes_read] = '\0';
            }
            markdown_insert(doc, doc->version, count, buffer);
            count++;
        }

    }
    
    fcntl(server_2_client_fd, F_SETFL, flags | O_NONBLOCK);
    return;
}



int is_doc_command(const char* token){
    const char* valid_commands[] = {"INSERT", "DEL", "NEWLINE", "HEADING",
                                    "BOLD", "ITALIC", "BLOCKQUOTE", "UNORDERED_LIST",
                                    "ORDERED_LIST" ,"CODE", "HORIZONTAL_RULE", "LINK"} ;
    
    int valid_commands_len = sizeof(valid_commands) / sizeof(valid_commands[0]);
    
    for (int i = 0; i < valid_commands_len; i++){
        if (strcmp(token, valid_commands[i]) == 0){
            return 1;
        }
    }
    return 0;
}

void send_command(int client_2_server_fd, char* input_buffer){
    printf("sending %s \n", input_buffer);
    write(client_2_server_fd, input_buffer, strlen(input_buffer));
   
}

void stdin_execution(int* client_loop, int client_2_server_fd, document* doc, char* input_buffer, int* client_role, char* client_log){
    char* input_buffer_dupe = malloc(strlen(input_buffer) + 1);
    strcpy(input_buffer_dupe, input_buffer);

    char* token = strtok(input_buffer_dupe, " ");
    if (strcmp(token, "DISCONNECT\n") == 0){
        printf("Disconnect\n");
        *client_loop = -2;
    }
    else if (strcmp(token, "DOC?\n") == 0){
        markdown_print(doc, stdout);
    }
    else if (strcmp(token, "LOG?\n") == 0){
        printf("%s", client_log);
    }
    else if (strcmp(token, "PERM?\n") == 0){
        if (*client_role == 0){
            printf("read\n");
        }
        else if (*client_role == 1){
            printf("write\n");
        }
    }
    else if (is_doc_command(token) == 1){
        send_command(client_2_server_fd, input_buffer);
    }
    else {
        puts("Unknown Command!");
    }
        
    free(input_buffer_dupe);

}

void command_execute(document* doc, char* command_line){
    char *saveptr;
    char* command;
    char* success_reject = NULL;
    
    printf("command_line is %s", command_line);
    success_reject = strstr(command_line, "SUCCESS\n");
    if (success_reject == NULL){
        success_reject = "Reject";
        return;
    }
    strtok_r(command_line, " ", &saveptr);  // EDIT
    strtok_r(NULL, " ", &saveptr);          // user
    command = strtok_r(NULL, "", &saveptr); 
   

    if (success_reject != NULL){
        char* command_dupe = strdup(command);
        char* command_saveptr;
        char* command_type = strtok_r(command_dupe, " ", &command_saveptr);
        char* token;
        

        if (strcmp(command_type, "INSERT") == 0){
            char* insert_string = malloc(sizeof(char) * 256);
            token = strtok_r(NULL, " ", &command_saveptr);     // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            char* start_of_string_count = command_saveptr;    // To help count
            token = strtok_r(NULL, "", &command_saveptr);
            unsigned int first_flag = 0;
            
            while (strcmp(token, "SUCCESS\n") != 0){
                token = strtok_r(NULL, " ", &start_of_string_count);
                if (strcmp(token, "SUCCESS\n") == 0 || strcmp(token, "SUCCESS\nEND\n") == 0  ){
                    break;
                }
                if (first_flag == 0){
                    first_flag = 1;
                    strcpy(insert_string, token);
                }
                else{
                    strcat(insert_string, token);
                }
                strcat(insert_string, " ");
                
            }
            insert_string[strlen(insert_string) - 1] = '\0';
        
            
            markdown_insert(doc, doc->version, pos, insert_string);
            free(insert_string);
        
        }
        else if (strcmp(command_type, "DEL") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Pos
            size_t pos;
            size_t no_chars;
            sscanf(token, "%zu", &pos);
            token = strtok_r(NULL, " ", &command_saveptr);       // no_char to delete
            sscanf(token, "%zu", &no_chars);
            
           markdown_delete(doc, doc->version, pos, no_chars);
            
            
        }
        else if (strcmp(command_type, "NEWLINE") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);      // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            markdown_newline(doc, doc->version, pos);

            
        }
        else if (strcmp(command_type, "HEADING") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Level
            size_t level;
            size_t pos;
            sscanf(token, "%zu", &level);
            token = strtok_r(NULL, " ", &command_saveptr);       // pos
            sscanf(token, "%zu", &pos);
            markdown_heading(doc, doc->version, level, pos);
        

        }
        else if (strcmp(command_type, "BOLD") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // pos start
            size_t pos_start;
            size_t pos_end;
            sscanf(token, "%zu", &pos_start);
            token = strtok_r(NULL, " ", &command_saveptr);       // pos end
            sscanf(token, "%zu", &pos_end);
            markdown_bold(doc, doc->version, pos_start, pos_end);
            

        }
        else if (strcmp(command_type, "ITALIC") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // pos start
            size_t pos_start;
            size_t pos_end;
            sscanf(token, "%zu", &pos_start);
            token = strtok_r(NULL, " ", &command_saveptr);       // pos end
            sscanf(token, "%zu", &pos_end);
        
            markdown_italic(doc, doc->version, pos_start, pos_end);
        
        }
        else if (strcmp(command_type, "BLOCKQUOTE") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            markdown_blockquote(doc, doc->version, pos);
            
            
        }
        else if (strcmp(command_type, "ORDERED_LIST") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            
            markdown_ordered_list(doc, doc->version, pos);
            
            
        }
        else if (strcmp(command_type, "UNORDERED_LIST") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            
            markdown_unordered_list(doc, doc->version, pos);
            
            
        }
        else if (strcmp(command_type, "CODE") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // pos start
            size_t pos_start;
            size_t pos_end;
            sscanf(token, "%zu", &pos_start);
            token = strtok_r(NULL, " ", &command_saveptr);      // pos end
            sscanf(token, "%zu", &pos_end);
            
            markdown_code(doc, doc->version, pos_start, pos_end);
            
            
        }
        else if (strcmp(command_type, "HORIZONTAL_RULE") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // Pos
            size_t pos;
            sscanf(token, "%zu", &pos);
            
           markdown_horizontal_rule(doc, doc->version, pos);
            
            
        }
        else if (strcmp(command_type, "LINK") == 0){
            token = strtok_r(NULL, " ", &command_saveptr);       // pos start
            size_t pos_start;
            size_t pos_end;
            sscanf(token, "%zu", &pos_start);
            token = strtok_r(NULL, " ", &command_saveptr);      // pos end
            sscanf(token, "%zu", &pos_end);
            token = strtok_r(NULL, " ", &command_saveptr);      // link
            markdown_link(doc, doc->version, pos_start, pos_end, token);
            
            
        }





        free(command_dupe);
    }
    return;
    
}

void from_server_execution(int* client_loop, document* doc, char* input_buffer, int byte_size, int* client_role, char** client_log){
    static unsigned int payload_flags = 0;      // Max is 3 due to payload being the role, version and doc size;
    static unsigned int document_count = 0;     //  where to insert in doc
    static unsigned int payload_size = 0;       // how big doc is
    static unsigned int version_flag = 0;       // when version is being broadcasted
    static unsigned int edited_flag = 0;
    static unsigned int empty_doc = 0;
    if (strcmp(input_buffer, "Reject UNAUTHORISED") == 0){
        *client_loop = -1;
        return;
    }
    if (payload_flags < 3){
        char* token = strtok(input_buffer, "\n");
        printf("toke is %s and payflag is %d\n", token, payload_flags);
        if (payload_flags == 0){
            if (strcmp(token, "read")){
                *client_role = 0;
            }
            else if (strcmp(token, "write")){
                *client_role = 1;
            }
        }
        while (token != NULL && payload_flags < 3){
            if (token == NULL){
                break;
            }
            if (payload_flags == 1){
                printf(" in paydload flags 1 %s", token);
                doc->version = atol(token);
                payload_size += strlen(token) + 1;
                printf("doc version is %ld\n", doc->version);
            }
            if (payload_flags == 2){
                doc->present_size = atol(token);
                payload_size += strlen(token) + 1;
                printf("doc size is %d\n", doc->present_size);
                if (doc->present_size == 0){
                    empty_doc = 1;
                }
            }
            payload_flags++;
            token = strtok(NULL, "\n");
        }
        return;
        
        
    }
    if (document_count < doc->present_size && empty_doc == 0){
        char* tmp_str = malloc(sizeof(char)* 2);
        for (int i = payload_size; i < byte_size; i++){
            if (document_count >= doc->present_size){
                puts("BREAKING OUT HULK");
                break;
            }
    
            tmp_str[1] = '\0';
            tmp_str[0] = input_buffer[i];

            markdown_insert(doc, doc->version, document_count, tmp_str);
            markdown_present_insert(doc, doc->version, document_count, tmp_str);
            document_count++;
        }
        free(tmp_str);
        return;
        
    }
    char* input_buffer_dupe = strdup(input_buffer);
    char* token = strtok(input_buffer_dupe, " ");
    // token = strtok(NULL, " ");
    printf("token is %s\n", token);
    // printf("input buffer is %s\n", input_buffer);
    if (strcmp(token, "VERSION") == 0 || version_flag == 1){
        if (strcmp(token, "VERSION") == 0){
            version_flag = 1;
            token = strtok(input_buffer_dupe, " ");
            doc->version = atol(token);
        }
        else if (strcmp(token, "END\n") == 0){
            version_flag = 0;
            if (edited_flag == 1){
                edited_flag = 0;
                markdown_increment_version(doc);
            }
        }
        else{
            command_execute(doc, input_buffer);
            edited_flag = 1;
            
        }
        client_log_appender(client_log, input_buffer);

    }
    else {
        puts("to do");
    }
    free(input_buffer_dupe);
    
}