#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

char *plxr_http_term_token(char *src, char sep) {
    while( (*src) != '\0' ) {
        if( (*src) == sep ) {
            *src = '\0';
            return src + 1;
        } else {
            src += 1;
        }
    }
    return NULL;
}

char *plxr_http_expect_token(char *src, char sep) {
    while( src[0] != '\0' )
        if( src[0] == sep )
            return src + 1;
        else
            src += 1;

    return NULL;
}

#define SPACE (' ')
#define CR    ('\r')
#define LF    ('\n')
#define COLON (':')

#define TERM_TOKEN(sep) \
if( (src = plxr_http_term_token(src, sep)) == NULL ) { return 0; } \

#define EXPECT_TOKEN(sep) \
if( (src = plxr_http_expect_token(src, sep)) == NULL ) { return 0; } \

// returns 1 or 0 on success or failure respectively.
int plxr_http_parse_line(struct http_line *dest, char *src, char **end) {

    dest->method = src;

    TERM_TOKEN( SPACE );

    dest->uri = src;

    TERM_TOKEN( SPACE );

    dest->version = src;

    TERM_TOKEN( CR );
    TERM_TOKEN( LF );

    *end = src;

    return 1;
}

int plxr_http_parse_header(struct http_header *dest, char *src, char **end) {

    if( ((*src) == CR) || ((*src) == '\0' ) ) {
        return 0;
    }

    dest->key = src;

    TERM_TOKEN( COLON );
    TERM_TOKEN( SPACE );

    dest->value = src;

    TERM_TOKEN( CR );
    TERM_TOKEN( LF );

    *end = src;

    return 1;
}

int plxr_http_parse_end(char *src) {
    EXPECT_TOKEN(CR);
    EXPECT_TOKEN(LF);
    return 1;
}

int plxr_http_parse_request(struct http_request *dest, char *src) {
    size_t i;
    i = 0;

    if( plxr_http_parse_line(&dest->line, src, &src) == 0 )
        return 0;

    while( plxr_http_parse_header(&dest->headers[i], src, &src) )
        if( (i += 1) > HTTP_MAX_HEADERS )
            return 0;

    dest->headers_len = i;

    return plxr_http_parse_end(src);
}

void plxr_http_print(struct http_request *req) {
    size_t i;
    
    printf(
        "%s %s %s\n",
        req->line.method,
        req->line.uri,
        req->line.version
    );

    for(i=0; i < req->headers_len; i++)
        printf(
            "%s: %s\n",
            req->headers[i].key,
            req->headers[i].value
        );
	printf("\n");
     
}
