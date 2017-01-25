#ifndef PLXR_CONNECTION_H
#define PLXR_CONNECTION_H

#include <arpa/inet.h>

#include "http.h"

struct plxr_connection {
	int fd;
	struct sockaddr addr;
	struct plxr_http_request request;
};

int
plxr_connection_read(struct plxr_connection *conn);

#endif /* PLXR_CONNECTION_H */
