#include <stdlib.h>
#include <stdio.h>
#include <string.h> // strerror
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>

#include "lsc.h"
#include "util/vector.h"
#include "util/listing.h"

static const char PATH_INVALID = 0;
static const char PATH_DIRECTORY = 1;
static const char PATH_FILE = 2;

static char* current_directory = ".";

static bool argumentParser(int argc, char* argv[], options_t* options, vector_t* files, vector_t* directories);
static char checkPath(char* path, options_t* options);
static void usageMessage();

int main(int argc, char* argv[]) {
    options_t options = {false, false, false, false, false, false};

    vector_t* files = vectorCreate();
    vector_t* directories = vectorCreate();

    // parse arguments
    if(!argumentParser(argc, argv, &options, files, directories)) {
        vectorFree(files);
        vectorFree(directories);
        exit(EXIT_FAILURE);
    }

    // firstly list all of the given files
    listFiles(&options, files, -1);

    // newline between files and directories (only if we have both)
    if(files->length != 0 && directories->length != 0) {
        printf("\n");
    }

    // second list all of the given directories
    for(size_t i = 0; i < directories->length; i++) {
        listDirectory(&options, directories->items[i]);

        // add an additional newline only if this is not the last listing
        if(i < directories->length - 1) {
            printf("\n");
        }
    }

    // only free vectors, not the items because they are from argv
    vectorFree(files);
    vectorFree(directories);
    return 0;
}

static bool argumentParser(int argc, char* argv[], options_t* options, vector_t* files, vector_t* directories) {
    // program name is argv[0], skip it
    int i = 1;

    // all options start with a '-'
    for(; i < argc && argv[i][0] == '-'; i++) {
        // multiple options may be provided in a "chunk" (e.g. -ilR), loop through each
        char* c = &argv[i][1];

        // if a user provides just a '-', then fail
        if(*c == '\0') {
            printf("\n No option given with '-'\n");
            usageMessage();
            return false;
        }

        for(; *c != '\0'; c += sizeof(char)) {
            switch(*c) {
                case 'a':
                    options->all = true;
                    break;
                case 'h':
                    options->nice_size = true;
                    break;
                case 'i':
                    options->index = true;
                    break;
                case 'l':
                    options->long_list = true;
                    break;
                case 'R':
                    options->recursive = true;
                    break;
                default:
                    printf("\n Unrecognized option: %c\n", *c);
                    usageMessage();
                    return false;
            }
        }
    }

    if(i == argc) {
        // user provided no paths, so let's use the current directory
        vectorPush(directories, current_directory);
    }

    if(i <= argc - 2 || options->recursive) {
        // user has provided multiple paths or we're printing recursively, so enable printing headers
        options->print_header = true;
    }

    // loop through each path and determine if it is a file or a directory
    for(; i < argc; i++) {
        // checkPath prints an error if the path is invalid
        char path_type = checkPath(argv[i], options);

        if(path_type == PATH_FILE) {
            vectorPush(files, argv[i]);
        }
        else if(path_type == PATH_DIRECTORY) {
            vectorPush(directories, argv[i]);
        }
    }

    vectorSort(files);
    vectorSort(directories);

    return true;
}

// check whether the path is valid or not
// only applicable to user-provided paths
static char checkPath(char* path, options_t* options) {
    struct stat stat_entry;

    // check if path is valid, if not then print error message
    if(lstat(path, &stat_entry) == -1) {
        printf("lsc: cannot access '%s': %s\n", path, strerror(errno));
        return PATH_INVALID;
    }

    // DIRECTORY
    if(S_ISDIR(stat_entry.st_mode)) {
        return PATH_DIRECTORY;
    }

    // SYMLINK
    if(S_ISLNK(stat_entry.st_mode)) {
        // the following is to match the exact behaviour of ls
        
        // if -l, then we treat a symlink as a file, otherwise treat it as what it points to
        if(options->long_list) {
            return PATH_FILE;
        }
        // if not -l nor -i, then we'll list the files inside the directory if it points to a directory
        else if(stat(path, &stat_entry) != -1 && S_ISDIR(stat_entry.st_mode)) {
            return PATH_DIRECTORY;
        }
        else {
            return PATH_FILE;
        }
    }
    
    return PATH_FILE;
}

static void usageMessage() {
    printf("\n");
    printf(" Usage: ./lsc [options] [paths]\n");
    printf("        [options] and [paths] are optional\n\n");

    printf(" Options (prefix with '-'):\n");
    printf("     a: show all files\n");
    printf("     h: show human-readable sizes (to be used with 'l')\n");
    printf("     i: show file inode numbers\n");
    printf("     l: long format list\n");
    printf("     R: recursive list\n\n");

    printf(" Paths: one or more absolute or relative paths separated by spaces; if none provided the current directory is assumed\n");
    printf("\n");
}