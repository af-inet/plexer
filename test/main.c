#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../http.h"

void test_http() {
	struct plxr_http_request req;
	char buf[4096] = {0};
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

int main(int argc, char *argv[]){
	
	printf("\n-- http --\n");
	test_http();

	return 0;
}
