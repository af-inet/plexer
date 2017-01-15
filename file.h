#ifndef PLXR_FILE_H
#define PLXR_FILE_H

/* reads the contents of a file into a `malloc`d buffer
 * returns pointer on success and sets `size` to the size of the buffer
 * returns NULL on failure
 */
char *
plxr_alloc_file(const char *filename, off_t *size);

char *
plxr_readdir();

#endif /* PLXR_FILE_H */
