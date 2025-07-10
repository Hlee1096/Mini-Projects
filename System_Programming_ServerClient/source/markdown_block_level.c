#include "../libs/markdown.h"



int markdown_heading(document *doc, uint64_t version, int level, size_t pos){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    size_t delete_cursor_pos = 0;
    unsigned int deleted_flag = 0;

    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
        if (cursor != NULL){                                                // incase at the end of doc
            if (cursor->marked_for_deletion == 1 && deleted_flag == 0){
                delete_cursor_pos = count;
                deleted_flag = 1;
            }
            else if (cursor->marked_for_deletion == 0){
                deleted_flag = 0;
            }
        }
    }
    chunk* start_of_heading = make_new_chunk('#');
    chunk* prev_chunk = start_of_heading;
    chunk* temp_chunk;
    chunk* end_of_heading  = make_new_chunk(' ');
    
    doc->size++;
    doc->size++;
    for (int i = 1; i < level; i++){
        temp_chunk = make_new_chunk('#');
        prev_chunk->next = temp_chunk;
        prev_chunk = temp_chunk;
        doc->size++;
    }
    prev_chunk->next = end_of_heading;
    
    if (cursor->marked_for_deletion == 1){
        cursor = doc->head;
        behind_cursor = NULL;
        count = 0;
        while (count < delete_cursor_pos){
            behind_cursor = cursor;
            cursor = cursor->next;
            count++;
        }
    }

    if (cursor == NULL && behind_cursor == NULL){
        doc->head = start_of_heading;
        return SUCCESS;
    }
    
    if (behind_cursor == NULL){
        temp_chunk = doc->head;
        doc->head = start_of_heading;
        end_of_heading->next =  temp_chunk;
     

    }
    else {
        if (behind_cursor->character != '\n'){
            chunk* newline_chunk = make_new_chunk('\n');
            newline_chunk->next = start_of_heading;
            behind_cursor->next = newline_chunk;
            end_of_heading->next = cursor;
            doc->size++;
        }
        else{
            behind_cursor->next = start_of_heading;
            end_of_heading->next = cursor;
        }

    }
    return SUCCESS;
}

int markdown_blockquote(document *doc, uint64_t version, size_t pos){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }
    chunk* start_of_block = make_new_chunk('>');
    start_of_block->next = make_new_chunk(' ');
 
    doc->size++;
    doc->size++;

 
    if (cursor == NULL && behind_cursor == NULL){
        doc->head = start_of_block;
        return SUCCESS;
    }
    
    if (behind_cursor == NULL){
        chunk* temp_chunk = doc->head;
        doc->head = start_of_block;
        start_of_block->next->next =  temp_chunk;
     

    }
    else {
        if (behind_cursor->character != '\n'){
            chunk* newline_chunk = make_new_chunk('\n');
            newline_chunk->next = start_of_block;
            behind_cursor->next = newline_chunk;
            start_of_block->next->next = cursor;
            doc->size++;
        }
        else{
            behind_cursor->next = start_of_block;
            start_of_block->next->next = cursor;
        }

    }
    return SUCCESS;
}

int markdown_horizontal_rule(document *doc, uint64_t version, size_t pos){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }
    chunk* start_of_line = make_new_chunk('-');
    start_of_line->next = make_new_chunk('-');
    start_of_line->next->next = make_new_chunk('-');
    chunk* end_of_line = start_of_line->next->next;
    doc->size += 3;



    if (cursor == NULL && behind_cursor == NULL){
        doc->head = start_of_line;
        end_of_line->next = make_new_chunk('\n');
        doc->size++;
        return SUCCESS;
    }
    if (cursor == NULL){
        end_of_line->next = make_new_chunk('\n');
        end_of_line = end_of_line->next;
        doc->size++;
        
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = start_of_line;
            start_of_line = new_line_chunk;
        }
        behind_cursor->next = start_of_line;

        return SUCCESS;
    }

    if (behind_cursor == NULL){
        if (cursor->character != '\n'){
            end_of_line->next = make_new_chunk('\n');
            doc->size++;
            end_of_line = end_of_line->next;
            chunk* temp = doc->head;
            doc->head = start_of_line;
            end_of_line->next = temp;
            return SUCCESS;
        }
        else{
            chunk* temp = doc->head;
            doc->head = start_of_line;
            end_of_line->next = temp;
            return SUCCESS;
        }

    }

    if (cursor != NULL && behind_cursor != NULL){
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = start_of_line;
            start_of_line = new_line_chunk;
        }
        if (cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            end_of_line->next = new_line_chunk;
            end_of_line = new_line_chunk;
        }
        behind_cursor->next = start_of_line;
        end_of_line->next = cursor;
    }

    
    
    return SUCCESS;
    
}

