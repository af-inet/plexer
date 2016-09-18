#ifndef PLXR_CORE_H
#define PLXR_CORE_H

#include <stdlib.h>
#include <socket.h>

#include "settings.h"
#include "socket.h"
#include "connection.h"
#include "pool.h"

/* The plxr context, think of one `struct plxr_ctx_t` as one server.
 */
struct plxr_ctx_t {

	/* listener */
	int fd;
	struct pollfd pfd;
	struct sockaddr addr;

	/* connection pool */
	struct plxr_pool_t pool[1];

	/* boolean, is the server running? */
	int running;

	/* event handler */
	void (*event_callback) (struct plxr_ctx_t *ctx, struct plxr_conn_t *);

};

/* Configures plxr to begin accepting connections.
 */
void plxr_init(struct plxr_ctx_t *ctx);

/* Executes the core logic, event_callback should be set up
 * before this is called, or connections will be accepted
 * and never handled.
 */
void plxr_run(struct plxr_ctx_t *ctx);

#endif /* PLXR_CORE_H */
