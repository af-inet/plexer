#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>

#include "server.h"
#include "socket.h"
#include "connection.h"

struct options {
	char *port;
	char verbose;
	int help;
};

static struct options opts = {0};

void parse_args(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "p:hv")) != -1)
	{
		switch (ch)
		{
		case 'v':
			opts.verbose = 1;
			break;
		case 'p':
			opts.port = optarg;
			break;
		case 'h':
		default:
			opts.help = 1;
		}
	}
}

void sigpipe_handler(int sig)
{
	if (opts.verbose) {
		printf("[!] caught SIGPIPE\n");
	}
}

void print_usage(char *name)
{
	printf(
		"usage: %s [-hv] [-p port]\n"
		"  -h  display this help page\n"
		"  -v  verbose\n"
		"  -p  port number\n",
		name
	);
}

static socklen_t socklen = sizeof(struct sockaddr);

int main(int argc, char *argv[])
{
	struct sockaddr_in listen_addr;
	struct plxr_connection conn = {0};
	int listen_fd;
	int port = 8080;

	parse_args(argc, argv);

	if (opts.help) {
		print_usage(argc > 0 ? argv[0] : "plexer");
		return 0;
	}

	if (opts.port) {
		port = atoi(opts.port);
	}

	/* ignore SIGPIPE */
	signal(SIGPIPE, sigpipe_handler);

	listen_fd = plxr_socket_listen(&listen_addr, port);
	if (listen_fd == -1) {
		perror("plxr_socket_listen");
		return 1;
	}

	if (opts.verbose)
		printf("[*] listening on port: %d\n", port);

	do {
		conn.fd = accept(listen_fd, &conn.addr, &socklen);

		if (conn.fd == -1) {
			perror("accept");
			continue;
		}

		if (opts.verbose)
			printf("[*] client connected %s\n", plxr_socket_ntop(&conn.addr));

		if (plxr_connection_read(&conn) == -1)
			if (opts.verbose)
				printf("[!] plxr_connection_read: failed\n");

		if (plxr_serve_file_or_dir(&conn) == -1)
			if (opts.verbose)
				printf("[!] plxr_serve_file_or_dir: failed\n");

		shutdown(conn.fd, SHUT_RDWR);
		close(conn.fd);
	}
	while (1);

	return 0;
}
