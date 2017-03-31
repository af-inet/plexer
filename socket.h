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

/* Writes to a socket `fd` with a specified timeout in milliseconds from a buffer `data`.
 *
 * returns the number of bytes written on success (including zero),
 * or less than 0 on failure (or timeout).
 *     PLX_POLL_FAILED
 *     PLX_WRITE_TIMEOUT
 *     PLX_WRITE_FAILED
 *     PLX_BAD_WRITE
 */
int
plxr_socket_write_timeout(
	int fd,
	const char *data,
	size_t data_len,
	int timeout_milliseconds
);

/* Read from a socket `fd` with a specified timeout in milliseconds into a buffer at `data`
 *
 * returns the number of bytes read on success (including zero),
 * or less than 0 on failure (or timeout).
 *     PLX_POLL_FAILED
 *     PLX_READ_TIMEOUT
 *     PLX_READ_FAILED
 */
int
plxr_socket_read_timeout(
	int fd,
	char *data,
	size_t data_max,
	int timeout_milliseconds
);

#endif /* PLXR_SOCKET_H */
