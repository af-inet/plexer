#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "file.h"
#include "http.h"
#include "socket.h"

const char *html_404 =
"<html><head></head><body><h1>file not found</h1></body></html>";
const char *html_500 =
"<html><head></head><body><h1>internal server error</h1></body></html>";

int serve_data(int sockfd, int status_code, const char *data, size_t data_len)
{
	char headers[4096];
	size_t headers_len;

	headers_len = plxr_http_response(
		headers, sizeof(headers), status_code, data_len);

	if (headers_len > sizeof(headers))
		return -1; /* not enough memory */
	if (write(sockfd, headers, headers_len) != headers_len)
		return -1; /* didn't write expected length */
	if (write(sockfd, data, data_len) != data_len)
		return -1; /* didn't write expected length */

	return 0;
}

int serve_file(int sockfd, int status_code, char *filename)
{
	char *data;
	off_t data_len;

	data = plxr_alloc_file(filename, &data_len);

	if (data == NULL)
		return -1;

	return serve_data(sockfd, status_code, data, data_len);
}

int serve_string(int sockfd, int status_code, const char *str)
{
	return serve_data(sockfd, status_code, str, strlen(str));
}

int serve_404(int sockfd)
{
	return serve_string(sockfd, 404, html_404);
}

int serve_500(int sockfd)
{
	return serve_string(sockfd, 500, html_500);
}

int render_dir_html(char *dest, size_t size, char *dirname, DIR *dir)
{
	const char *page_template =
		"<!doctype html>"
		"<html><head></head>"
		"<body><ul>%s</ul></body></html>"
	;

	const char *li_template =
		"<li><a href=\"%s/%s\">%s</a></li>"
	;

	struct dirent *ent;
	char li_buffer[4096] = {0};
	size_t count = 0;

	while (( ent = readdir(dir) ))
	{
		/* exclude "." and ".." */
		if (strcmp(ent->d_name, ".") == 0)
			continue;
		if (strcmp(ent->d_name, "..") == 0)
			continue;

		count += snprintf(
			&li_buffer[count],
			sizeof(li_buffer) - count,
			li_template,
			dirname,
			ent->d_name,
			ent->d_name);

		if (count > sizeof(li_buffer))
			return -1; /* not enough memory */
	}

	return snprintf(dest, size, page_template, li_buffer);
}

int serve_dir(int sockfd, char *dirname, DIR *dir)
{
	char headers[4096];
	size_t headers_len;
	char data[4096];
	size_t data_len;

	data_len = render_dir_html(data, sizeof(data), dirname, dir);

	if (data_len == -1)
		return -1; /* not enough memory */
	if (data_len > sizeof(data))
		return -1; /* not enough memory */

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

enum {
	FILE_ERR = -1,
	FILE_REG = 1,
	FILE_DIR = 2,
	FILE_NOT_FOUND = 3
};

int check_dir(char *dirname, DIR *dir, char *filename)
{
	struct stat info;

	switch (plxr_in_dir_recursive(dirname, filename))
	{
	case 0:
		if (stat(filename, &info) != 0)
			return FILE_ERR;

		if (S_ISREG(info.st_mode))
			return FILE_REG;

		if (S_ISDIR(info.st_mode))
			return FILE_DIR;

		break;

	case 1:
		return FILE_NOT_FOUND;

	case -1:
	default:
		return FILE_ERR;
	}

	return FILE_ERR;
}

int router(int sockfd, char *filename)
{
	DIR *dir;
	int ret;
	int dir_type;

	/* serve the current directory */
	if (strcmp("/", filename) == 0)
	{
		dir = opendir(".");
		ret = serve_dir(sockfd, "", dir);
		closedir(dir);
		return ret;
	}

	/* get rid of the leading '/' */
	if (strlen(filename) > 1)
	{
		filename = filename + 1;
	}

	/* (recursively) check the current directory for this file */
	dir = opendir(".");
	dir_type = check_dir(".", dir, filename);
	closedir(dir);

	switch (dir_type)
	{
	case FILE_REG:
		return serve_file(sockfd, 200, filename);

	case FILE_DIR:

		dir = opendir(filename);
		ret = serve_dir(sockfd, filename, dir);
		closedir(dir);
		return ret;

	case FILE_NOT_FOUND:
		return serve_404(sockfd);

	case FILE_ERR:
		return serve_500(sockfd);

	default:
		return serve_500(sockfd);
	}
	return serve_500(sockfd);
}

int handle_client(int client_fd, struct sockaddr *client_addr)
{
	struct plxr_http_request req;
	char req_buffer[4096];
	ssize_t req_len;

	printf("[*] client connected %s\n",
		plxr_socket_ntop(client_addr));

	req_len = read(client_fd, req_buffer, sizeof(req_buffer));

	if (req_len == -1)
	{
		perror("read");
		return -1; /* read failed */
	}

	if (plxr_http_parse(&req, req_buffer) == 0)
	{
		printf("plxr_http_parse: failed\n");
		return -1;
	}

	if (router(client_fd, req.uri) == -1)
	{
		printf("router: failed\n");
		return -1;
	}

	return 0;
}

struct option_struct {
	char *port;
	int help;
};

void parse_args(int argc, char *argv[], struct option_struct *opts)
{
	int bflag, ch;

	bflag = 0;

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

int main(int argc, char *argv[])
{
	socklen_t socklen = sizeof(struct sockaddr);
	struct sockaddr_in listen_addr;
	struct sockaddr client_addr;
	struct option_struct opts = {0};
	int listen_fd;
	int client_fd;
	int port = 8080;

	parse_args(argc, argv, &opts);

	if (opts.help)
	{
		print_usage(argc > 0 ? argv[0] : "plexer");
		return 0;
	}

	if (opts.port)
	{
		port = atoi(opts.port);
	}

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
		shutdown(client_fd, SHUT_RDWR);
		close(client_fd);
	}
	while (1);

	return 0;
}
