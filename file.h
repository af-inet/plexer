#ifndef PLXR_FILE_H
#define PLXR_FILE_H

/* reads the contents of a file into a `malloc`d buffer
 * returns pointer on success and sets `size` to the size of the buffer
 * returns NULL on failure
 */
char *
plxr_alloc_file(const char *filename, off_t *size);

typedef enum {
	PLX_FILE_ERR = -1,
	PLX_FILE_REG = 1,
	PLX_FILE_DIR = 2,
	PLX_FILE_NOT_FOUND = 3
} plxr_filetype_t;

plxr_filetype_t
plxr_stat(char *path);

#endif /* PLXR_FILE_H */
