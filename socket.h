#ifndef PLXR_SOCKET_H
#define PLXR_SOCKET_H

#include <netinet/in.h>

/* creates & binds a new listener socket
 * returns a new file descriptor on success or -1 on failure
 */
int
plxr_socket_listen(struct sockaddr_in *addr, int port);

/* wrapper around inet_ntop
 */
const char *
plxr_socket_ntop(struct sockaddr *addr);

// TODO: move read / write logic under a nicer abstraction.
// right now the server logic has to worry about too many low level issues.

/* uses poll(2) to implement a nonblocking timeout */
int
plxr_socket_write_timeout(
	int fd,
	const char *data,
	size_t data_len,
	int timeout_milliseconds
);

/* ditto */
int
plxr_socket_read_timeout(
	int fd,
	char *data,
	size_t data_max,
	int timeout_milliseconds
);

#endif /* PLXR_SOCKET_H */
