
#include "core.h"

void plxr_init(struct plxr_ctx_t *ctx) {

	ctx->fd = socket_tcp_listen((struct sockaddr_in *)&ctx->addr, SERVER_PORT);

	if( ctx->fd == SOCKET_ERROR )
	{
		FATAL("listen");
	}

	ctx->pfd.fd = ctx->fd;
	ctx->pfd.events = POLLIN;

	if( fcntl(ctx->fd, F_SETFD, O_NONBLOCK) == -1 ) {
		FATAL("fcntl");
		return;
	}

	plxr_pool_init(ctx->pool);
}

void plxr_accept(struct plxr_ctx_t *ctx) {
	struct plxr_conn_t *conn;
	int res;

	conn = plxr_conn_alloc(ctx->pool);
	if ( conn == NULL ) {
		return;
	}

	res = plxr_conn_accept(conn, ctx->fd); 

	if ( res == -1 ) {
		plxr_conn_free(ctx->pool, conn);
		return;
	}
}

void plxr_nonblock_accept(struct plxr_ctx_t *ctx) {
	switch( poll(&ctx->pfd, 1, SERVER_TIMEOUT) ) {
	case -1:
		/* there was an error polling */
		ERROR("poll");
		break;
	case  0:
		/* no connections were accepted (timeout) */
		break;
	default:
		/* connections are ready  to be accepted. */
		plxr_accept(ctx);
		break;
	}
}

void plxr_run(struct plxr_ctx_t *ctx){
	size_t i;
	struct plxr_conn_t *conn;
	int res;

	ctx->running = 1;

	while( ctx->running ){

		plxr_nonblock_accept(ctx);

		/* If there are no connections, do a blocking accept. */
		if( ctx->pool->total_used == 0 )
			plxr_accept(ctx);

		/* poll every connection */
		res = poll(ctx->pool->pfd_list, CLIENT_MAX, CLIENT_TIMEOUT); 

		if ( res == -1 ) {
			ERROR("poll");
			continue;
		}
		
		/* If there are no connections, go back to accepting. */
		if ( ctx->pool->total_used == 0 ) {
			continue;
		}

		for (i=0; i<CLIENT_MAX; i++) {
			conn = & ctx->pool->conn_list[i];
			if ( conn->used ) {
				ctx->event_callback(ctx, conn);
			}
		}
	}
}

