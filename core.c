
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
		ERROR("fcntl");
		return;
	}

	plxr_pool_init(ctx->pool);
}

void plxr_run(struct plxr_ctx_t *ctx){
	size_t i, free_clients;
	struct plxr_conn_t *conn;
	int ret;

	ctx->running = 1;

	while( ctx->running ){
		free_clients = (CLIENT_MAX - ctx->pool->total_used);

		// accepting
		for(i = 0; i < free_clients; i++) {
			conn = plxr_conn_alloc(ctx->pool);

			if ( conn == NULL ) {
				break;
			}

			switch( (ret=poll(&ctx->pfd, 1, SERVER_TIMEOUT)) ) {
			case -1:
				break;
			case  0:
				break;
			default:
				plxr_conn_accept(conn, ctx->fd);
				break;
			}

			if ( !conn->active )
				plxr_conn_free(ctx->pool, conn);

			if ( ret == 0 )
				break;
		}

		// if no connections, do a blocking accept
		if( ctx->pool->total_used == 0 ) {
			conn = plxr_conn_alloc(ctx->pool);
			if ( conn )
			{
				if ( plxr_conn_accept(conn, ctx->fd) == -1 ) {
					plxr_conn_free(ctx->pool, conn);
				}
			}
			continue;
		}
		
		printf("ACTIVE CONNECTIONS [%d]\n", ctx->pool->total_used);

		// handling
		switch( poll(ctx->pool->pfd_list, CLIENT_MAX, CLIENT_TIMEOUT) ){ 
		case -1:
			ERROR("poll error");
			break;
		case  0: 
			WARNING("poll timeout");
			break;
		default:
			for (i=0; i<CLIENT_MAX; i++) {
				if ( ctx->pool->conn_list[i].active ) {
					ctx->event_callback(ctx, &ctx->pool->conn_list[i]);
				}
			}
		}
	}
}

