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
		perror("read");
		//server_client_inactive(server, client);
		break;
	case 0:
		printf("closing connection\n");
		//shutdown(client->fd, SHUT_RDWR);
		//close(client->fd);
	    server_client_inactive(server, client);
		break;
	default:
		printf("read: %lu\n", client->bytes_read);

        if( plxr_http_parse_request(&req, buf) )
            plxr_http_print(&req);
        else
            printf("parse bad\n");

		break;	
	}
}

void response(server_t *server, client_t *client){
	const char *resp = 
		"HTTP/1.1 200 OK\n"
		"Content-Length: 14\n\n"
		"<h1>test</h1>\n"
	;

	if(client->bytes_wrote == 0){
		server_write(server, client, resp, strlen(resp));
		//printf("wrote: %lu/%lu\n", client->bytes_wrote, strlen(resp));
	}

	//server_client_shutdown(server, client);
}

int main(int argc, char *argv[]){
	server_t server;
	
	server_init(&server);
	server_register_write(&server, &response);
	server_register_read(&server, &request);
	server_run(&server);

	return 0;
}
