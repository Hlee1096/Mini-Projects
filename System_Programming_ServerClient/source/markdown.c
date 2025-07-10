

#include "../libs/markdown.h"
#include "markdown_block_level.c"

document* markdown_init(void){
    document* doc = malloc(sizeof(document));
    doc->head = NULL;
    doc->present = NULL;
    doc->present_size = 0;
    doc->version = 0;
    doc->size = 0;
    return doc;
}

void markdown_free(document *doc){
    chunk* cursor = doc->head;
    while (cursor != NULL){
        chunk* temp = cursor;
        cursor = cursor->next;
        free(temp);
    }

    cursor = doc->present;
    while (cursor != NULL){
        chunk* temp = cursor;
        cursor = cursor->next;
        free(temp);
    }
    free(doc);
}

chunk* make_new_chunk(char character){
    chunk* new_chunk;
    new_chunk = malloc(sizeof(chunk));
    new_chunk->character = character;
    new_chunk->next = NULL;
    new_chunk->marked_for_deletion = 0;
    new_chunk->new_insert = 1;
    return new_chunk;
}

// === Edit Commands ===
int markdown_insert(document *doc, uint64_t version, size_t pos, const char *content){
    chunk* cursor = doc->head;
    size_t count = 0;
    chunk* behind_cursor = NULL;
    size_t delete_cursor_pos = 0;
    unsigned int deleted_flag = 0;

    if (version != doc->version){
        return OUTDATED_VERSION;
    }

    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }

    
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        if (cursor != NULL){
            if (cursor->new_insert == 1){
                count--;
            }
        }
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
    
    if (cursor == NULL){ // handles start and end of document inserts
        chunk* new_chunk = make_new_chunk(content[0]);
        if (behind_cursor == NULL){
            doc->head = new_chunk;
        }
        else{
            behind_cursor->next = new_chunk;
        }
        doc->size++;
        chunk* temp = new_chunk;
        for (size_t i = 1; i < strlen(content); i++){
            new_chunk = make_new_chunk(content[i]);
            temp->next = new_chunk;
            temp = new_chunk;
            doc->size++;
        }
        return SUCCESS;
    }
    else {
        chunk* new_chunk = make_new_chunk(content[0]);
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

        if (behind_cursor == NULL){
            doc->head = new_chunk;
        }
        else{
            behind_cursor->next = new_chunk;
        }
        
        chunk* temp = new_chunk;
        doc->size++;
        for (size_t i = 1; i < strlen(content); i++){
            new_chunk = make_new_chunk(content[i]);
            temp->next = new_chunk;
            temp = new_chunk;
            doc->size++;
        }
        temp->next = cursor;
        return SUCCESS;
    }
    
}
int markdown_delete(document *doc, uint64_t version, size_t pos, size_t len){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    //chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < pos){
        //behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }
    if (cursor == NULL){
        return SUCCESS;
    }
    else{
        //chunk* temp = doc->head;
        
        count = 0;
        while (cursor != NULL && count < len){
            // temp = cursor->next;
            // free(cursor);
            // cursor = temp;
            // count++;
            
            if (cursor->new_insert == 0 && cursor->marked_for_deletion == 0){
                cursor->marked_for_deletion = 1;
                doc->size--;
                count++;
            }
            else if (cursor->marked_for_deletion){
                count++;
            }
            cursor = cursor->next;
            
        }
        // if (behind_cursor == NULL){
        //     doc->head = temp;
        // }
        // else{
        //     behind_cursor->next = temp;
        // }
    }
    return SUCCESS;
}

// === Formatting Commands ===
int markdown_newline(document *doc, uint64_t version, size_t pos){
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
        if (cursor != NULL){
            if (cursor->marked_for_deletion == 1 && deleted_flag == 0){
            delete_cursor_pos = count;
            deleted_flag = 1;
            }
            else if (cursor->marked_for_deletion == 0){
                deleted_flag = 0;
            }
        }
    }
    chunk* new_chunk = make_new_chunk('\n');
    
    if (cursor != NULL){
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

    }
    
    if (cursor == NULL && behind_cursor == NULL){
        doc->head = new_chunk;
        doc->size++;
        markdown_fix_ordered_list(doc);
        return SUCCESS;
    }

    if (behind_cursor == NULL){
        doc->head = new_chunk;
        new_chunk->next = cursor;
    }
    else{
        behind_cursor->next = new_chunk;
        new_chunk->next = cursor;
    }
    doc->size++;
    markdown_fix_ordered_list(doc);
    return SUCCESS;
}


