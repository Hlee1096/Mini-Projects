
#include "../libs/server_execution.h"
#include "../libs/client_handler_functions.h"
#include "../libs/dynamic_queue_arr.h"




void broadcast(dyn_array* server_2_client_fd, unsigned int flag,
                version_queue_array* server_version_q_arr, pthread_mutex_t* broadcast_lock){
    int current_version = server_version_q_arr->current_version;
    if (flag == 1){ // broadcast version and all edits
        int length = snprintf( NULL, 0, "%d", current_version);
        char* version_string = malloc(sizeof(char) * (strlen("VERSION ") + length + 1 + 1));
        char* version_num_string =  malloc(sizeof(char) * (length + 1 + 1));
        strcpy(version_string, "VERSION ");
        snprintf(version_num_string, length + 1 + 1, "%d\n", current_version);
        strcat(version_string, version_num_string);
        int write_size = strlen(version_string);

        pthread_mutex_lock(broadcast_lock);
        for (int i = 0; i < server_2_client_fd->size; i++){
            int write_fd = dyn_array_get(server_2_client_fd, i);
            write(write_fd , version_string, write_size);
            version_node* version_node_cursor = server_version_q_arr->version_queue_arr[current_version]->head;
            while (version_node_cursor != NULL){
                char* username = version_node_cursor->username;
                char* unstripped_command = version_node_cursor->command;
                char* command = strndup(unstripped_command, strlen(unstripped_command) - 1);
                char* success_reject = version_node_cursor->success_reject;
                char* reason = version_node_cursor->reason;
                if (strcmp(success_reject, "SUCCESS") == 0){
                    size_t total_len = strlen(username) + strlen(command) + strlen(success_reject);
                    int num_space = 3;
                     // 4 for EDIT, 1 for new line, 1 for null
                    char* writing_string = malloc(sizeof(char) * total_len + 4 + 1 + 1 + num_space);   
                    strcpy(writing_string, "EDIT");
                    strcat(writing_string, " ");
                    strcat(writing_string, username);
                    strcat(writing_string, " ");
                    strcat(writing_string, command);
                    strcat(writing_string, " ");
                    strcat(writing_string, success_reject);
                    strcat(writing_string, "\n");
                    write(write_fd , writing_string, strlen(writing_string));
                    free(writing_string);
                    free(command);
                }
                else{

                    int num_space = 4;
                    size_t total_len = strlen(username) + strlen(command) + strlen(success_reject) + strlen(reason);
                    char* writing_string = malloc(sizeof(char) * total_len + 4 + 1 + 1 + num_space);    
                    strcpy(writing_string, "EDIT");
                    strcat(writing_string, " ");
                    strcat(writing_string, username);
                    strcat(writing_string, " ");
                    strcat(writing_string, command);
                    strcat(writing_string, " ");
                    strcat(writing_string, success_reject);
                    strcat(writing_string, " ");
                    strcat(writing_string, reason);
                    strcat(writing_string, "\n");
                    write(write_fd , writing_string, strlen(writing_string));
                    free(writing_string);
                    free(command);
                }


                version_node_cursor = version_node_cursor->next;
            }
            // length of end with new line in 4 bytes, dont wanna use strlen to slow program down.
            write(write_fd, "END\n", 4);              
            
        }
        pthread_mutex_unlock(broadcast_lock);
        free(version_string);
        free(version_num_string);

    }
    else{           // Broadcast just the version if edit flag is false
        int length = snprintf( NULL, 0, "%d", current_version);
        char* version_string = malloc(sizeof(char) * (strlen("VERSION ") + length + 1 + 1));
        char* version_num_string =  malloc(sizeof(char) * (length + 1 + 1));
        strcpy(version_string, "VERSION ");
        snprintf(version_num_string, length + 1 + 1, "%d\n", current_version);
        strcat(version_string, version_num_string);
        int write_size = strlen(version_string);
        
        
        pthread_mutex_lock(broadcast_lock);
        for (int i = 0; i < server_2_client_fd->size; i++){
            int write_fd = dyn_array_get(server_2_client_fd, i);
            write(write_fd , version_string, write_size);
            write(write_fd, "END\n", 4);               
        }
        pthread_mutex_unlock(broadcast_lock);
        free(version_string);
        free(version_num_string);
    }
}

