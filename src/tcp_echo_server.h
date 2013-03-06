#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sancus_buffer.h>
#include <sancus_tcp_server.h>
#include <sancus_stream.h>

enum {
	LISTEN_BACKLOG = 5,

	READ_BUFFER_SIZE = 1024,
};

struct echo_server {
	struct sancus_tcp_server tcp_server;
};

struct echo_session {
	struct sancus_stream conn;
	char read_buffer[READ_BUFFER_SIZE];
};