int markdown_unordered_list(document *doc, uint64_t version, size_t pos){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }

    chunk* list_begin = make_new_chunk('-');
    list_begin->next = make_new_chunk(' ');
    chunk* list_end = list_begin->next;
    doc->size++; 
    doc->size++;

    if (cursor == NULL && behind_cursor == NULL){
        doc->head = list_begin;
        return SUCCESS;
    }
    if (cursor == NULL){
       
        
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = list_begin;
            list_begin = new_line_chunk;
        }
        behind_cursor->next = list_begin;

        return SUCCESS;
    }

    if (behind_cursor == NULL){
        
        chunk* temp = doc->head;
        doc->head = list_begin;
        list_end->next = temp;
        return SUCCESS;
    

    }

    if (cursor != NULL && behind_cursor != NULL){
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = list_begin;
            list_begin = new_line_chunk;
        }

        behind_cursor->next = list_begin;
        list_end->next = cursor;
    }

    
    

    return SUCCESS;
    


}

int valid_list(chunk* cursor, chunk* behind_cursor){
    chunk* csr = cursor;
    if (behind_cursor == NULL){
        char character = csr->character;
        int num = (int) character - '0';
        if (!(num > 0 && num < 10)){
            return 0;
        }
        
        if (csr->next != NULL){
            csr = csr->next;
            if (csr->character != '.'){
                return 0;
            }
        }

        if (csr->next != NULL){
            csr = csr->next;
            if (csr->character != ' '){
                return 0;
            }
        }
        return 1;
    }
    
    if (csr != NULL){
        if (csr->character != '\n'){
            return 0;
        }
        if (csr->next != NULL){  // checks if number exist
            csr = csr->next;
        }
        char character = csr->character;
        int num = (int) character - '0';
        if (!(num > 0 && num < 10)){
            return 0;
        }
        if (csr->next != NULL){ 
            csr = csr->next;
            if (csr->character != '.'){
                return 0;
            }
        }

        if (csr->next != NULL){
            csr = csr->next;
            if (csr->character != ' '){
                return 0;
            }
        }
        return 1;
    }
    
    return 0;
}

void markdown_fix_ordered_list(document* doc){
    char character;
    int current_int = 0;
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    
   
 
    while (cursor != NULL){
        if (current_int > 9){
            current_int = 0;
        }
        if (behind_cursor == NULL){
            if (valid_list(cursor, behind_cursor) != 0){
                current_int = (int) cursor->character - '0';
            }
            else{
                current_int = 0;
            }
        }
        if (cursor->character == '\n'){
            int temp_int;
             if (valid_list(cursor, behind_cursor) != 0){
                temp_int = cursor->next->character - '0';
                if (temp_int != (current_int + 1)){
                    character = (current_int + 1) + '0';
                    cursor->next->character = character;
                    current_int += 1;
                }
                else{
                    current_int = temp_int;
                }
             }
             else {
                current_int = 0;
             }
             
        }
        


        behind_cursor = cursor;
        cursor = cursor->next;
       

    }
}

int markdown_ordered_list(document *doc, uint64_t version, size_t pos){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }
    chunk* start_of_list = make_new_chunk('1');
    start_of_list->next = make_new_chunk('.');
    start_of_list->next->next = make_new_chunk(' ');
    doc->size += 3;
    chunk* end_of_list = start_of_list->next->next;

    if (cursor == NULL && behind_cursor == NULL){
        doc->head = start_of_list;
        return SUCCESS;
    }
    if (cursor == NULL){
       
        
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = start_of_list;
            start_of_list = new_line_chunk;
        }
        behind_cursor->next = start_of_list;
        markdown_fix_ordered_list(doc);
        return SUCCESS;
    }

    if (behind_cursor == NULL){
        
        chunk* temp = doc->head;
        doc->head = start_of_list;
        end_of_list->next = temp;
        markdown_fix_ordered_list(doc);
        return SUCCESS;
    

    }

    if (cursor != NULL && behind_cursor != NULL){
        if (behind_cursor->character != '\n'){
            chunk* new_line_chunk = make_new_chunk('\n');
            doc->size++;
            new_line_chunk->next = start_of_list;
            start_of_list = new_line_chunk;
        }

        behind_cursor->next = start_of_list;
        end_of_list->next = cursor;
    }

    
    
    markdown_fix_ordered_list(doc);
    return SUCCESS;
    
}
