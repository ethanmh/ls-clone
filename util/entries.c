#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h> // strerror
#include <errno.h>
#include <time.h>
#include <pwd.h> // user
#include <grp.h> // group
#include <sys/stat.h> // lstat / fstatat
#include <unistd.h> // readlink
#include <fcntl.h> // AT_SYMLINK_NOFOLLOW

#include "../lsc.h"
#include "entries.h"
#include "utility.h"

// take a 'name' within an optional directory given by dir_fd (-1 if relative to cwd)
// and fill out the entry_info struct based upon the user's specified options
void processEntry(entry_info_t* entry_info, options_t* options, char* name, int dir_fd) {
    struct stat stat_entry;

    // if we need it based upon the current options, grab the stat_entry for the file
    if(options->index || options->long_list) {
        int stat_res;

        if(dir_fd != -1) {
            stat_res = fstatat(dir_fd, name, &stat_entry, AT_SYMLINK_NOFOLLOW);
        }
        else {
            stat_res = lstat(name, &stat_entry);
        }

        if(stat_res == -1) {
            printf("lsc: cannot access '%s': %s\n", name, strerror(errno));
            entry_info->error = true;
            return;
        }
    }

    if(options->index) {
        // INO
        snprintf(entry_info->ino, MAX_STR_INO, "%lu", (unsigned long)stat_entry.st_ino);
    }

    if(options->long_list) {
        // MODE
        getModeString(entry_info->mode, stat_entry.st_mode);

        // NLINKS
        snprintf(entry_info->nlinks, MAX_STR_NLINKS, "%u", (unsigned)stat_entry.st_nlink);

        // USER / GROUP
        struct passwd* user = getpwuid(stat_entry.st_uid);
        struct group* grp = getgrgid(stat_entry.st_gid);

        if(user) {
            snprintf(entry_info->user, MAX_STR_USER, "%s", user->pw_name);
        }
        else {
            snprintf(entry_info->user, MAX_STR_USER, "%u", stat_entry.st_uid);
        }

        if(grp) {
            snprintf(entry_info->group, MAX_STR_GROUP, "%s", grp->gr_name);
        }
        else {
            snprintf(entry_info->group, MAX_STR_GROUP, "%u", stat_entry.st_gid);
        }

        // SIZE
        if(options->nice_size) {
            getNiceSize(entry_info->size, stat_entry.st_size);
        }
        else {
            snprintf(entry_info->size, MAX_STR_SIZE, "%ld", (long)stat_entry.st_size);
        }

        // DATE / TIME:  mmm dd yyyy hh:mm
        strftime(entry_info->time, MAX_STR_TIME, "%b %e %Y %H:%M", localtime(&stat_entry.st_mtime));

        // if its a link, in long mode format we show the path where it's pointing
        // malloc here is freed when it is printed
        if(S_ISLNK(stat_entry.st_mode)) {
            char* buffer = malloc(sizeof(char) * MAX_STR_PATH);
            if(!buffer) {
                printf("malloc() failed...exiting\n");
                exit(EXIT_FAILURE);
            }

            ssize_t link_res;

            if(dir_fd != -1) {
                link_res = readlinkat(dir_fd, name, buffer, MAX_STR_PATH - 1);
            }
            else {
                link_res = readlink(name, buffer, MAX_STR_PATH - 1);
            }

            if(link_res == -1) {
                printf("lsc: cannot read symbolic link '%s': %s\n", name, strerror(errno));
                entry_info->error = true;
                free(buffer);
                return;
            }

            buffer[link_res] = '\0';

            entry_info->path = buffer;
        }
        else {
            entry_info->path = NULL;
        }
    }

    // NAME
    entry_info->name = name;
    entry_info->name_quotes = stringNeedsQuotes(name);

    entry_info->error = false;
}

// print the entry based upon the proper column widths and current options
void printEntry(column_widths_t* column_widths, entry_info_t* entry_info, options_t* options) {
    if(entry_info->error) {
        return;
    }

    if(options->index) {
        printf("%*s ", column_widths->ino, entry_info->ino);
    }

    if(options->long_list) {
        printf("%s ", entry_info->mode); // mode is fixed width
        printf("%*s ", column_widths->nlinks,   entry_info->nlinks);
        printf("%-*s ", column_widths->user,    entry_info->user);
        printf("%-*s ", column_widths->group,   entry_info->group);
        printf("%*s ", column_widths->size,     entry_info->size);
        printf("%s ", entry_info->time); // time is fixed width
    }

    // NAME
    if(!entry_info->name_quotes && column_widths->name_needs_space) {
        // line this name up if required (-l option and other file with quotes)
        printf(" %s", entry_info->name);
    }
    else {
        printWithQuotes(entry_info->name, entry_info->name_quotes);
    }

    // if this is a symlink, we'll print the path where it points
    // then free the malloc-ed memory. in C, we take out the garbage ourselves :)
    if(options->long_list && entry_info->path != NULL) {
        printf(" -> ");
        printWithQuotes(entry_info->path, stringNeedsQuotes(entry_info->path));
        free(entry_info->path);
    }

    printf("\n");
}