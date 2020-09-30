#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdlib.h>

typedef struct vector_t {
    size_t length;
    size_t capacity;
    char** items;
} vector_t;

vector_t* vectorCreate();
void vectorPush(vector_t* vector, char* item);
void vectorSort(vector_t* vector);
void vectorFree(vector_t* vector);

#endif