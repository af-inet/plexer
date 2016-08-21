#include "server.h"
#include "socket.h"
#include "settings.h"

socklen_t sockaddr_len = sizeof(struct sockaddr);

void server_listen(server_t *server){

	server->fd =
		socket_tcp_listen((struct sockaddr_in *)&server->addr, 8080);

	if( server->fd == SOCKET_ERROR )
	{
		FATAL("listen");
	}
}

void server_init(server_t *server){
	bzero(server, sizeof(server_t));

	server_listen(server);

	server->pfd.events = POLLIN;
	server->pfd.fd = server->fd;

	if( fcntl(server->fd, F_SETFD, O_NONBLOCK) == -1 ) {
		ERROR("fcntl");
		return;
	}
}

client_t *server_next_client(server_t *server){
	size_t i;

	for(i = 0; i < CLIENT_MAX; i++) {
		if(server->clients[i].status == CLIENT_INACTIVE) {
			server->clients[i].pfd = &server->client_pfds[i];
			return &server->clients[i];
		}
	}
	
	return NULL;
}

void server_client_shutdown(server_t *server, client_t *client) {
	if ( client->status == CLIENT_INACTIVE ) {
		WARNING("attempted to shutdown inactive client");
		return;
	}

	shutdown(client->fd, SHUT_RDWR);
	close(client->fd);

	bzero(client, sizeof(client_t));
	client->status = CLIENT_INACTIVE;

	server->active_clients--;
}

void server_accept_blocking(server_t *server){
	client_t *client;
	
	client = server_next_client(server);
	
	if ( client == NULL ) {
		WARNING("client pool full!");
		return;
	}

	client->fd = accept(server->fd, &client->addr, &sockaddr_len);
	
	if( client->fd == SOCKET_ERROR ) {
		ERROR("accept");
		return;
	}
	
	if( fcntl(client->fd, F_SETFD, O_NONBLOCK) == -1 ) {
		ERROR("fcntl");
		return;
	}

	client->pfd->fd = client->fd;
	client->pfd->events = POLLIN | POLLOUT;

	client->status = CLIENT_ACTIVE;
	server->active_clients++;
}

int server_accept_nonblocking(server_t *server){
	int result;

	switch( result = poll(&server->pfd, 1, SERVER_TIMEOUT) ){
	case -1:
		ERROR("server poll");
		break;
	case  0:
		/* timeout */
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
	server_client_shutdown(server, client);
}

void server_handle_closed(server_t *server, client_t *client){
	WARNING("client closed");
	server_client_shutdown(server, client);
}

void server_handle_invalid(server_t *server, client_t *client){
	WARNING("client invalid");
	server_client_shutdown(server, client);
}

void server_handle_read(server_t *server, client_t *client){
	if(server->event_read) {
		server->event_read(server, client);
	}
}

void server_handle_write(server_t *server, client_t *client){
	if(server->event_write) {
		server->event_write(server, client);
	}
}

int server_poll_all_clients(server_t *server){
	return poll(server->client_pfds, CLIENT_MAX, CLIENT_TIMEOUT); 
}

void server_handle_client_events(server_t *server, client_t *client){
	int events;

	events = client->pfd->revents;

	client->attempts += 1;

	if( (events & POLLIN) && (client->status == CLIENT_ACTIVE) ) {
		server_handle_read(server, client);
	}

	if( (events & POLLOUT) && (client->status == CLIENT_ACTIVE) ) {
		server_handle_write(server, client);
	}

	if( (events & POLLERR) && (client->status == CLIENT_ACTIVE) ) {
		server_handle_error(server, client);
	}

	if( (events & POLLHUP) && (client->status == CLIENT_ACTIVE) ) {
		server_handle_closed(server, client);
	}

	if( (events & POLLNVAL) && (client->status == CLIENT_ACTIVE) ) {
		server_handle_invalid(server, client); 
	}

	if( client->attempts > CLIENT_MAX_ATTEMPTS ) {
		server_client_shutdown(server, client);
	}
}

void server_handle_client(server_t *server, client_t *client){
	switch( poll(client->pfd, 1, CLIENT_TIMEOUT) ) {
	case -1:
		ERROR("poll");
		break;
	case  0:
		WARNING("poll timeout");
		break;
	default:
		server_handle_client_events(server, client);
	}
}

void server_handle_all_clients(server_t *server){
	size_t i;
	for(i = 0; i < CLIENT_MAX; i++){
		if(server->clients[i].status == CLIENT_ACTIVE) {
			server_handle_client_events(server, &server->clients[i]);
		}
	}
}

ssize_t server_read(server_t *server, client_t *client, char *buf, size_t size){
	ssize_t result;
	result = read(client->fd, buf, size);

	if( result == -1 ) {
		ERROR("read");
	}

	if( result > 0 ) {
		client->bytes_read += result;
	}
	
	return result;
}

ssize_t server_write(server_t *server, client_t *client, char *buf, size_t size){
	ssize_t result;
	result = write(client->fd, buf, size);

	if( result == -1 ) {
		ERROR("read");
	}

	if( result > 0 ) {
		client->bytes_wrote += result;
	}
	
	return result;
}

void server_run(server_t *server){
	size_t i, free_clients;

	server->running = 1;

	while( server->running ){
		free_clients = (CLIENT_MAX - server->active_clients);

		for(i = 0; i < free_clients; i++) {
			if( server_accept_nonblocking(server) <= 0 ) {
				break;
			}
		}

		if( server->active_clients == 0 ) {
			server_accept_blocking(server);
			continue;
		}

		switch( server_poll_all_clients(server) ){
		case -1:
			ERROR("server_run server_poll_clients");
			break;
		case  0: 
			WARNING("server_run poll timeout");
			break;
		default:
			server_handle_all_clients(server);
		}
	}
}

