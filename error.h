#ifndef PLXR_ERROR
#define PLXR_ERROR

#include <stdlib.h>

static const char *
plxr_error_string[] = {
	"unknown error",

#define PLX_UNEXPECTED        -1  /* reserved for undefined errors */
	"unexpected error",

#define PLX_OUT_OF_MEMORY     -2  /* ran out of memory */
	"out of memory",

#define PLX_POLL_FAILED       -3  /* poll(2) returned an error code */
	"poll failed",

#define PLX_WRITE_FAILED      -4  /* write(2) returned an error code */
	"write failed",

#define PLX_BAD_WRITE         -5  /* write(2) returned a value != nbyte */
	"bad write",

#define PLX_READ_FAILED       -6  /* read(2) returned an error code */
	"read failed",

#define PLX_PARSE_FAILED      -7  /* failed to parse http request */
	"parse failed",

#define PLX_READ_TIMEOUT      -8  /* a read operation timed out */
	"read timeout",

#define PLX_WRITE_TIMEOUT     -9  /* a write operation timed out */
	"write timeout",

#define PLX_ALLOC_FILE_FAILED -10  /* plxr_alloc_file failed */
	"plxr_alloc_file() failed",
};

#define PLX_NERR 10

const char *
plxr_strerror(unsigned int errnum);

#endif /* PLXR_ERROR */
