#include "socket.h"

void socket_die(char *reason){
	perror(reason);
	exit(EXIT_FAILURE);
}

// Close a socket before we return from an error, so we aren't leaking descriptors.
int socket_error(int sock){
	close(sock);
	return SOCKET_ERROR;
}

const char *socket_ntop(char *dest, struct sockaddr *addr) {
	struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
	return inet_ntop(AF_INET, &addr_in->sin_addr, dest, INET_ADDRSTRLEN);
}

int socket_tcp_listen(struct sockaddr_in *addr, int port){
	int sockfd;
	int ONE;
	bzero(addr, sizeof(struct sockaddr_in));

	ONE = 1;

	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if( sockfd == SOCKET_ERROR )
	{
		return socket_error(sockfd);
	}
	
	if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &ONE, sizeof(ONE))
		== SOCKET_ERROR )
	{
		return socket_error(sockfd);
	}

	if( bind(sockfd, (struct sockaddr *) addr, sizeof(*addr))
		== SOCKET_ERROR )
	{
		return socket_error(sockfd);
	}

	if( listen(sockfd, SOCK_STREAM) 
		== SOCKET_ERROR )
	{
		return socket_error(sockfd);
	}
	
	return sockfd;
}
