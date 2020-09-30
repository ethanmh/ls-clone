#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "vector.h"

static const size_t GROW_CAPACITY = 25;

// creates a variably resized array of strings; initially length 0 and capacity GROW_CAPACITY
vector_t* vectorCreate() {
    vector_t* vector = malloc(sizeof(vector_t));

    if(!vector) {
        printf("malloc() failed...exiting\n");
        exit(EXIT_FAILURE);
    }

    vector->length = 0;
    vector->capacity = GROW_CAPACITY;
    vector->items = malloc(sizeof(char*) * GROW_CAPACITY);

    if(!vector->items) {
        printf("malloc() failed...exiting\n");
        exit(EXIT_FAILURE);
    }

    return vector;
}

// push a new string on a vector; resizes the vector if needed
void vectorPush(vector_t* vector, char* item) {
    // check if we're at capacity and need to resize
    if(vector->length == vector->capacity) {
        size_t realloc_size = sizeof(char*) * (vector->capacity + GROW_CAPACITY);

        vector->items = realloc(vector->items, realloc_size);

        if(!vector->items) {
            printf("realloc() failed...exiting\n");
            exit(EXIT_FAILURE);
        }

        vector->capacity += GROW_CAPACITY;
    }

    // actually add the item to the vector
    vector->items[ vector->length ] = item;
    vector->length++;
}

// qsort gives the comparator two pointers to elements in the list
// since each element is a char*, the void* is actually char**
static int qsortCmp(const void* a, const void* b) {
    return strcmp(*(char**)a, *(char**)b);
}

// sort a vector lexographically (according to strcmp)
void vectorSort(vector_t* vector) {
    qsort(vector->items, vector->length, sizeof(char*), qsortCmp);
}

// free memory malloc-ed by creating the vector; does not free any items contained within the vector
void vectorFree(vector_t* vector) {
    free(vector->items);
    free(vector);
}