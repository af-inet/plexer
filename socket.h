#ifndef PLXR_SOCKET_H
#define PLXR_SOCKET_H

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define SOCKET_ERROR (-1)

void socket_die(char *reason);

int socket_error(int sock);

int socket_tcp_listen(struct sockaddr_in *addr, int port);

#endif

