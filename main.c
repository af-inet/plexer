#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "socket.h"
#include "server.h"
#include "http.h"
#include "table.h"
#include "file.h"

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

void request(server_t *server, client_t *client){
	char buf[4096] = {0};
    struct plxr_http_request req;

	switch( server_read(server, client, buf, 4095) ){

	case READ_ERROR: /* fallthrough */
	case READ_ZERO:
		server_client_shutdown(server, client);
		break;

	default:
        if( plxr_http_parse(&req, buf) ) {
			if ( strlen(req.uri) > 0 ) {
				printf("Requested: [%s]\n", req.uri+1); // cut off "/"
				client->data = htable_gets(&g_table, req.uri+1);
			}
        } else {
            printf("Invalid HTTP Request\n");
		}
		break;	
	}
}

const char *html_404 = 
	"<html>"
	"<body>"
	"<h1> 404 Not Found </h1>"
	"</body>"
	"</html>"
;

void response(server_t *server, client_t *client){
	size_t data_len, header_len;
	char header[4096], *data;

	// If we've already wrote or haven't read, don't bother responding.
	if( (client->bytes_wrote != 0) || (client->bytes_read == 0) ) {
		return;
	}

	if ( client->data == NULL )
		data = html_404;
	else
		data = client->data;

	data_len = strlen(data);
	header_len = plxr_http_response(header, sizeof(header), 404, data_len);

	server_write(server, client, header, header_len);
	server_write(server, client, data, data_len);

	server_client_shutdown(server, client);
}

int main(int argc, char *argv[]){
	server_t server;

	// Load the current directory into a hash table.
	g_table.size = TABLE_SIZE;
	g_table.entries = g_entries;
	bzero(g_entries, sizeof(g_entries));
	plxr_ntfw(&table_insert);

	// initialize the server
	server_init(&server);
	server.event_read = &request;
	server.event_write = &response;
	server_run(&server);

	return 0;
}
