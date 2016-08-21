#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "socket.h"
#include "server.h"
#include "http.h"

void request(server_t *server, client_t *client){
	char buf[4096] = {0};
	ssize_t result;
    struct http_request req;

	result = server_read(server, client, buf, 4095);

	switch(result){
	case -1:
		/* read error */
		break;
	case 0:
		/* read nothing */
		server_client_shutdown(server, client);
		break;
	default:

        if( plxr_http_parse_request(&req, buf) )
			return;
            //plxr_http_print(&req);
        else
            printf("parse bad\n");

		break;	
	}
}

void response(server_t *server, client_t *client){
	const char *resp = 
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 14\r\n"
		"\r\n"
		"<h1>test</h1>\n"
	;

	if(
		(client->bytes_wrote == 0) &&
		(client->bytes_read > 0)
	){
		server_write(server, client, resp, strlen(resp));
		server_client_shutdown(server, client);
		//printf("wrote: %lu/%lu\n", client->bytes_wrote, strlen(resp));
	}
}

int main(int argc, char *argv[]){
	server_t server;
	
	server_init(&server);
	server.event_read = &request;
	server.event_write = &response;
	server_run(&server);

	return 0;
}
