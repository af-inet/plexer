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

#endif /* PLXR_SOCKET_H */