void execute_commands_from_arr(document* doc, pthread_mutex_t* document_lock_ptr,
                                dynamic_cmd_array* server_command_array, version_queue_array* server_version_q_arr, 
                                dyn_array* server_2_client_fd, pthread_mutex_t* broadcast_lock){
    
    unsigned int edit_flag = 0;
    version_queue* current_vq = version_queue_array_get(server_version_q_arr, server_version_q_arr->current_version);
    for (int i = 0; i < server_command_array->size; i++){
        version_node* version_node_ptr = make_new_version_node();
        command* cmd = dynamic_command_arr_get(server_command_array, i);
        version_node_ptr->command = cmd->command_str;
        version_node_ptr->username = cmd->username;
        int return_value = 0;
        return_value = command_execute(doc, document_lock_ptr, cmd);
        if (return_value == SUCCESS){
            edit_flag = 1;
            version_node_ptr->success_reject = "SUCCESS";
        }
        else if (return_value == -4){       // Unauthorised
            version_node_ptr->success_reject = "Reject";
            version_node_ptr->reason = "UNAUTHORISED";
        }
        else if (return_value == INVALID_CURSOR_POS){
            version_node_ptr->success_reject = "Reject";
            version_node_ptr->reason = "INVALID_CURSOR_POS";
        }
        else if (return_value == DELETED_POSITION){
            version_node_ptr->success_reject = "Reject";
            version_node_ptr->reason = "DELETED_POSITION";
        }
        free(cmd);
        version_queue_enqueue(current_vq, version_node_ptr);
        

    }
    server_command_array->size = 0;
    if (edit_flag == 1){
        broadcast(server_2_client_fd, edit_flag, server_version_q_arr, broadcast_lock);
        server_version_q_arr->current_version++;
        version_queue* new_vq = version_queue_init();
        version_queue_array_add(server_version_q_arr, new_vq);
        pthread_mutex_lock(document_lock_ptr);
        markdown_increment_version(doc);
        pthread_mutex_unlock(document_lock_ptr);

    }
    else{
        broadcast(server_2_client_fd, edit_flag, server_version_q_arr, broadcast_lock);
    }
    

}


int command_execute(document* doc, pthread_mutex_t* document_lock_ptr, command* cmd){   // Returns 1 if successful
    
    int role = cmd->role;
    if (role != 1){
        return -4;
    }

    int return_value = 0;
    char* command_dupe = strdup(cmd->command_str);
    char* stripped_command = strndup(cmd->command_str, strlen(cmd->command_str)-1);
    char* token = strtok(stripped_command, " ");            // Command such as insert, del...
    
    if (strcmp(token, "INSERT") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        token = strtok(NULL, "");
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_insert(doc, doc->version, pos, token);
        pthread_mutex_unlock(document_lock_ptr);
    }
    else if (strcmp(token, "DEL") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        size_t no_chars;
        sscanf(token, "%zu", &pos);
        token = strtok(NULL, " ");      // no_char to delete
        sscanf(token, "%zu", &no_chars);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_delete(doc, doc->version, pos, no_chars);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "NEWLINE") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_newline(doc, doc->version, pos);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "HEADING") == 0){
        token = strtok(NULL, " ");      // Level
        size_t level;
        size_t pos;
        sscanf(token, "%zu", &level);
        token = strtok(NULL, " ");      // pos
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_heading(doc, doc->version, level, pos);
        pthread_mutex_unlock(document_lock_ptr);

    }
    else if (strcmp(token, "BOLD") == 0){
        token = strtok(NULL, " ");      // pos start
        size_t pos_start;
        size_t pos_end;
        sscanf(token, "%zu", &pos_start);
        token = strtok(NULL, " ");      // pos end
        sscanf(token, "%zu", &pos_end);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_bold(doc, doc->version, pos_start, pos_end);
        pthread_mutex_unlock(document_lock_ptr);

    }
    else if (strcmp(token, "ITALIC") == 0){
        token = strtok(NULL, " ");      // pos start
        size_t pos_start;
        size_t pos_end;
        sscanf(token, "%zu", &pos_start);
        token = strtok(NULL, " ");      // pos end
        sscanf(token, "%zu", &pos_end);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_italic(doc, doc->version, pos_start, pos_end);
        pthread_mutex_unlock(document_lock_ptr);
    }
    else if (strcmp(token, "BLOCKQUOTE") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_blockquote(doc, doc->version, pos);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "ORDERED_LIST") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_ordered_list(doc, doc->version, pos);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "UNORDERED_LIST") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_unordered_list(doc, doc->version, pos);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "CODE") == 0){
        token = strtok(NULL, " ");      // pos start
        size_t pos_start;
        size_t pos_end;
        sscanf(token, "%zu", &pos_start);
        token = strtok(NULL, " ");      // pos end
        sscanf(token, "%zu", &pos_end);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_code(doc, doc->version, pos_start, pos_end);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "HORIZONTAL_RULE") == 0){
        token = strtok(NULL, " ");      // Pos
        size_t pos;
        sscanf(token, "%zu", &pos);
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_horizontal_rule(doc, doc->version, pos);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    else if (strcmp(token, "LINK") == 0){
        token = strtok(NULL, " ");      // pos start
        size_t pos_start;
        size_t pos_end;
        sscanf(token, "%zu", &pos_start);
        token = strtok(NULL, " ");      // pos end
        sscanf(token, "%zu", &pos_end);
        token = strtok(NULL, " ");      // link
        pthread_mutex_lock(document_lock_ptr);
        return_value = markdown_link(doc, doc->version, pos_start, pos_end, token);
        pthread_mutex_unlock(document_lock_ptr);
        
    }
    free(command_dupe);
    return return_value;
}



