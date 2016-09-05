#ifndef PLXR_SOCKET_H
#define PLXR_SOCKET_H

#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <arpa/inet.h>

#define SOCKET_ERROR (-1)

typedef char addrstr_t[INET_ADDRSTRLEN];

const char *socket_ntop(char *dest, struct sockaddr *addr);

void socket_die(char *reason);

int socket_error(int sock);

int socket_tcp_listen(struct sockaddr_in *addr, int port);

#endif

