#include "socket.h"

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <arpa/inet.h>

int
plxr_socket_listen(struct sockaddr_in *addr, int port)
{
	int sockfd;
	static int YES = 1;

	bzero(addr, sizeof(struct sockaddr_in));
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))
		== -1 )
		goto error;

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &YES, sizeof(YES))
		== -1 )
		goto error;

	if (bind(sockfd, (struct sockaddr *) addr, sizeof(*addr))
		== -1 )
		goto error;

	if (listen(sockfd, SOCK_STREAM)
		== -1 )
		goto error;

	return sockfd;
error:
	close(sockfd); /* avoid leaking descriptors */
	return -1;
}

static char ntop_buffer[INET_ADDRSTRLEN];

const char *
plxr_socket_ntop(struct sockaddr *addr)
{
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
	bzero(ntop_buffer, sizeof(ntop_buffer));
	inet_ntop(AF_INET, &addr_in->sin_addr, ntop_buffer, INET_ADDRSTRLEN);
	return (const char *) ntop_buffer;
}
