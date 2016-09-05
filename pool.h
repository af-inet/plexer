#ifndef PLXR_POOL_H
#define PLXR_POOL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "settings.h"
#include "log.h"
#include "connection.h"

/* A static pool of connection objects.
 */
struct plxr_pool_t {

	/* We keep all our `pfd`s in one place for quick polling. */
	struct pollfd pfd_list[CLIENT_MAX];

	/* array of connections */
	struct plxr_conn_t conn_list[CLIENT_MAX];

	/* total number of used connections */
	size_t total_used;

	/* used as the root node of a linked list
 	 * for keeping track of free connections
 	 */
	struct plxr_conn_t free_list[1];

};

/* returns
 * 	on success: a pointer to next available object
 * 	on error: NULL
 */
struct plxr_conn_t *plxr_conn_alloc(struct plxr_pool_t *pool);

void plxr_conn_free(struct plxr_pool_t *pool, struct plxr_conn_t *conn);

void plxr_pool_init(struct plxr_pool_t *pool);

#endif /* PLXR_POOL_H */
