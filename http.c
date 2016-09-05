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

const char *plxr_http_phrase(int status_code) {
	switch(status_code){
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Time-out";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request-URI Too Large";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested range not satisfiable";
	case 417: return "Expectation Failed";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Time-out";
	case 505: return "HTTP Version not supported";
	default: return "Invalid Status Code";
	}
}

/* === RESPONSE PARSING === */

/* RFC 2616 Section 6.1
 * http://www.rfc-base.org/txt/rfc-2616.txt
 * Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
 */
const char *HTTP_RESP_FORMAT =
	"HTTP/1.1 %d %s\r\n"
	"Content-Length: %d\r\n"
	"\r\n"
;

int plxr_http_response(
	char *dest, size_t size,
	int status_code,
	int content_length
){
	return snprintf(
		dest,
		size,
		HTTP_RESP_FORMAT,
		status_code,
		plxr_http_phrase(status_code),
		content_length
	);
}

/* === REQUEST PARSING === */

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
#define CR	('\r')
#define LF	('\n')
#define COLON (':')

#define TERM_TOKEN(sep) \
if( (src = plxr_http_term_token(src, sep)) == NULL ) { return 0; } \

#define EXPECT_TOKEN(sep) \
if( (src = plxr_http_expect_token(src, sep)) == NULL ) { return 0; } \

// returns 1 or 0 on success or failure respectively.
int plxr_http_parse_line(struct plxr_http_request *dest, char *src, char **end) {

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

int plxr_http_parse_header(struct plxr_http_header *dest, char *src, char **end) {

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

int plxr_http_parse(struct plxr_http_request *dest, char *src) {
	size_t i;
	i = 0;

	if( plxr_http_parse_line(dest, src, &src) == 0 )
		return 0;

	while( plxr_http_parse_header(&dest->headers[i], src, &src) )
		if( (i += 1) > HTTP_MAX_HEADERS )
			return 0;

	dest->headers_len = i;

	return plxr_http_parse_end(src);
}

void plxr_http_print(struct plxr_http_request *req) {
	size_t i;
	
	printf(
		"%s %s %s\n",
		req->method,
		req->uri,
		req->version
	);

	for(i=0; i < req->headers_len; i++)
		printf(
			"%s: %s\n",
			req->headers[i].key,
			req->headers[i].value
		);
	printf("\n");
	 
}
