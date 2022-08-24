#include "trcRecorder.h"

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)
#if (TRC_USE_TRACEALYZER_RECORDER == 1)

#include "FreeRTOS.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include <stdio.h>

#define SERVER_BACKLOG (1)

static Socket_t g_server_socket = FREERTOS_INVALID_SOCKET;
static struct freertos_sockaddr g_server_addr = {0};

static Socket_t g_client_socket = FREERTOS_INVALID_SOCKET;
static struct freertos_sockaddr g_client_addr = {0};

static void init_server_socket(void)
{
    BaseType_t status;

    if(g_server_addr.sin_port == 0)
    {
        g_server_addr.sin_port = FreeRTOS_htons(configTRACE_SERVER_PORT);
        g_server_addr.sin_addr = 0;
    }

    if(g_server_socket == FREERTOS_INVALID_SOCKET)
    {
        g_server_socket = FreeRTOS_socket(
                FREERTOS_AF_INET,
                FREERTOS_SOCK_STREAM,
                FREERTOS_IPPROTO_TCP);
        configASSERT(g_server_socket != FREERTOS_INVALID_SOCKET);

        status = FreeRTOS_bind(g_server_socket, &g_server_addr, sizeof(g_server_addr));
        configASSERT(status == 0);

        status = FreeRTOS_listen(g_server_socket, SERVER_BACKLOG);
        configASSERT(status == 0);

        printf("Server listening on port %u\n", configTRACE_SERVER_PORT);
    }
}

/* Checks for new connection from a client */
static void init_client_socket(void)
{
    socklen_t len;
    BaseType_t status;
    const TickType_t timeout = 0;

    configASSERT(g_server_socket != FREERTOS_INVALID_SOCKET);

    if(g_client_socket == FREERTOS_INVALID_SOCKET)
    {
        /* Block until we have a client */
        //printf("Waiting for a client\n");
        len = sizeof(g_client_addr);
        g_client_socket = FreeRTOS_accept(g_server_socket, &g_client_addr, &len);

        /* Timeout */
        if(g_client_socket == NULL)
        {
            g_client_socket = FREERTOS_INVALID_SOCKET;
        }

        if(g_client_socket != FREERTOS_INVALID_SOCKET)
        {
            status = FreeRTOS_setsockopt(g_client_socket, 0, FREERTOS_SO_RCVTIMEO, &timeout, 0);
            configASSERT(status == 0);

            printf("Client connected\n");
        }
    }
}

int32_t trace_io_read(void* data, uint32_t size, int32_t* bytes_read)
{
    int32_t ret = 0;

    configASSERT(data != NULL);
    configASSERT(size != 0);
    configASSERT(bytes_read != NULL);

    if(FreeRTOS_IsNetworkUp() == pdTRUE)
    {
        init_server_socket();
        init_client_socket();
    }

    if(g_client_socket != FREERTOS_INVALID_SOCKET)
    {
        ret = (int32_t) FreeRTOS_recv(g_client_socket, data, (size_t) size, 0);

        if(ret == FREERTOS_SOCKET_ERROR)
        {
            ret = -1;
        }
        else if(ret >= 0)
        {
            *bytes_read = ret;
            ret = 0;
        }
    }

    return ret;
}

int32_t trace_io_write(void* data, uint32_t size, int32_t* bytes_written)
{
    int32_t ret = 0;

    configASSERT(data != NULL);
    configASSERT(size != 0);
    configASSERT(bytes_written != NULL);

    if(FreeRTOS_IsNetworkUp() == pdTRUE)
    {
        init_server_socket();
        init_client_socket();
    }

    if(g_server_socket != FREERTOS_INVALID_SOCKET)
    {
        ret = (int32_t) FreeRTOS_send(g_client_socket, data, size, 0);
        if(ret < 0)
        {
            ret = -1;
        }
        else
        {
            *bytes_written = ret;
            ret = 0;
        }
    }

    return ret;
}

#endif /*(TRC_USE_TRACEALYZER_RECORDER == 1)*/
#endif /*(TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)*/
