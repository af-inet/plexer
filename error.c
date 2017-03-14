#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

static char plxr_buf[256];

const char *
plxr_unknown_error(unsigned int errnum)
{
	bzero(plxr_buf, sizeof(plxr_buf));
	snprintf(plxr_buf, sizeof(plxr_buf), "unknown error (%u)", errnum);
	return plxr_buf;
}

const char *
plxr_strerror(unsigned int errnum)
{
	return (-errnum < PLX_NERR)
		? plxr_error_string[-errnum]
		: plxr_unknown_error(-errnum);
}


