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

#include "file.h"

char *
plxr_alloc_file(const char *filename, off_t *size)
{
	struct stat info;
	char *buffer;
	int fd;

	if ((fd = open(filename, O_RDONLY)) == -1)
		goto error_exit;

	if ((fstat(fd, &info) != 0))
		goto error_cleanup_file;

	if ((buffer = malloc(info.st_size + 1)) == NULL)
		goto error_cleanup_file;

	if ((read(fd, buffer, info.st_size) == -1))
		goto error_cleanup_buffer;

	buffer[info.st_size] = '\0'; // null terminator
	*size = info.st_size;
	close(fd);
	return buffer;

error_cleanup_buffer:
	free(buffer);
error_cleanup_file:
	close(fd);
error_exit:
	return NULL;
}

char *
plxr_readdir()
{
	struct dirent *ent;
	DIR *dir;

	dir = opendir(".");

	if (dir == NULL)
		return NULL;

	while (( ent = readdir(dir) ))
		printf("%s\n", ent->d_name);

	closedir(dir);
	return NULL;
}
