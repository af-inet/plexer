#ifndef PLXR_SERVER_H
#define PLXR_SERVER_H

#include "socket.h"
#include "settings.h"

#define CLIENT_ACTIVE   (1)
#define CLIENT_INACTIVE (0)

#define WARNING(msg) \
printf("WARNING: %s:%d %s", __FILE__, __LINE__, msg "\n") \

#define ERROR(msg) \
printf("ERROR: %s:%d %s", __FILE__, __LINE__, msg "\n"); \
perror(msg) \

#define FATAL(msg) \
printf("FATAL: %s:%d %s", __FILE__, __LINE__, msg "\n"); \
perror(msg); \
exit(EXIT_FAILURE) \

struct client_t {
	struct sockaddr addr;
	struct pollfd *pfd;
	int fd;
	int events;
	int status;
	int attempts;
	size_t bytes_wrote;
	size_t bytes_read;
};

struct server_t {
	struct pollfd client_pfds[CLIENT_MAX];
	struct pollfd pfd;
	struct client_t clients[CLIENT_MAX];
	struct sockaddr addr;
	size_t active_clients;
	int fd;
	int running;

	void (*event_read) (struct server_t *, struct client_t *);
	void (*event_write) (struct server_t *, struct client_t *);

};

typedef struct client_t client_t;
typedef struct server_t server_t;

void error_fatal(char *reason);

void error_warning(char *reason);

void server_init(server_t *server);

client_t *server_next_client(server_t *server);

void server_client_active(server_t *server, client_t *client);

void server_client_inactive(server_t *server, client_t *client);

void server_accept_blocking(server_t *server);

int server_accept_nonblocking(server_t *server);

void server_handle_error(server_t *server, client_t *client);

void server_handle_closed(server_t *server, client_t *client);

void server_handle_invalid(server_t *server, client_t *client);

void server_handle_read(server_t *server, client_t *client);

void server_handle_write(server_t *server, client_t *client);

void server_handle_client(server_t *server, client_t *client);

void server_register_read(server_t *server, void (*callback)(server_t *, client_t *) );

void server_register_write(server_t *server, void (*callback)(server_t *, client_t *) );

ssize_t server_write(server_t *server, client_t *client, char *buf, size_t size);

ssize_t server_read(server_t *server, client_t *client, char *buf, size_t size);

void server_client_shutdown(server_t *server, client_t *client);

void server_run();

#endif
