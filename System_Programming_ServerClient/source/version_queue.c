


#include "../libs/version_queue.h"


version_queue* version_queue_init(){
    version_queue* version_q = malloc(sizeof(version_queue));
    version_q->head = NULL;
    version_q->tail = NULL;
    return version_q;
}

version_node* make_new_version_node(){
    version_node* v_node = malloc(sizeof(version_node));
    v_node->command = NULL;
    v_node->username = NULL;
    v_node->success_reject = NULL;
    v_node->reason = NULL;
    v_node->next = NULL;
    return v_node;
}

void version_queue_enqueue(version_queue* version_q, version_node* version_n){
    if (version_q->head == NULL){
        version_q->head = version_n;
        version_q->tail = version_n;
        return;
    }
    version_q->tail->next = version_n;
    version_q->tail = version_n;
}

version_node* version_queue_dequeue(version_queue* version_q){
    if (version_q->head == NULL){
        printf("Command Queue is Empty\n");
        return NULL;
    }
    version_node* temp = version_q->head;
    version_q->head = version_q->head->next;
    if (version_q->head == NULL){
        version_q->tail = NULL;
    }
    return temp;
}

void version_queue_destroy(version_queue* version_queue){
    version_node* cursor = version_queue->head;
    version_node* tmp = NULL;
    while (cursor != NULL){
        tmp = cursor;
        cursor = cursor->next;
        free(tmp->command);
        free(tmp);
    }
    free(version_queue);
    
}