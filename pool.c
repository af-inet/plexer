
#include "pool.h"

void plxr_conn_push(struct plxr_conn_t *root, struct plxr_conn_t *conn) {
	conn->next = root->next;
	root->next = conn;
}

struct plxr_conn_t *plxr_conn_pop(struct plxr_conn_t *root) {
	struct plxr_conn_t *temp;
	if ( root->next == NULL ) {
		return NULL;
	}

	temp = root->next;
	root->next = root->next->next;

	return temp;
}

void plxr_pool_init(struct plxr_pool_t *pool) {
	size_t i;
	struct plxr_conn_t *temp;
	
	bzero(pool, sizeof(struct plxr_pool_t));

	for (i = 0; i < CLIENT_MAX; i++) {
		temp = & pool->conn_list[i];
		temp->pfd = & pool->pfd_list[i];
		plxr_conn_push(pool->free_list, temp);
	}
}

struct plxr_conn_t *plxr_conn_alloc(struct plxr_pool_t *pool) {
	struct plxr_conn_t *temp;

	temp = plxr_conn_pop(pool->free_list);
	if (temp == NULL) {
		WARNING("pool alloc failed, ran out of memory");
		return NULL;
	}

	pool->total_used += 1;
	temp->used = 1;

	return temp;
}

void plxr_conn_free(struct plxr_pool_t *pool, struct plxr_conn_t *conn) {
	if ( conn->used ) {
		conn->used = 0;
		pool->total_used -= 1;
		plxr_conn_push(pool->free_list, conn);
	} else {
		WARNING("pool attempted to free unused connection");
	}
}

