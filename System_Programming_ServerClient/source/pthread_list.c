#include "../libs/pthread_list.h"





pthread_dynamic_list* pthread_list_init(){
    pthread_dynamic_list* thread_list = malloc(sizeof(pthread_dynamic_list));
    thread_list->head = NULL;
    thread_list->tail = NULL;
    thread_list->size = 0;
    return thread_list;
}

thread_struct* pthread_list_add_new(pthread_dynamic_list* pthread_list){
    thread_struct* new_thread = malloc(sizeof(thread_struct));
    new_thread->disconneted = 0;
    new_thread->next = NULL;
    if (pthread_list->head == NULL){
        pthread_list->head = new_thread;
        pthread_list->tail = new_thread;
        pthread_list->size++;
        return new_thread;
    }
    pthread_list->tail->next = new_thread;
    pthread_list->tail = new_thread;
    pthread_list->size++;
    return new_thread;
}
void free_pthread_list(pthread_dynamic_list* pthread_list){
    thread_struct* csr = pthread_list->head;
    thread_struct* temp = NULL;
    while (csr != NULL){
        temp = csr;
        csr = csr->next;
        free(temp);
    }
    free(pthread_list);
}



