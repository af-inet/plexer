#ifndef PLXR_HTTP_H
#define PLXR_HTTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HTTP_MAX_HEADERS (40)

struct plxr_http_header {
    char *key;
    char *value;
};

struct plxr_http_request {
	char *method;
    char *uri;
    char *version;
    struct plxr_http_header headers[HTTP_MAX_HEADERS];
    size_t headers_len;
};

int plxr_http_response(
    char *dest, size_t size,
    int status_code,
    int content_length
);

/* Parses a *mutable* string buffer `src` 
 * into a struct plxr_http_request `dest`
 *
 * returns
 *  1 on success
 *  0 on failure
 */
int plxr_http_parse(struct plxr_http_request *dest, char *src);

/* printf()'s a request and its headers.
 */
void plxr_http_print(struct plxr_http_request *req);

#endif /* PLXR_HTTP_H */
