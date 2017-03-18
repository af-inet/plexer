#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

static char plxr_error_buf[256];

static const char *
plxr_error_table[PLX_NERR] =
{
	[0]                      = "not an error code (0)",
	[-PLX_UNEXPECTED]        = "an unexpected error occurred",
	[-PLX_OUT_OF_MEMORY]     = "ran out of memory",
	[-PLX_POLL_FAILED]       = "poll(2) returned an error code",
	[-PLX_WRITE_FAILED]      = "write(2) returned an error code",
	[-PLX_BAD_WRITE]         = "write(2) returned < the expected nbytes",
	[-PLX_READ_FAILED]       = "read(2) returned an error code",
	[-PLX_PARSE_FAILED]      = "failed to parse http request",
	[-PLX_READ_TIMEOUT]      = "read(2) timed out",
	[-PLX_WRITE_TIMEOUT]     = "write(2) timed out",
	[-PLX_ALLOC_FILE_FAILED] = "plxr_alloc file failed"
};

const char *
plxr_unknown_error(int errnum)
{
	bzero(plxr_error_buf, sizeof(plxr_error_buf));
	snprintf(plxr_error_buf, sizeof(plxr_error_buf), "undefined error code (%d)", errnum);
	return plxr_error_buf;
}

const char *
plxr_not_error(int errnum)
{
	bzero(plxr_error_buf, sizeof(plxr_error_buf));
	snprintf(plxr_error_buf, sizeof(plxr_error_buf), "not an error code (%d)", errnum);
	return plxr_error_buf;
}

const char *
plxr_known_error(int errnum)
{
	const char *errmsg = plxr_error_table[-errnum];
	if (errmsg == NULL)
		return plxr_unknown_error(errnum); /* error code was not defined */

	bzero(plxr_error_buf, sizeof(plxr_error_buf));
	snprintf(plxr_error_buf, sizeof(plxr_error_buf), "%s (%d)", errmsg, errnum);
	return plxr_error_buf;
}

const char *
plxr_strerror(int errnum)
{
	if (errnum >= 0)
		return plxr_not_error(errnum); /* error codes are not positive */
	else
		return (-errnum < PLX_NERR)
			? plxr_known_error(errnum)
			: plxr_unknown_error(errnum); /* error code out of range */
}
