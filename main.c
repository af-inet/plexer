#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "socket.h"
#include "core.h"
#include "http.h"
#include "table.h"
#include "file.h"
#include "connection.h"

// Define a static table.
#define TABLE_SIZE (128)
static htable_t       g_table;
static htable_entry_t g_entries[TABLE_SIZE * TABLE_SIZE];

// Load a file into the table.
void table_insert(const char *filename) {
	char *buf;
	ssize_t size;
	if ( (buf = plxr_alloc_file(filename, &size)) )
	{
		filename += 2; // cut off "./"
		printf("loaded [%s]\n", filename);
		if ( htable_sets(&g_table, filename, buf) == 0 ) {
			printf("htable_failed\n");
			exit(EXIT_FAILURE);
		}
	}
}

#define READ_ERROR (-1)
#define READ_ZERO   (0)

void request(struct plxr_ctx_t *ctx, struct plxr_conn_t *conn){
	char buf[4096] = {0};
	ssize_t ret;
    struct plxr_http_request req;

	switch( (ret=read(conn->fd, buf, 4095)) ){

	case READ_ERROR:
		/* fallthrough */
	case READ_ZERO:
		plxr_conn_close(conn);
		plxr_conn_free(ctx->pool, conn);
		break;

	default:
        if( plxr_http_parse(&req, buf) ) {
			if ( strlen(req.uri) > 0 ) {
				printf("Requested: [%s]\n", req.uri+1); // cut off "/"
				conn->data = htable_gets(&g_table, req.uri+1);
			}
        } else {
            printf("Invalid HTTP Request\n");
		}
		break;	
	}

	if (ret > 0)
		conn->bytes_read += ret;
}

const char *html_404 = 
	"<html>"
	"<body>"
	"<h1> 404 Not Found </h1>"
	"</body>"
	"</html>"
;

void response(struct plxr_ctx_t *ctx, struct plxr_conn_t *conn){
	size_t data_len, header_len;
	char header[4096];
	const char *data;

	// If we've already wrote or haven't read, don't bother responding.
	if( (conn->bytes_wrote != 0) || (conn->bytes_read == 0) ) {
		return;
	}

	if ( conn->data == NULL )
		data = html_404;
	else
		data = conn->data;

	data_len = strlen(data);
	header_len = plxr_http_response(header, sizeof(header), 404, data_len);

	write(conn->fd, header, header_len);
	write(conn->fd, data, data_len);

	plxr_conn_close(conn);
	plxr_conn_free(ctx->pool, conn);
}

void callback_handler(struct plxr_ctx_t *ctx, struct plxr_conn_t *conn) {
	if ( conn->pfd->revents & POLLIN ) {
		request(ctx, conn);
	}
	if ( conn->pfd->revents & POLLOUT ) {
		response(ctx, conn);
	}
}

int main(int argc, char *argv[]){
	struct plxr_ctx_t ctx;

	// Load the current directory into a hash table.
	g_table.size = TABLE_SIZE;
	g_table.entries = g_entries;
	bzero(g_entries, sizeof(g_entries));
	plxr_ntfw(&table_insert);

	// initialize the server
	plxr_init(&ctx);
	ctx.event_callback = &callback_handler;
	plxr_run(&ctx);

	return 0;
}