int markdown_bold(document *doc, uint64_t version, size_t start, size_t end){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (end <= start || start > doc->present_size || end > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    size_t delete_cursor_pos_start = 0;
    unsigned int deleted_flag_start = 0;

    size_t delete_cursor_pos_end = 0;
    unsigned int deleted_flag_end = 0;

    while (cursor != NULL && count < start){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
        if (cursor != NULL){                                                // incase at the end of doc
            if (cursor->marked_for_deletion == 1 && deleted_flag_start == 0){
                delete_cursor_pos_start = count;
                deleted_flag_start = 1;
            }
            else if (cursor->marked_for_deletion == 0){
                deleted_flag_start = 0;
            }
        }
    }

    chunk* end_cursor = doc->head;
    chunk* end_behind = NULL;
    count = 0;
    while (end_cursor != NULL && count < end){
        end_behind = end_cursor;
        end_cursor = end_cursor->next;
        count++;

        if (end_cursor != NULL){
            if (end_cursor->marked_for_deletion == 1 && deleted_flag_end == 0){
                delete_cursor_pos_end = count;
                deleted_flag_end = 1;
            }
            else if (end_cursor->marked_for_deletion == 0){
                deleted_flag_end = 0;
            }
        }
    }

    chunk* beginning_bold = make_new_chunk('*');
    beginning_bold->next = make_new_chunk('*');
    doc->size++;
    doc->size++;

    chunk* end_bold = make_new_chunk('*');
    end_bold->next = make_new_chunk('*');
    doc->size++;
    doc->size++;

    if (behind_cursor == NULL && cursor != NULL){               // HANDLES BEGINNING
        chunk* temp = doc->head;
        doc->head = beginning_bold;
        beginning_bold->next->next = temp;
        end_behind->next = end_bold;
        end_bold->next->next = end_cursor;

    }

    if (end_cursor == NULL){                                    // HANDLES WHEN INSERTING AT END
        end_behind->next = end_bold;
        
    }
    
    if (end_cursor != NULL && cursor != NULL){
        if (end_cursor->marked_for_deletion == 1 && cursor->marked_for_deletion == 1){
            doc->size -= 4;
            free(beginning_bold->next);
            free(beginning_bold);
            free(end_bold->next);
            free(end_bold);
            return DELETED_POSITION;
        }

    }
    
    if (end_cursor != NULL){
        if (end_cursor->marked_for_deletion == 1){
            end_cursor = doc->head;
            end_behind = NULL;
            count = 0;
            while (count < delete_cursor_pos_end){
                end_behind = end_cursor;
                end_cursor = end_cursor->next;
                count++;
            }
            //while ()
        }

    }
   
    if (cursor != NULL){
        if (cursor->marked_for_deletion == 1){
            cursor = doc->head;
            behind_cursor = NULL;
            count = 0;
            while (count < delete_cursor_pos_start){
                behind_cursor = cursor;
                cursor = cursor->next;
                count++;
            }
        }

    }
   

    if (end_behind != NULL){                                   
        end_behind->next = end_bold;
        end_bold->next->next = end_cursor;
    }
    
    if (behind_cursor != NULL){
        behind_cursor->next = beginning_bold;
        beginning_bold->next->next = cursor;
    }

    
    return SUCCESS;
    
}

int markdown_italic(document *doc, uint64_t version, size_t start, size_t end){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (end <= start || start > doc->present_size || end > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;

    size_t delete_cursor_pos_start = 0;
    unsigned int deleted_flag_start = 0;

    size_t delete_cursor_pos_end = 0;
    unsigned int deleted_flag_end = 0;
    while (cursor != NULL && count < start){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
        if (cursor != NULL){                                                // incase at the end of doc
            if (cursor->marked_for_deletion == 1 && deleted_flag_start == 0){
                delete_cursor_pos_start = count;
                deleted_flag_start = 1;
            }
            else if (cursor->marked_for_deletion == 0){
                deleted_flag_start = 0;
            }
        }
    }

    chunk* end_cursor = doc->head;
    chunk* end_behind = NULL;
    count = 0;
    while (end_cursor != NULL && count < end){
        end_behind = end_cursor;
        end_cursor = end_cursor->next;
        count++;
        if (end_cursor != NULL){
            if (end_cursor->marked_for_deletion == 1 && deleted_flag_end == 0){
                delete_cursor_pos_end = count;
                deleted_flag_end = 1;
            }
            else if (end_cursor->marked_for_deletion == 0){
                deleted_flag_end = 0;
            }
        }
    }

    chunk* beginning_italic = make_new_chunk('*');
    doc->size++;

    chunk* end_italic = make_new_chunk('*');
    doc->size++;


    if (behind_cursor == NULL && cursor != NULL){               // HANDLES BEGINNING
        chunk* temp = doc->head;
        doc->head = beginning_italic;
        beginning_italic->next = temp;
        end_behind->next = end_italic;
        end_italic->next = end_cursor;

    }

    if (end_cursor == NULL){                                    // HANDLES WHEN INSERTING AT END
        end_behind->next = end_italic;
        
    }
    if (end_cursor != NULL && cursor != NULL){
        if (end_cursor->marked_for_deletion == 1 && cursor->marked_for_deletion == 1){
            doc->size -= 2;
            free(beginning_italic);
            free(end_italic);
            return DELETED_POSITION;
        }

    }

    if (end_cursor != NULL){
        if (end_cursor->marked_for_deletion == 1){
            end_cursor = doc->head;
            end_behind = NULL;
            count = 0;
            while (count < delete_cursor_pos_end){
                end_behind = end_cursor;
                end_cursor = end_cursor->next;
                count++;
            }
        }

    }
   
    if (cursor != NULL){
        if (cursor->marked_for_deletion == 1){
            cursor = doc->head;
            behind_cursor = NULL;
            count = 0;
            while (count < delete_cursor_pos_start){
                behind_cursor = cursor;
                cursor = cursor->next;
                count++;
            }
        }

    }
    
    if (end_behind != NULL){                                   
        end_behind->next = end_italic;
        end_italic->next = end_cursor;
    }
    
    if (behind_cursor != NULL){
        behind_cursor->next = beginning_italic;
        beginning_italic->next = cursor;
    }

    return SUCCESS;

}

int markdown_code(document *doc, uint64_t version, size_t start, size_t end){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (end <= start || start > doc->present_size || end > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < start){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }

    chunk* end_cursor = doc->head;
    chunk* end_behind = NULL;
    count = 0;
    while (end_cursor != NULL && count < end){
        end_behind = end_cursor;
        end_cursor = end_cursor->next;
        count++;
    }

    chunk* beginning_code = make_new_chunk('`');
    doc->size++;

    chunk* end_code = make_new_chunk('`');
    doc->size++;


    if (behind_cursor == NULL && cursor != NULL){               // HANDLES BEGINNING
        chunk* temp = doc->head;
        doc->head = beginning_code;
        beginning_code->next = temp;
        end_behind->next = end_code;
        end_code->next = end_cursor;

    }

    if (end_cursor == NULL){                                    // HANDLES WHEN INSERTING AT END
        end_behind->next = end_code;
        
    }
    
    if (end_behind != NULL){                                   
        end_behind->next = end_code;
        end_code->next = end_cursor;
    }
    
    if (behind_cursor != NULL){
        behind_cursor->next = beginning_code;
        beginning_code->next = cursor;
    }

    return SUCCESS;

    
}


int markdown_link(document *doc, uint64_t version, size_t start, size_t end, const char *url){
    if (version != doc->version){
        return OUTDATED_VERSION;
    }
    if (end <= start || start > doc->present_size || end > doc->present_size){
        return INVALID_CURSOR_POS;
    }
    chunk* cursor = doc->head;
    chunk* behind_cursor = NULL;
    size_t count = 0;
    while (cursor != NULL && count < start){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }

    chunk* end_cursor = doc->head;
    chunk* end_behind = NULL;
    count = 0;
    while (end_cursor != NULL && count < end){
        end_behind = end_cursor;
        end_cursor = end_cursor->next;
        count++;
    }

    chunk* beginning_link = make_new_chunk('(');
    doc->size++;
    chunk* end_link = beginning_link;
    for (size_t i = 0; i < strlen(url); i++){
        end_link->next = make_new_chunk(url[i]);
        end_link = end_link->next;
        doc->size++;
    }
    end_link->next = make_new_chunk(')');
    end_link = end_link->next;
    doc->size++;
    
    chunk* beginning_title = make_new_chunk('[');
    chunk* end_title = make_new_chunk(']');
    end_title->next = beginning_link;
    doc->size++;
    doc->size++;
    

    if (behind_cursor == NULL && cursor != NULL){               // HANDLES BEGINNING
        chunk* temp = doc->head;
        doc->head = beginning_title;
        beginning_title->next = temp;
        end_behind->next = end_title;
        end_link->next = end_cursor;

    }
    if (end_cursor == NULL && behind_cursor != NULL){                                    // HANDLES WHEN INSERTING AT END
        behind_cursor->next = beginning_title;
        beginning_title->next = cursor;
        end_behind->next = end_title;
        
    }
    if (behind_cursor != NULL && cursor != NULL) {
        behind_cursor->next = beginning_title;
        beginning_title->next = cursor;
        end_behind->next = end_title;
        end_link->next = end_cursor;
    }
    return SUCCESS;
    
}

// === Utilities ===
void markdown_print(const document *doc, FILE *stream){
    chunk* cursor = doc->present;
    if (cursor != NULL){
        while (cursor != NULL){
            fputc(cursor->character, stream);
            cursor = cursor->next;
        }
        fflush(stream);
    }
}
char *markdown_flatten(const document *doc){
    if (doc->present_size <= 0){
        char* arr = malloc(sizeof(char));
        arr[0] = 0;
        return arr;
    }
    char* arr = malloc(sizeof(char)* doc->present_size + 1);
    arr[doc->present_size] = 0;
    chunk* csr = doc->present;
    for (unsigned int i = 0; i < doc->present_size; i++){
        arr[i] = csr->character;
        csr = csr->next;
    }
    return arr;
}


// === Versioning ===
void markdown_increment_version(document *doc){
    doc->version++;
    chunk* cursor = doc->present;
    while (cursor != NULL){
        chunk* temp = cursor;
        cursor = cursor->next;
        free(temp);
    }

    if (doc->head == NULL){
        doc->present = NULL;
        return;
    }
    doc->present_size = doc->size;
    cursor = doc->head;
    while (cursor->marked_for_deletion == 1 && cursor != NULL){
        cursor = cursor->next;
    }
    chunk* beginning_chunk = make_new_chunk(cursor->character);
    chunk* temp = beginning_chunk;
    cursor = cursor->next;
    while (cursor != NULL){
        if (cursor->marked_for_deletion == 0){
            chunk* new_chunk = make_new_chunk(cursor->character);
            temp->next = new_chunk;
            temp = new_chunk;   
        }
        cursor = cursor->next;
    }
    doc->present = beginning_chunk;
    
    cursor = doc->head;
    while (cursor != NULL){
        chunk* temp = cursor;
        cursor = cursor->next;
        free(temp);
    }

    if (doc->present == NULL){
        doc->head = NULL;
        return;
    }
    doc->size = doc->present_size;
    cursor = doc->present;
    beginning_chunk = make_new_chunk(cursor->character);
    beginning_chunk->new_insert = 0;
    temp = beginning_chunk;
    cursor = cursor->next;
    while (cursor != NULL){
        if (cursor->marked_for_deletion == 0){
            chunk* new_chunk = make_new_chunk(cursor->character);
            new_chunk->new_insert = 0;
            temp->next = new_chunk;
            temp = new_chunk;   
        }
        cursor = cursor->next;
    }
    doc->head = beginning_chunk;
    
}


int markdown_present_insert(document *doc, uint64_t version, size_t pos, const char *content){
    chunk* cursor = doc->present;
    size_t count = 0;
    chunk* behind_cursor = NULL;

    if (version != doc->version){
        return OUTDATED_VERSION;
    }

    if (pos > doc->present_size){
        return INVALID_CURSOR_POS;
    }

    
    while (cursor != NULL && count < pos){
        behind_cursor = cursor;
        cursor = cursor->next;
        count++;
    }
    if (cursor == NULL){ // handles start and end of document inserts
        chunk* new_chunk = make_new_chunk(content[0]);
        if (behind_cursor == NULL){
            doc->present = new_chunk;
        }
        else{
            behind_cursor->next = new_chunk;
        }
        // doc->size++;
        chunk* temp = new_chunk;
        for (size_t i = 1; i < strlen(content); i++){
            new_chunk = make_new_chunk(content[i]);
            temp->next = new_chunk;
            temp = new_chunk;
            // doc->size++;
        }
        return SUCCESS;
    }
    else {
        chunk* new_chunk = make_new_chunk(content[0]);

        if (behind_cursor == NULL){
            doc->present = new_chunk;
        }
        else{
            behind_cursor->next = new_chunk;
        }
        
        chunk* temp = new_chunk;
        // doc->size++;
        for (size_t i = 1; i < strlen(content); i++){
            new_chunk = make_new_chunk(content[i]);
            temp->next = new_chunk;
            temp = new_chunk;
            // doc->size++;
        }
        temp->next = cursor;
        return SUCCESS;
    }
    
}
// int main(){
//     document* doc = markdown_init();
//     markdown_insert(doc, doc->version, 0 , "hello there");
//      markdown_increment_version(doc); 
//     markdown_delete(doc, doc->version, 0, 11);
//     markdown_insert(doc, doc->version, 4, "hey1");
//     markdown_insert(doc, doc->version, 2, "hey2");
// //     markdown_insert(doc, 0, 0, "I love comp2017");
// //     markdown_increment_version(doc); 
// //     markdown_delete(doc, doc->version, 1, 5);
// //     markdown_bold(doc, doc->version, 2, 6);
//  markdown_increment_version(doc); 

//     char* te = markdown_flatten(doc);

//     markdown_free(doc);
// }