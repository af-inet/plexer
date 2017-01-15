#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "file.h"
#include "http.h"
#include "socket.h"

int serve_file(int sockfd, char *filename)
{
	char headers[4096];
	size_t headers_len;
	char *data;
	off_t data_len;

	data = plxr_alloc_file(filename, &data_len);

	if (data == NULL)
		return -1;

	headers_len = plxr_http_response(
		headers, sizeof(headers), 200, data_len);

	if (headers_len > sizeof(headers))
		return -1; /* not enough memory */
	if (write(sockfd, headers, headers_len) != headers_len)
		return -1; /* didn't write expected length */
	if (write(sockfd, data, data_len) != data_len)
		return -1; /* didn't write expected length */

	return 0;
}

void handle_client(int client_fd, struct sockaddr *client_addr)
{
	printf("[*] client connected %s\n",
		plxr_socket_ntop(client_addr));

	if (serve_file(client_fd, "./index.html") == -1)
		perror("serve_file");

	shutdown(client_fd, SHUT_RDWR);
	close(client_fd);
}

int main(int argc, char *argv[])
{
	socklen_t socklen = sizeof(struct sockaddr);
	struct sockaddr_in listen_addr;
	struct sockaddr client_addr;
	int listen_fd;
	int client_fd;
	int port = 8080;

	listen_fd = plxr_socket_listen(&listen_addr, port);
	if (listen_fd == -1) {
		perror("plxr_socket_listen");
		return 1;
	}
	printf("[*] listening on port: %d\n", port);

	do {
		client_fd = accept(listen_fd, &client_addr, &socklen);
		if (client_fd == -1)
		{
			perror("accept");
			return 1;
		}
		handle_client(client_fd, &client_addr);
	}
	while (1);

	return 0;
}
