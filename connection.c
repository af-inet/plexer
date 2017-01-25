
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "connection.h"

int
plxr_connection_read(struct plxr_connection *conn)
{
	char req_buffer[4096];
	ssize_t req_len;

	req_len = read(conn->fd, req_buffer, sizeof(req_buffer));

	if (req_len == -1)
		return -1; /* read failed */
	if (plxr_http_parse(&conn->request, req_buffer) == 0)
		return -1; /* parse failed */

	return 0;
}

