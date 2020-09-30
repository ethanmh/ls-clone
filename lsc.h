#ifndef _LSC_H_
#define _LSC_H_

#include <stdbool.h>

typedef struct options_t {
    bool all;           // -a
    bool index;         // -i
    bool long_list;     // -l
    bool recursive;     // -R
    bool nice_size;     // -h
    bool print_header;  // not a user specified option, based on number of paths or -R
} options_t;

#endif