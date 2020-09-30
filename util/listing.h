#ifndef _LISTING_H_
#define _LISTING_H_

#include <stdbool.h>
#include "../lsc.h"
#include "vector.h"

void listFiles(options_t* options, vector_t* files, int dir_fd);
void listDirectory(options_t* options, char* path);

#endif