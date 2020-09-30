#include <stdlib.h>
#include <stdio.h>
#include <string.h> // memset / strlen
#include <errno.h>
#include <sys/stat.h> // constants for mode

#include "../lsc.h"
#include "utility.h"
#include "entries.h"

// return the maximum of a, b (used in getColumnWidths)
static unsigned max(unsigned a, unsigned b) {
    return a > b ? a : b;
}

// get the column widths based upon the max length of the data in each column
void getColumnWidths(column_widths_t* column_widths, entry_info_t* entry_infos, size_t num_entries, options_t* options) {
    // zero out the lengths initially
    memset(column_widths, 0, sizeof(column_widths_t));

    for(size_t i = 0; i < num_entries; i++) {
        if(entry_infos[i].error) {
            continue;
        }

        if(options->index) {
            column_widths->ino = max(column_widths->ino, strlen(entry_infos[i].ino));
        }

        if(options->long_list) {
            column_widths->nlinks = max(column_widths->nlinks, strlen(entry_infos[i].nlinks));
            column_widths->user = max(column_widths->user, strlen(entry_infos[i].user));
            column_widths->group = max(column_widths->group, strlen(entry_infos[i].group));
            column_widths->size = max(column_widths->size, strlen(entry_infos[i].size));
        }

        // if we're printing with the -l option, line up file names if one has quotes
        if(options->long_list && !column_widths->name_needs_space) {
            // haven't found a name needing a quote yet, check this one
            column_widths->name_needs_space = entry_infos[i].name_quotes ? true : false;
        }
    }
}

// get a mode string based upon the mode_t value
void getModeString(char* buffer, mode_t mode) {
    // first character shows the type of the file
    if(S_ISBLK(mode)) {
        buffer[0] = 'b';
    }
    else if(S_ISCHR(mode)) {
        buffer[0] = 'c';
    }
    else if(S_ISDIR(mode)) {
        buffer[0] = 'd';
    }
    else if(S_ISLNK(mode)) {
        buffer[0] = 'l';
    }
    else if(S_ISFIFO(mode)) {
        buffer[0] = 'p';
    }
    else if(S_ISSOCK(mode)) {
        buffer[0] = 's';
    }
    else {
        buffer[0] = '-';
    }

    // user permissions
    buffer[1] = mode & S_IRUSR ? 'r' : '-';
    buffer[2] = mode & S_IWUSR ? 'w' : '-';

    // setuid sticky bit
    if(mode & S_ISUID) {
        buffer[3] = mode & S_IXUSR ? 's' : 'S';
    }
    else {
        buffer[3] = mode & S_IXUSR ? 'x' : '-';
    }

    // group permissions
    buffer[4] = mode & S_IRGRP ? 'r' : '-';
    buffer[5] = mode & S_IWGRP ? 'w' : '-';

    // setgid sticky bit
    if(mode & S_ISGID) {
        buffer[6] = mode & S_IXGRP ? 's' : 'S';
    }
    else {
        buffer[6] = mode & S_IXGRP ? 'x' : '-';
    }

    // others permissions
    buffer[7] = mode & S_IROTH ? 'r' : '-';
    buffer[8] = mode & S_IWOTH ? 'w' : '-';

    // sticky bit
    if(S_ISDIR(mode) && (mode & S_ISVTX)) {
        buffer[9] = mode & S_IXOTH ? 't' : 'T';
    }
    else {
        buffer[9] = mode & S_IXOTH ? 'x' : '-';
    }

    buffer[10] = '\0';
}

// get human readable size (in bytes, KB, MB or GB)
void getNiceSize(char* buffer, size_t size) {
    const int DIVISOR = 1024;

    char symbol = 'B';
    float floatsize = (float)size;
    int decimals = 0;
    
    if(floatsize > DIVISOR) {
        symbol = 'K';
        floatsize /= DIVISOR;
        decimals = 1;
    }

    if(floatsize > DIVISOR) {
        symbol = 'M';
        floatsize /= DIVISOR;
    }

    if(floatsize > DIVISOR) {
        symbol = 'G';
        floatsize /= DIVISOR;
    }

    snprintf(buffer, MAX_STR_SIZE, "%.*f%c", decimals, floatsize, symbol);
}

char stringNeedsQuotes(char* name) {
    // check for the prescence of special characters requiring quotes
    const char* chars_need_double = "'";
    const char* chars_need_single = " `!$^&*()[\"?;|<>=";

    for(int i = 0; i < strlen(chars_need_double); i++) {
        if(strchr(name, chars_need_double[i]) != NULL) {
            return QUOTE_DOUBLE;
        }
    }

    for(int i = 0; i < strlen(chars_need_single); i++) {
        if(strchr(name, chars_need_single[i]) != NULL) {
            return QUOTE_SINGLE;
        }
    }

    return QUOTE_NONE;
}

void printWithQuotes(char* str, char needs_quotes) {
    switch(needs_quotes) {
        case QUOTE_NONE:
            printf("%s", str);
            return;
        case QUOTE_SINGLE:
            printf("'%s'", str);
            return;
        case QUOTE_DOUBLE:
            printf("\"%s\"", str);
            return;
    }
}