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
	int ret;

	data = plxr_alloc_file(filename, &data_len); /* dynamically allocated */
	if (data == NULL)
		return PLX_ALLOC_FILE_FAILED;

	ret = plxr_serve_data(conn, status_code, data, data_len);

	free(data); /* don't leak memory */

	return ret;
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
plxr_render_dir_html(char *dest, size_t size, char *path, DIR *dir, char *host)
{
	const char *page_template =
		"<!doctype html>"
		"<html><head></head>"
		"<body><ul>%s</ul></body></html>"
	;

	const char *li_template =
		"<li><a href=\"http://%s/%s\">%s</a></li>"
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
			host,
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
	char *host;

	host = plxr_http_header(&conn->request, "Host");
	if (host == NULL)
		return -1; /* missing `Host` header, should 400 */

	data_len = plxr_render_dir_html(data, sizeof(data), path, dir, host);

	if (data_len < 0)
		return data_len;
	if (data_len > sizeof(data))
		return PLX_OUT_OF_MEMORY;

	return plxr_serve_data(conn, 200, data, data_len);
}

char *
uri_normalize(char *uri)
{
	char *start, *ptr;
	start = uri;

	/* cut preceding slashes */
	while (*uri == '/') {
		uri += 1;
	}

	/* skip to the end */
	ptr = uri;
	while (*ptr != '\0') {
		ptr += 1;
	}

	/* sweep up the trailing slashes */
	while (--ptr > start) {
		if (*ptr == '/') {
			*ptr = '\0';
		} else {
			break;
		}
	}

	return uri;
}

char *
plxr_append_strings(char *a, char *b)
{
	static char buf[512];
	bzero(buf, sizeof(buf));

	if (snprintf(buf, sizeof(buf), "%s%s", a, b) > sizeof(buf))
		return NULL; /* out of memory */

	return buf;
}

/* Takes a URI and figures out a filepath
 * to serve, or NULL for 404.
 */
char *
plxr_router(int *filetype, DIR *dir, char *uri)
{
	char *filename;

	if (uri == NULL)
		return NULL;
	if (uri[0] == '\0')
		return NULL; /* empty string */

	/* special case for root */
	if (strcmp(uri, "/") == 0) {
		if (plxr_check_dir(dir, "index.html") == PLX_FILE_REG) {
			*filetype = PLX_FILE_REG;
			return "index.html"; /* serve index.html */
		} else {
			*filetype = PLX_FILE_DIR;
			return "."; /* serve the current directory */
		}
	}

	/* clean up preceding and trailing slashes */
	uri = uri_normalize(uri);
	if (uri == NULL)
		return NULL;

	*filetype = plxr_check_dir(dir, uri);
	switch (*filetype)
	{
	case PLX_FILE_REG:
		return uri;
	case PLX_FILE_DIR:
		/* if it's a directory, check for an index.html */
		filename = plxr_append_strings(uri, "/index.html");
		if (filename &&
			(plxr_check_dir(dir, filename) == PLX_FILE_REG)) {
			*filetype = PLX_FILE_REG;
			return filename; /* found an index.html in this folder */
		}
		return uri;
	case PLX_FILE_ERR:
	case PLX_FILE_NOT_FOUND:
		break;
	}

	return NULL; /* TODO: we're losing error information here (PLX_FILE_ERR) */
}

/* TODO this is the messiest function, would be nice to clean it up
 * with some better url parsing.
 */
int
plxr_serve_file_or_dir(struct plxr_connection *conn)
{
	DIR *dir;
	char *path;
	char buf[256] = {0};
	int ret;
	int filetype;

	/* copy the uri into a buffer */
	strncpy(buf, conn->request.uri, sizeof(buf));

	if (( dir = opendir(".") )) {
		path = plxr_router(&filetype, dir, buf);
		closedir(dir);
	} else {
		return -1;
	}
	/* if no file was found, 404 */
	if (path == NULL) {
		return plxr_serve_string(conn, 404, plxr_html_404);
	}

	switch (filetype)
	{
	case PLX_FILE_REG:
		return plxr_serve_file(conn, 200, path);
	case PLX_FILE_DIR:
		/* if its a directory, open it up and render it into an html page */
		if (( dir = opendir(path) )) {
			ret = plxr_serve_dir(conn, path, dir);
			closedir(dir);
		} else {
			return -1;
		}
		return ret;
	case PLX_FILE_NOT_FOUND:
		return plxr_serve_string(conn, 404, plxr_html_404);
	case PLX_FILE_ERR:
		return plxr_serve_string(conn, 500, plxr_html_500);
	default:
		break;
	}
	return plxr_serve_string(conn, 500, plxr_html_500);
}

