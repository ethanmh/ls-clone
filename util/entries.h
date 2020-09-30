#ifndef _ENTRIES_H_
#define _ENTRIES_H_

#include <stdbool.h>
#include "../lsc.h"

#define QUOTE_NONE 0
#define QUOTE_SINGLE 1
#define QUOTE_DOUBLE 2

// max string sizes (including \0)
#define MAX_STR_INO 21
#define MAX_STR_MODE 11
#define MAX_STR_NLINKS 11
#define MAX_STR_USER 33
#define MAX_STR_GROUP 33
#define MAX_STR_SIZE 21
#define MAX_STR_TIME 18
#define MAX_STR_PATH 4097

// one entry struct will be filled per file / directory
typedef struct entry_info_t {
    bool error;
    char ino[MAX_STR_INO];
    char mode[MAX_STR_MODE];
    char nlinks[MAX_STR_NLINKS];
    char user[MAX_STR_USER];
    char group[MAX_STR_GROUP];
    char size[MAX_STR_SIZE];
    char time[MAX_STR_TIME];
    char* name;
    char name_quotes; // either QUOTE_NONE, QUOTE_SINGLE, QUOTE_DOUBLE
    char* path; // for symlinks, big so will malloc if needed
} entry_info_t;

// columns that are variable width
typedef struct column_widths_t {
    unsigned ino;
    unsigned nlinks;
    unsigned user;
    unsigned group;
    unsigned size;
    bool name_needs_space; // fix spacing if one of the file names will be wrapped in quotes
} column_widths_t;

void processEntry(entry_info_t* entry_info, options_t* options, char* name, int dir_fd);
void printEntry(column_widths_t* column_widths, entry_info_t* entry_info, options_t* options);

#endif