#include "server.h"
#include "socket.h"

#define SERVER_TIMEOUT (10)
#define CLIENT_TIMEOUT (10)

static void (*SERVER_READ) (server_t *, client_t *);
static void (*SERVER_WRITE) (server_t *, client_t *);
socklen_t sockaddr_len = sizeof(struct sockaddr);

void error_fatal(char *reason){
	perror(reason);
	exit(EXIT_FAILURE);
}

void error_warning(char *reason){
	perror(reason);
}

void server_init(server_t *server){
	bzero(server, sizeof(server_t));

	if( (server->fd = socket_tcp_listen((struct sockaddr_in *)&server->addr, 8080))
		== SOCKET_ERROR )
	{
		error_fatal("server_init:socket_tcp_listen");
	}

	server->pfd.events = POLLIN;
	server->pfd.fd = server->fd;
}

client_t *server_next_client(server_t *server){
	size_t i;

	for(i = 0; i < CLIENT_MAX; i++) {
		if(server->clients[i].status == CLIENT_INACTIVE) {
			return &server->clients[i];
		}
	}
	
	return NULL;
}

void server_client_active(server_t *server, client_t *client){
	client->status = CLIENT_ACTIVE;
	server->active_clients++;
}

void server_client_inactive(server_t *server, client_t *client){
	bzero(client, sizeof(client_t));
	server->active_clients--;
}

void server_accept_blocking(server_t *server){
	client_t *client;
	
	client = server_next_client(server);
	client->fd = accept(server->fd, &client->addr, &sockaddr_len);
	
	if( client->fd == SOCKET_ERROR ) {
		error_warning("server_accept_blocking:accept");
	}else{
		server_client_active(server, client);
	}
}

int server_accept_nonblocking(server_t *server){
	int result;

	switch( result = poll(&server->pfd, 1, SERVER_TIMEOUT) ){
	case -1:
		error_warning("server_accept_nonblocking:poll");
		break;
	case  0:
		// timeout
		WARNING("server poll timeout");
		break;
	default:
		if(server->pfd.revents & POLLIN) {
			server_accept_blocking(server);
		}else{
			WARNING("server poll revents");
		}
	}
	
	return result;
}

void server_handle_error(server_t *server, client_t *client){
	WARNING("client error");
	server_client_inactive(server, client);
}

void server_handle_closed(server_t *server, client_t *client){
	WARNING("client closed");
	server_client_inactive(server, client);
}

void server_handle_invalid(server_t *server, client_t *client){
	WARNING("client invalid");
	server_client_inactive(server, client);
}

void server_handle_read(server_t *server, client_t *client){
	SERVER_READ(server, client);
}

void server_handle_write(server_t *server, client_t *client){
	SERVER_WRITE(server, client);
}

void server_handle_client(server_t *server, client_t *client){
	int events;
	client->pfd.events = POLLIN | POLLOUT;
	client->pfd.fd = client->fd;

	switch( poll(&client->pfd, 1, CLIENT_TIMEOUT) ) {
	case -1:
		error_warning("server_handle_client:poll");
		break;
	case  0:
		WARNING("client poll timeout");
		break;
	default:
		events = client->pfd.revents;
		if(events & POLLERR)
			return server_handle_error(server, client);
		if(events & POLLHUP)
			return server_handle_closed(server, client);
		if(events & POLLNVAL)
			return server_handle_invalid(server, client); 
		if(events & POLLOUT)
			if(client->status == CLIENT_ACTIVE)
				server_handle_write(server, client);
		if(events & POLLIN)
			if(client->status == CLIENT_ACTIVE)
				server_handle_read(server, client);
		break;
	}
}

ssize_t server_read(server_t *server, client_t *client, char *buf, size_t size){
	ssize_t result;
	result = read(client->fd, buf, size);

	if( result > 0 ) {
		client->bytes_read += result;
	}
	
	return result;
}

ssize_t server_write(server_t *server, client_t *client, char *buf, size_t size){
	ssize_t result;
	result = write(client->fd, buf, size);

	if( result > 0 ) {
		client->bytes_wrote += result;
	}
	
	return result;
}

void server_register_read( void (*callback)(server_t *, client_t *) ){
	SERVER_READ = callback;
}

void server_register_write( void (*callback)(server_t *, client_t *) ){
	SERVER_WRITE = callback;
}

void server_run(server_t *server){
	size_t i;
	server_init(server);
	while(1){
		server_accept_nonblocking(server);
		if( server->active_clients ) {
			for(i=0; i<32; i++){
				if( !(server_accept_nonblocking(server) > 0) ) {
					break;
				}
			}
		}else{
			printf("accept blocking\n");
			server_accept_blocking(server);
		}
		printf("active clients: %lu\n", server->active_clients);
		for(i = 0; i < CLIENT_MAX; i++){
			if(server->clients[i].status == CLIENT_ACTIVE) {
				server_handle_client(server, &server->clients[i]);
			}
		}
	}
}
