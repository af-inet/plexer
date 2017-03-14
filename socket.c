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

#include "error.h"

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

/* TODO: handle EAGAIN, EWOULDBLOCK. see write(2) */
int
plxr_socket_write_timeout(
	int fd,
	const char *data,
	size_t data_len,
	int timeout_milliseconds
){
	int ret;
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLOUT,
		.revents = 0
	};

	switch (poll(&pfd, 1, timeout_milliseconds)) {

	case -1:
		return PLX_POLL_FAILED;

	case 0:
		return PLX_WRITE_TIMEOUT; /* timeout */

	case 1: /* successful poll(2) */
		ret = write(fd, data, data_len);

		if (ret == -1)
			return PLX_WRITE_FAILED;
		if (ret != data_len)
			return PLX_BAD_WRITE; /* wrote less than the desired nbytes */

		return ret; /* success! return the number of bytes written */

	default:
		break;
	}

	return PLX_UNEXPECTED;
}

int
plxr_socket_read_timeout(
	int fd,
	char *data,
	size_t data_max,
	int timeout_milliseconds
){
	int ret;
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLIN,
		.revents = 0
	};

	switch (poll(&pfd, 1, timeout_milliseconds)) {

	case -1:
		return PLX_POLL_FAILED;

	case 0:
		return PLX_READ_TIMEOUT;

	case 1: /* successful poll(2) */
		ret = read(fd, (void *)data, data_max);

		if (ret == -1)
			return PLX_READ_FAILED;

		return ret;

	default:
		break;
	}

	return PLX_UNEXPECTED;
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
