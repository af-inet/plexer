#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "socket.h"
#include "server.h"

void request(server_t *server, client_t *client){
	char buf[4096] = {0};
	ssize_t result;

	result = server_read(server, client, buf, 4095);

	switch(result){
	case -1:
		perror("read");
		server_client_inactive(server, client);
		break;
	case 0:
		printf("closing connection\n");
		//shutdown(client->fd, SHUT_RDWR);
		//close(client->fd);
		//server_client_inactive(server, client);
		break;
	default:
		printf("read: %lu\n", client->bytes_read);
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

	shutdown(client->fd, SHUT_RDWR);
	close(client->fd);
	server_client_inactive(server, client);
}

int main(int argc, char *argv[]){
	server_t server;

	server_register_write(&response);
	server_register_read(&request);
	server_run(&server);

	return 0;
}
