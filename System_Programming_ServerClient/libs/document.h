#ifndef DOCUMENT_H

#define DOCUMENT_H
#include <stdint.h>
/**
 * This file is the header file for all the document functions. You will be tested on the functions inside markdown.h
 * You are allowed to and encouraged multiple helper functions and data structures, and make your code as modular as possible. 
 * Ensure you DO NOT change the name of document struct.
 */

typedef struct chunk chunk;

struct chunk{
    char character;
    unsigned int marked_for_deletion;
    unsigned int new_insert;
    chunk* next;
};

typedef struct {
    chunk* head;
    chunk* present;
    uint64_t version;
    unsigned int size;
    unsigned int present_size;
} document;


// Functions from here onwards.
#endif
