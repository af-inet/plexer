#ifndef PLXR_CORE_H
#define PLXR_CORE_H

#include <stdlib.h>
#include <socket.h>

#include "settings.h"
#include "socket.h"
#include "connection.h"
#include "pool.h"

struct plxr_ctx_t {

	/* listener */
	int fd;
	struct pollfd pfd;
	struct sockaddr addr;

	/* client pool */
	struct plxr_pool_t pool[1];

	/* boolean, is the server running? */
	int running;

	/* event handler */
	void (*event_callback) (struct plxr_ctx_t *ctx, struct plxr_conn_t *);

};

void plxr_init(struct plxr_ctx_t *ctx);

void plxr_run(struct plxr_ctx_t *ctx);

#endif /* PLXR_CORE_H */
