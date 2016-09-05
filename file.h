#ifndef PLXR_FILE_H
#define PLXR_FILE_H

#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

/* Reads the contents of a file to a malloc()'d, null terminated string.
 * returns NULL on error.
 */
char *plxr_alloc_file(const char *filename, off_t *size);

void plxr_ntfw(void (*callback)(const char *filename));

void plxr_display_info();

#endif /* PLXR_FILE_H */
