// _DEFAULT_SOURCE needed for d_type and DT_DIR
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // strerror
#include <errno.h>
#include <dirent.h>

#include "listing.h"
#include "vector.h"
#include "entries.h"
#include "utility.h"

// lists a given vector of files relative to dir_fd (-1 if relative to cwd)
void listFiles(options_t* options, vector_t* files, int dir_fd) {
    if(files->length == 0) {
        return;
    }

    entry_info_t* entry_infos = malloc(sizeof(entry_info_t) * files->length);
    column_widths_t* column_widths = malloc(sizeof(column_widths_t));

    if(!entry_infos || !column_widths) {
        printf("malloc() failed... exiting\n");
        exit(EXIT_FAILURE);
    }
    
    // process each entry and put the results into an entry_info_t struct
    for(size_t i = 0; i < files->length; i++) {
        processEntry(&entry_infos[i], options, files->items[i], dir_fd);
    }

    // based upon all of the entry_info_t structs, grab the proper column widths
    getColumnWidths(column_widths, entry_infos, files->length, options);

    // print all the entries
    for(size_t i = 0; i < files->length; i++) {
        printEntry(column_widths, &entry_infos[i], options);
    }

    // finished printing these files, free memory malloc-ed by this function
    free(entry_infos);
    free(column_widths);
}

// lists the given directory (recursive with -R option)
void listDirectory(options_t* options, char* path) {
    DIR* directory = opendir(path);

    // ensure opendir was successful
    // if not, print an error and return
    if(directory == NULL) {
        printf("lsc: cannot open directory '%s': %s\n", path, strerror(errno));
        return;
    }

    if(options->print_header) {
        printWithQuotes(path, stringNeedsQuotes(path));
        printf(":\n");
    }

    // entries in this directory, whether a subdirectory or a file
    vector_t* entry_vector = vectorCreate();

    // directory vector (used for -R option)
    vector_t* dir_vector = vectorCreate();

    // loop through all entries in the directory
    for(struct dirent* entry = readdir(directory); entry != NULL; entry = readdir(directory)) {
        // skip the entry if it's an empty string
        if(entry->d_name[0] == '\0') {
            continue;
        }

        // if we're not printing all, hide hidden files (starting with '.')
        if(!options->all && entry->d_name[0] == '.') {
            continue;
        }

        // malloc and make a copy of the entry name
        // freed at the bottom of this function
        size_t entry_name_len = strlen(entry->d_name) + 1;
        char* entry_name = malloc(sizeof(char) * entry_name_len);

        if(entry_name == NULL) {
            printf("malloc() failed... exiting\n");
            exit(EXIT_FAILURE);
        }

        // put this file name into the entry vector
        strncpy(entry_name, entry->d_name, entry_name_len);
        vectorPush(entry_vector, entry_name);
        
        // if we're recursively printing and this is a directory, also put it into the dir vector
        if(options->recursive && entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            vectorPush(dir_vector, entry_name);
        }
    }

    // sort both vectors
    vectorSort(entry_vector);
    vectorSort(dir_vector);

    int dir_fd = dirfd(directory);

    if(dir_fd == -1) {
        printf("dirfd() failed with error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // print all of the files we grabbed in this directory
    listFiles(options, entry_vector, dir_fd);

    // if we're recursively printing, we might have some directories in dir_vector to list
    for(size_t i = 0; i < dir_vector->length; i++) {
        printf("\n");

        int path_length = strlen(path);

        // ls ignores extra slashes at the end of the directory, so cut them off
        while(path_length > 0 && path[path_length - 1] == '/') {
            path_length--;
        }

        // create a string for the current path + '/' + the name of the directory in dir_vector
        // +2: 1 for '/', 1 for '\0'
        size_t new_path_len = path_length + strlen(dir_vector->items[i]) + 2;
        char* new_path = malloc(sizeof(char) * new_path_len);

        if(new_path == NULL) {
            printf("malloc() failed... exiting\n");
            exit(EXIT_FAILURE);
        }

        snprintf(new_path, new_path_len, "%.*s/%s", path_length, path, dir_vector->items[i]);

        // recursively list the directory
        // base case: no more directories
        listDirectory(options, new_path);

        free(new_path);
    }

    // items inside dir_vector are the same as those in entry_vector, only free once
    for(size_t i = 0; i < entry_vector->length; i++) {
        free(entry_vector->items[i]);
    }

    // free malloc-ed memory :)
    vectorFree(entry_vector);
    vectorFree(dir_vector);

    closedir(directory);
}