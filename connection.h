#ifndef PLXR_CONNECTION_H
#define PLXR_CONNECTION_H

#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include "socket.h" 
#include "log.h" 

struct plxr_conn_t {

	/* connection data */
	struct sockaddr addr;
	struct pollfd *pfd;
	int fd;
	int events;
	int attempts;
	size_t bytes_wrote;
	size_t bytes_read;
	int active;

	/* keeping track of free/alloc'd objects */
	int used;

	/* linked list */
	struct plxr_conn_t *next;

	/* user data */
	void *data;

};

/* Attempts to accept and configure a new 
 * connection from a listener socket `fd`.
 *
 * returns
 * 	on success: 0
 * 	on error: -1
 */
int plxr_conn_accept(struct plxr_conn_t *conn, int fd);

/* Close a connection, marking it as inactive.
 */
void plxr_conn_close(struct plxr_conn_t *conn);

#endif /* PLXR_CONNECTION_H */
