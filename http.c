#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

/* Maps every possible hex character value to its numerical value or -1 if invalid.
 * ex. 'F' -> 15
 */
static const int hex_conversion_table[256] = {
   [0 ... 255] = -1,
   ['0'] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
   ['A'] = 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
   ['a'] = 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
};

/* Converts a 2 digit hex string at `src` to a numerical value at `dest`.
 * ex. "A0" -> 160
 *
 * on success `dest` is set to the result of the conversion, and 0 is returned.
 * on error, -1 is returned.
 */
int plxr_unescape_hex(unsigned char *dest, unsigned char *src) {
    int a, b;
    a = hex_conversion_table[src[0]];
    b = hex_conversion_table[src[1]];
    if ( (a == -1) || (b == -1) ) {
        return -1;
    }
    *dest = ((a & 0xFF) << 4) + (b & 0xFF);
    return 0;
}

ssize_t plxr_unescape_url(char *dest, size_t count, const char *src) {
    unsigned char byte;
    char *limit, *start;
    start = dest;
    limit = dest + count;

    while ( (byte = *src) != '\0' ) {
        if ( byte == '%' ) {
            if( plxr_unescape_hex(&byte, (unsigned char *)src+1) == -1 ) {
                return -1;
            }
            src += 2;
        }
        if ( dest < limit ) {
            *(dest++) = byte;
        }
        src += 1;
    }

    return dest-start;
}

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
    return (*src) == '\0';
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
        "{%s : %s : %s}\n",
        req->line.method,
        req->line.uri,
        req->line.version
    );

    for(i=0; i < req->headers_len; i++)
        printf(
            "[%s: %s]\n",
            req->headers[i].key,
            req->headers[i].value
        );
     
}
