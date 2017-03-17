#ifndef PLXR_FILE_H
#define PLXR_FILE_H

/* reads the contents of a file into a `malloc`d buffer
 * returns pointer on success and sets `size` to the size of the buffer
 * returns NULL on failure
 */
char *
plxr_alloc_file(const char *filename, off_t *size);

/* returns 0 if directory `dirname` contains file `filename`
 * returns 1 if it does not
 */
int
plxr_in_dir(DIR *dir, char *filename);

/* returns 0 if directory `dirname` contains file `filename`
 * returns 1 if it does not
 * returns -1 on error
 */
int
plxr_in_dir_recursive(char *filename);

enum {
	PLX_FILE_ERR = -1,
	PLX_FILE_REG = 1,
	PLX_FILE_DIR = 2,
	PLX_FILE_NOT_FOUND = 3
};

int
plxr_check_dir(DIR *dir, char *path);

#endif /* PLXR_FILE_H */
