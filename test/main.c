#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../http.h"

#define SIZE (256)

void test_http() {
	struct plxr_http_request req;
	char buf[SIZE] = {0};
	char *raw =
		"GET / HTTP/1.1\r\n"
		"Host: www.google.com\r\n"
		"User-Agent: curl/7.48.0\r\n"
		"Accept: */*\r\n"
		"\r\n"
	;

	memcpy(buf, raw, strlen(raw));

	CTEST_ASSERT(plxr_http_parse(&req, buf) == 1);

	CTEST_STREQ(req.method, "GET")
	CTEST_STREQ(req.uri, "/")
	CTEST_STREQ(req.version, "HTTP/1.1");

	CTEST_ASSERT(req.headers_len == 3);

	CTEST_STREQ(req.headers[0].key, "Host");
	CTEST_STREQ(req.headers[1].key, "User-Agent");
	CTEST_STREQ(req.headers[2].key, "Accept");
	CTEST_STREQ(req.headers[0].value, "www.google.com");
	CTEST_STREQ(req.headers[1].value, "curl/7.48.0");
	CTEST_STREQ(req.headers[2].value, "*/*");
}

void test_scheme() {
	char buf[SIZE] = {0};
	ssize_t res;

	bzero(buf, SIZE);

	res = plxr_parse_scheme(buf, SIZE, "http://");
	CTEST_ASSERT( res > 0 );
	CTEST_STREQ(buf, "http");

	bzero(buf, SIZE);

	plxr_parse_scheme(buf, SIZE, "https://");
	CTEST_STREQ(buf, "https");

	bzero(buf, SIZE);

	res = plxr_parse_scheme(buf, SIZE, "https//");
	CTEST_ASSERT(res == -1);
}

void test_path() {
	char buf[SIZE];
	const char *src_a = "http://www.google.com/?a=b";
	const char *src_b = "http://www.google.com";
	ssize_t res;

	bzero(buf, SIZE);

	res = plxr_parse_scheme(buf, SIZE, src_a);
	CTEST_ASSERT( res > 0 );
	CTEST_STREQ(buf, "http");
	res = plxr_parse_path(buf, SIZE, src_a+res);
	CTEST_ASSERT(res > 0);
	CTEST_STREQ(buf, "www.google.com/");

	bzero(buf, SIZE);

	res = plxr_parse_scheme(buf, SIZE, src_b);
	CTEST_ASSERT( res > 0 );
	CTEST_STREQ(buf, "http");
	res = plxr_parse_path(buf, SIZE, src_b+res);
	CTEST_ASSERT(res > 0);
	CTEST_STREQ(buf, "www.google.com");
}

int main(int argc, char *argv[]) {

	CTEST_SUB(test_http);

	CTEST_SUB(test_scheme);

	CTEST_SUB(test_path);

	CTEST_RETURN();
}
