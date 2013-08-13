#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>	/* malloc(), free() */
#include <stddef.h>	/* offsetof */
#include <stdbool.h>	/* offsetof */
#include <unistd.h>
#include <netinet/in.h> /* struct sockaddr */
#include <stdio.h>	/* printf */
#include <errno.h>	/* sancus_fd */
#include <string.h>	/* sancus_zalloc uses memset */

#include <ev.h>
#include <sancus_common.h>
#include <sancus_alloc.h>
#include <sancus_buffer.h>
#include <sancus_stream.h>
#include <sancus_fd.h>

#include "tcp_echo_server.h"


static bool echo_stream_error(struct sancus_stream *UNUSED(stream),
		   struct ev_loop *UNUSED(loop),
		   enum sancus_stream_error error)
{
	switch (error) {
	case SANCUS_STREAM_READ_ERROR:
		printf("echo_stream_error(): read error\n"); break;
	case SANCUS_STREAM_READ_EOF:
		printf("echo_stream_error(): stream read eof\n"); break;
	case SANCUS_STREAM_READ_FULL:
		printf("echo_stream_error(): read full\n"); break;
	}
	return true;
}

ssize_t echo_stream_read(struct sancus_stream *stream, char *buffer, size_t len)
{
	printf("echo_stream_read(): read buffer at %p has %zu bytes\n", buffer, len);
	if (sancus_write(stream->read_watcher.fd, buffer, len) < 0)
		return -1;

	return len;
}

static void echo_stream_close(struct sancus_stream *UNUSED(stream))
{
	printf("echo_stream_close()\n");
}

static void echo_session_pre_bind(struct sancus_tcp_server *UNUSED(server))
{
	printf("echo_on_prebind()\n");
}

static void echo_session_error(struct sancus_tcp_server *UNUSED(server),
		   struct ev_loop *UNUSED(loop),
		   enum sancus_tcp_server_error error)
{
	switch (error) {
	case SANCUS_TCP_SERVER_ACCEPT_ERROR:
		printf("accept error\n"); break;
	}
}

bool echo_session_new(struct sancus_tcp_server *UNUSED(server),
		 struct ev_loop *loop, int fd,
		 struct sockaddr *UNUSED(peer), socklen_t UNUSED(peerlen))
{
	struct echo_session *self = sancus_zalloc(sizeof(*self));

	static struct sancus_stream_settings settings = {
		.on_read = echo_stream_read,
		.on_close = echo_stream_close,
		.on_error = echo_stream_error,
	};

	if (self) {
		printf("echo_session_new() fd=%d\n", fd);
		sancus_stream_init(&self->conn, &settings, fd, self->read_buffer, sizeof(self->read_buffer));
		sancus_stream_start(&self->conn, loop);
	}

	return self != NULL;
}

static int echo_init(struct echo_server *self, const char *addr, unsigned port)
{
	static const struct sancus_tcp_server_settings settings = {
		.pre_bind = echo_session_pre_bind,
		.on_connect = echo_session_new,
		.on_error = echo_session_error,
	};

	/* socket */
	switch (sancus_tcp_ipv4_listen(&self->tcp_server, &settings, addr, port, 1, LISTEN_BACKLOG)) {
	case -1:
		//syserrf("sancus_tcp_ipv4_listen(..., \"%s\", %u)", addr, port);
		return -1;
	case 0:
		//errf("sancus_tcp_ipv4_listen(..., \"%s\", %u): bad address",
		//     addr, port);
		return -1;
	}

	return self != NULL;
}

static inline void echo_start(struct echo_server *self, struct ev_loop *loop)
{
	sancus_tcp_server_start(&self->tcp_server, loop);
}


/*
 * main
 */
int main(int UNUSED(argc), char * UNUSED(argv[]))
{
	struct ev_loop *loop = ev_default_loop(0);
	struct echo_server *server = sancus_zalloc(sizeof(struct echo_server));

	if (echo_init(server, "127.0.0.1", 12345) < 0)
		return -1;

	echo_start(server, loop);

	ev_loop(loop, 0);

	return 0;
}
