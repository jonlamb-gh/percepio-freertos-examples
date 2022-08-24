#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "logging.h"
#include "config.h"
#include "comms.h"

typedef struct
{
    uint8_t magic0; /* 0xAA */
    uint8_t magic1; /* 0xBB */
    uint8_t cmd;
    uint8_t magic2; /* 0xCC */
} __attribute__((packed, aligned(1))) cmd_s;

static void comms_task(void* params);

#define COMMS_QUEUE_LENGTH (8)
static QueueHandle_t g_comms_queue = NULL;
static Socket_t g_socket = FREERTOS_INVALID_SOCKET;
static struct freertos_sockaddr g_addr = {0};

void comms_init(void)
{
    g_comms_queue = xQueueCreate(COMMS_QUEUE_LENGTH, sizeof(comms_msg_s));
    configASSERT(g_comms_queue != NULL);

    xTaskCreate(comms_task, COMMS_NAME, COMMS_STACK_SIZE, NULL, COMMS_PRIO, NULL);
}

void comms_send_adc_measurement(int8_t adc_measurement)
{
    comms_msg_s msg =
    {
        .msg_kind = COMMS_MSG_ADC_MEASUREMENT,
        .data.adc_measurement = adc_measurement,
    };

    const BaseType_t was_posted = xQueueSend(g_comms_queue, &msg, 0);
    if(was_posted != pdTRUE)
    {
        ERR("Failed to send ADC measurement");
    }
}

void comms_send_cmd(uint8_t cmd)
{
    comms_msg_s msg =
    {
        .msg_kind = COMMS_MSG_CMD,
        .data.cmd = cmd,
    };

    const BaseType_t was_posted = xQueueSend(g_comms_queue, &msg, 0);
    if(was_posted != pdTRUE)
    {
        ERR("Failed to send command");
    }
}

comms_msg_s comms_recv(void)
{
    comms_msg_s msg;
    const BaseType_t was_recvd = xQueueReceive(g_comms_queue, &msg, portMAX_DELAY);
    configASSERT(was_recvd == pdTRUE);
    return msg;
}

static void comms_task(void* params)
{
    BaseType_t status;
    traceString ch;
    cmd_s cmd;
    struct freertos_sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    (void) params;

    ch = xTraceRegisterString("comms-cmd");

    while(FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    g_socket = FreeRTOS_socket(
            FREERTOS_AF_INET,
            FREERTOS_SOCK_DGRAM,
            FREERTOS_IPPROTO_UDP);
    configASSERT(g_socket != FREERTOS_INVALID_SOCKET);

    TickType_t timeout = portMAX_DELAY;
    status = FreeRTOS_setsockopt(g_socket, 0, FREERTOS_SO_RCVTIMEO, &timeout, 0);
    configASSERT(status == 0);

    g_addr.sin_port = FreeRTOS_htons(configCMD_PORT);
    g_addr.sin_addr = 0;
    status = FreeRTOS_bind(g_socket, &g_addr, sizeof(g_addr));
    configASSERT(status == 0);

    INFO("Command service listening on port %d", configCMD_PORT);

    while(1)
    {
        status = FreeRTOS_recvfrom(g_socket, &cmd, sizeof(cmd), 0, &client_addr, &client_addr_len);
        if(status < 0)
        {
           ERR("Command service recv error (%ld)", status);
        }
        else if(status == sizeof(cmd))
        {
            if((cmd.magic0 == 0xAA) && (cmd.magic1 == 0xBB) && (cmd.magic2 == 0xCC))
            {
                INFO("Command service recv command %u", (unsigned int) cmd.cmd);
                vTracePrintF(ch, "%u", cmd.cmd);
                comms_send_cmd(cmd.cmd);
            }
            else
            {
                ERR("Received invalid command magic");
            }
        }
        else if(status > 0)
        {
            ERR("Received invalid command size");
        }
    }
}
