#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server.h"
#include "socket.h"
#include "connection.h"

struct option_struct {
	char *port;
	int help;
};

void parse_args(int argc, char *argv[], struct option_struct *opts)
{
	int ch;

	while ((ch = getopt(argc, argv, "p:h")) != -1)
	{
		switch (ch)
		{
		case 'p':
			opts->port = optarg;
			break;
		case 'h':
		default:
			opts->help = 1;
		}
	}
}

void print_usage(char *name)
{
	printf(
		"usage: %s [-h] [-p port]\n",
		name
	);
}

static socklen_t socklen = sizeof(struct sockaddr);

int main(int argc, char *argv[])
{
	struct sockaddr_in listen_addr;
	struct plxr_connection conn = {0};
	struct option_struct opts = {0};
	int listen_fd;
	int port = 8080;

	parse_args(argc, argv, &opts);
	if (opts.help) {
		print_usage(argc > 0 ? argv[0] : "plexer");
		return 0;
	}
	if (opts.port) {
		port = atoi(opts.port);
	}

	listen_fd = plxr_socket_listen(&listen_addr, port);
	if (listen_fd == -1) {
		perror("plxr_socket_listen");
		return 1;
	}

	printf("[*] listening on port: %d\n", port);

	do {
		conn.fd = accept(listen_fd, &conn.addr, &socklen);

		if (conn.fd == -1) {
			perror("accept");
			continue;
		}

		printf("[*] client connected %s\n", plxr_socket_ntop(&conn.addr));

		if (plxr_connection_read(&conn) == -1)
			printf("plxr_connection_read: failed\n");

		if (plxr_serve_file_or_dir(&conn) == -1)
			printf("plxr_serve_file_or_dir: failed\n");

		shutdown(conn.fd, SHUT_RDWR);
		close(conn.fd);
	}
	while (1);

	return 0;
}
