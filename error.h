#ifndef PLXR_ERROR
#define PLXR_ERROR

#include <stdlib.h>

#define PLX_UNEXPECTED        -1
#define PLX_OUT_OF_MEMORY     -2
#define PLX_POLL_FAILED       -3
#define PLX_WRITE_FAILED      -4
#define PLX_BAD_WRITE         -5
#define PLX_READ_FAILED       -6
#define PLX_PARSE_FAILED      -7
#define PLX_READ_TIMEOUT      -8
#define PLX_WRITE_TIMEOUT     -9
#define PLX_ALLOC_FILE_FAILED -10

#define PLX_NERR 11

const char *
plxr_strerror(int errnum);

#endif /* PLXR_ERROR */
