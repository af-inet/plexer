
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "connection.h"

/* since we're single threaded, we can use the same buffer for everyone */
static char request_buffer[4096];

int
plxr_connection_read(struct plxr_connection *conn)
{
	ssize_t req_len;
	bzero(request_buffer, sizeof(request_buffer));

	req_len = read(conn->fd, request_buffer, sizeof(request_buffer));

	if (req_len == -1)
		return -1; /* read failed */
	if (plxr_http_parse(&conn->request, request_buffer) == 0)
		return -1; /* parse failed */

	return 0;
}

