#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "trcRecorder.h"

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)
#if (TRC_USE_TRACEALYZER_RECORDER == 1)

#if (TRC_STREAM_PORT_USE_INTERNAL_BUFFER != 1)
#error "TRC_STREAM_PORT_USE_INTERNAL_BUFFER != 1"
#endif

#define LISTEN_PORT (8888)
#define SERVER_BACKLOG (3)

/* Control plane data in (server socket) */
static int g_server_socket = -1;
static struct sockaddr_in g_server_addr = {0};

/* Trace data out (client socket) */
static int g_trace_socket = -1;
static struct sockaddr_in g_trace_addr = {0};

static void init_server_socket(void)
{
    int err;

    if(g_server_socket == -1)
    {
        g_server_socket= socket(AF_INET, SOCK_STREAM, 0);
        assert(g_server_socket != -1);

        g_server_addr.sin_family = AF_INET;
        g_server_addr.sin_port = htons(LISTEN_PORT);
        g_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        int flag = 1;
        err = setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
        assert(err == 0);

        err = bind(g_server_socket, (const struct sockaddr*) &g_server_addr, sizeof(g_server_addr));
        assert(err == 0);

        err = listen(g_server_socket, SERVER_BACKLOG);
        assert(err == 0);

        printf("Server listening on port %u\n", LISTEN_PORT);
    }
}

/* Checks for new connection from a client */
static void init_trace_socket(void)
{
    socklen_t len;

    assert(g_server_socket != -1);

    if(g_trace_socket == -1)
    {
        /* Block until we have a client */
        printf("Waiting for a client\n");
        len = sizeof(g_trace_addr);
        g_trace_socket = accept(g_server_socket, (struct sockaddr*) &g_trace_addr, &len);
        assert(g_trace_socket != -1);

        printf("Client connected\n");
    }
}

int32_t stream_port_read(void* data, uint32_t size, int32_t* bytes_read)
{
    int err;
    fd_set fds;
    struct timeval tv = {0};
    ssize_t recvd;

    init_server_socket();
    init_trace_socket();

    FD_ZERO(&fds);
    FD_SET(g_trace_socket, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    err = select(g_trace_socket + 1, &fds, NULL, NULL, &tv);
    if((err == -1) && (errno != EINTR))
    {
        printf("select err, %s\n", strerror(errno));
        assert(err != -1);
    }

    if(err > 0)
    {
        recvd = recv(g_trace_socket, data, size, 0);
        assert(recvd >= 0);

        if(recvd == 0)
        {
            printf("Client disconnected\n");
            err = close(g_trace_socket);
            assert(err != -1);
            g_trace_socket = -1;
        }

        *bytes_read = (int32_t) recvd;
    }

    return 0;
}

/* Assumes we have a connection already */
int32_t stream_port_write(void* data, uint32_t size, int32_t *bytes_written)
{
    int err;
    ssize_t ret;

    //printf("WRITE %d\n", (int) size);
    if(bytes_written != NULL)
    {
        *bytes_written = 0;
    }

    //assert(g_server_socket != -1);
    if(g_trace_socket == -1)
    {
        return -1;
    }
    assert(g_trace_socket != -1);

    ret = send(g_trace_socket, data, (size_t) size, 0);
    //assert(ret >= 0);
    if(ret <= 0)
    {
        printf("Failed to send client data (%zd, %s), reseting connection\n", ret, strerror(errno));
        err = close(g_trace_socket);
        assert(err != -1);
        g_trace_socket = -1;
        //init_trace_socket();
        return -1;
    }

    assert(bytes_written != NULL);

    //printf("RET %d\n", (int) ret);
    if(bytes_written != NULL)
    {
        *bytes_written = (int32_t) ret;
    }

    return 0;
}

#endif /*(TRC_USE_TRACEALYZER_RECORDER == 1)*/
#endif /*(TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)*/
