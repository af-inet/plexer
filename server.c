#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "server.h"
#include "file.h"
#include "socket.h"
#include "http.h"
#include "connection.h"
#include "error.h"

static const char *plxr_html_404 =
"<html><head></head><body><h1>file not found</h1></body></html>";

static const char *plxr_html_500 =
"<html><head></head><body><h1>internal server error</h1></body></html>";

int
plxr_serve_data(
	struct plxr_connection *conn,
	int status_code,
	const char *data,
	size_t data_len)
{
	char headers[4096] = {0};
	size_t headers_len;
	int ret;

	headers_len = plxr_http_response(
		headers, sizeof(headers), status_code, "text/html", data_len);

	if (headers_len > sizeof(headers))
		return PLX_OUT_OF_MEMORY;

	ret = plxr_socket_write_timeout(conn->fd, headers, headers_len, 250);
	if (ret != headers_len)
		return ret;

	ret = plxr_socket_write_timeout(conn->fd, data, data_len, 250);
	if (ret != data_len)
		return ret;

	return 0;
}

int
plxr_serve_file(
	struct plxr_connection *conn,
	int status_code,
	char *filename)
{
	char *data;
	off_t data_len;

	data = plxr_alloc_file(filename, &data_len);
	if (data == NULL)
		return PLX_ALLOC_FILE_FAILED;

	return plxr_serve_data(conn, status_code, data, data_len);
}

int
plxr_serve_string(
	struct plxr_connection *conn,
	int status_code,
	const char *str)
{
	return plxr_serve_data(conn, status_code, str, strlen(str));
}

static char path_buffer[1024];
char *
compose_path(char *path, char *filename)
{
	bzero(path_buffer, sizeof(path_buffer));
	if (
		snprintf(
			path_buffer,
			sizeof(path_buffer),
			strlen(path) == 0 ?
				"%s%s" :
				"%s/%s",
			path,
			filename) > sizeof(path_buffer)
	)
		return NULL; /* not enough memory */
	else
		return path_buffer;
}

int
plxr_render_dir_html(char *dest, size_t size, char *path, DIR *dir)
{
	const char *page_template =
		"<!doctype html>"
		"<html><head></head>"
		"<body><ul>%s</ul></body></html>"
	;

	const char *li_template =
		"<li><a href=\"http://localhost:8080/%s\">%s</a></li>"
	;

	struct dirent *ent;
	char li_buffer[4096] = {0};
	size_t count = 0;

	while (( ent = readdir(dir) ))
	{
		count += snprintf(
			&li_buffer[count],
			sizeof(li_buffer) - count,
			li_template,
			compose_path(path, ent->d_name),
			ent->d_name);

		if (count > sizeof(li_buffer))
			return PLX_OUT_OF_MEMORY;
	}

	return snprintf(dest, size, page_template, li_buffer);
}

int
plxr_serve_dir(struct plxr_connection *conn, char *path, DIR *dir)
{
	char data[4096];
	int data_len;

	data_len = plxr_render_dir_html(data, sizeof(data), path, dir);

	if (data_len < 0)
		return data_len;
	if (data_len > sizeof(data))
		return PLX_OUT_OF_MEMORY;

	return plxr_serve_data(conn, 200, data, data_len);
}

/* TODO this is the messiest function, would be nice to clean it up
 * with some better url parsing.
 */
int
plxr_serve_file_or_dir(struct plxr_connection *conn)
{
	DIR *dir;
	int ret;
	int dir_type;
	size_t len;
	char path_buffer[256] = {0};
	char *path;
	char *ptr;

	ptr = stpncpy(path_buffer, conn->request.uri, sizeof(path_buffer));
	path = path_buffer;

	/* `ptr` points to the null terminator, lets roll that back */
	ptr -= 1;
	/* cut trailing slashes */
	for (; (ptr > path_buffer) && ((*ptr) == '/'); --ptr) {
		*ptr = '\0';
	}

	/* serve the current directory */
	if (strcmp("/", path) == 0)
	{
		dir = opendir(".");
		ret = plxr_serve_dir(conn, "", dir);
		closedir(dir);
		return ret;
	}

	/* get rid of the leading '/' (for comparison) */
	len = strlen(path) ;
	if (len > 0)
	{
		path = path + 1;
	}

	/* (recursively) check the current directory for this file */
	dir = opendir(".");
	dir_type = plxr_check_dir(".", dir, path);
	closedir(dir);

	switch (dir_type)
	{
	case PLX_FILE_REG:
		return plxr_serve_file(conn, 200, path);
	case PLX_FILE_DIR:
		dir = opendir(path);
		ret = plxr_serve_dir(conn, path, dir);
		closedir(dir);
		return ret;
	case PLX_FILE_NOT_FOUND:
		return plxr_serve_string(conn, 404, plxr_html_404);
	case PLX_FILE_ERR:
		return plxr_serve_string(conn, 500, plxr_html_500);
	default:
		return plxr_serve_string(conn, 500, plxr_html_500);
	}
	return plxr_serve_string(conn, 500, plxr_html_500);
}

