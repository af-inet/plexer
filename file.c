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

static int nftw_cb_result;
static char *nftw_cb_parameter;

int
nftw_cb(const char *name, const struct stat *ptr, int flag, struct FTW *sftw)
{
	if ( (flag==FTW_F) || (flag==FTW_D))
	{
		if (strcmp(name, ".") == 0)
			return 0; /* exclude "." */

		name += 2; /* cut of the leading "./" */

		if (strcmp(name, nftw_cb_parameter) == 0)
		{
			nftw_cb_result = 0;
			return 1; /* found what we're looking for, exit */
		}
	}

	return 0;
}

int
plxr_in_dir_recursive(char *dirname, char *filename)
{
	nftw_cb_result = 1; /* will be set to `0` if the file is found */
	nftw_cb_parameter = filename;

	if (nftw(dirname, &nftw_cb, 1, FTW_PHYS | FTW_MOUNT) == -1)
	{
		return -1;
	}

	return nftw_cb_result;
}

int
plxr_check_dir(char *dirname, DIR *dir, char *path)
{
	struct stat info;

	switch (plxr_in_dir_recursive(dirname, path))
	{
	case 0:
		if (stat(path, &info) != 0)
			return PLX_FILE_ERR;
		if (S_ISREG(info.st_mode))
			return PLX_FILE_REG;
		if (S_ISDIR(info.st_mode))
			return PLX_FILE_DIR;
	case 1:
		return PLX_FILE_NOT_FOUND;
	case -1:
	default:
		break;
	}
	return PLX_FILE_ERR;
}

int
plxr_in_dir(DIR *dir, char *filename)
{
	struct dirent *ent;

	while (( ent = readdir(dir) ))
	{
		if (strcmp(ent->d_name, ".") == 0)
			continue; /* exclude "." */
		if (strcmp(ent->d_name, "..") == 0)
			continue; /* exclude ".." */
		if (strcmp(filename, ent->d_name) == 0)
			return 0; /* file found */
	}

	return 1; /* file not found */
}
