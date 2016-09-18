#include "connection.h"

static socklen_t sockaddr_len = sizeof(struct sockaddr);

int plxr_conn_accept(struct plxr_conn_t *conn, int fd) {
	int nfd;

	nfd = accept(fd, &conn->addr, &sockaddr_len);
	
	if( nfd == SOCKET_ERROR ) {
		ERROR("accept");
		return -1;
	}
	
	if( fcntl(nfd, F_SETFD, O_NONBLOCK) == -1 ) {
		ERROR("fcntl");
		return -1;
	}

	// success, configure the new connection
	conn->fd = nfd;

	conn->pfd->fd = conn->fd;

	// TODO: check if POLLHUP / POLLERR actually 
	// need to be set or if they're implied.
	conn->pfd->events = POLLIN | POLLOUT | POLLHUP | POLLERR;
	
	return 0;
}

void plxr_conn_close(struct plxr_conn_t *conn) {
	if ( shutdown(conn->fd, SHUT_RDWR) == -1 ) {
		ERROR("shutdown");
	}
	/* There isn't really a way to handle a failed close, 
	 * so we only log it for now.
	 */
	if ( close(conn->fd) == -1 ) {
		ERROR("close");
	}

	/* To be safe, we set unused `fd`s to -1 
	 * (instead of 0, which is a valid fd).
	 * This way, if we accidently try to read/write we'll get 
	 * an appropriate error, (EBADF?) ...
	 */
	conn->fd = -1;

	/* ... poll(3) actually documents something similar
	 * "If the value of fd is less than 0, events shall be ignored, 
	 * and revents shall be set to 0 in that entry on return from poll()."
	 */
	conn->pfd->fd = -1;
}