void editing_mode(document* doc, pthread_mutex_t* document_lock_ptr,
                    dyn_array* server_2_client_fd, pthread_mutex_t* broadcast_lock,
                    version_queue_array* server_version_q_arr, dynamic_cmd_array* server_command_array,
                    dyn_cmd_queue_arr* server_command_queue_array){
    
    // Loop below dequeues all command queues into array of commands
    for (int i = 0; i < server_command_queue_array->size; i++){             
        command_queue* cmd_queue = dyn_cmd_queue_arr_get(server_command_queue_array, i);
        pthread_mutex_lock(&cmd_queue->queue_lock);
        //printf("cmd queue size is %d\n", cmd_queue->size);
        int cmd_q_len = cmd_queue->size;
        for (int j = 0; j < cmd_q_len; j++){
            command* tmp_cmd = command_queue_dequeue(cmd_queue);
            printf("tmp_cmd string %s\n", tmp_cmd->command_str);
            dynamic_command_arr_add(server_command_array, tmp_cmd);
        }
        pthread_mutex_unlock(&cmd_queue->queue_lock);
    }
    dynamic_command_arr_sort(server_command_array);
    execute_commands_from_arr(doc, document_lock_ptr, server_command_array, server_version_q_arr, 
                                server_2_client_fd, broadcast_lock);
        
}


void teardown(document* doc){
    FILE *fptr;
    fptr = fopen("doc.md", "w");
    markdown_print(doc, fptr);
    fclose(fptr);
}


long timediff(clock_t t1, clock_t t2) { // Returns the time passed from t1
    long elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}


void log_function(version_queue_array* server_version_q_arr){
    for (int i = 0; i < server_version_q_arr->size; i++){
        printf("VERSION %d\n", i);

        version_queue* v_queue = version_queue_array_get(server_version_q_arr, i);
        version_node* v_node_cursor = v_queue->head;
        while (v_node_cursor != NULL){
            char* username = v_node_cursor->username;
            char* success_reject = v_node_cursor->success_reject;
            char* reason = v_node_cursor->reason;
            char* stripped_command = strndup(v_node_cursor->command, strlen(v_node_cursor->command)-1);
            if (strcmp(success_reject, "SUCCESS") == 0){
                printf("EDIT %s %s %s\n", username, stripped_command, success_reject);
            }
            else{
                printf("EDIT %s %s %s %s\n", username, stripped_command, success_reject, reason);
            }
            free(stripped_command);
            v_node_cursor = v_node_cursor->next;
        }
        printf("END\n");
    }
}

void quit_function(int* server_loop, pthread_mutex_t* active_clients_lock, unsigned int* active_clients){
    pthread_mutex_lock(active_clients_lock);
    if (*active_clients == 0){
        *server_loop = 0;
    }
    else{
        printf("QUIT rejected, %u clients still connected.\n", *active_clients);
    }
    pthread_mutex_unlock(active_clients_lock);
}

void server_execute(char* input_buffer, int bytes_read, document* doc, int* server_loop,
     pthread_mutex_t* document_lock, pthread_mutex_t* active_clients_lock, unsigned int* active_clients,
     version_queue_array* server_version_q_arr){  // DISCONNECT , DOC?, LOG? , QUIT
    
    
    if (strcmp(input_buffer, "LOG?\n") == 0){
        log_function(server_version_q_arr);
        return;
    }
    else if (strcmp(input_buffer, "DOC?\n") == 0){
        pthread_mutex_lock(document_lock);
        markdown_print(doc, stdout);
        pthread_mutex_unlock(document_lock);
        return;
    }
    else if (strcmp(input_buffer, "QUIT\n") == 0){
        quit_function(server_loop, active_clients_lock, active_clients);
        return;
    }
    
    if (valid_command_format(input_buffer, bytes_read) == 0){
        puts("NOT RIGHT!!");
    }

   
}

