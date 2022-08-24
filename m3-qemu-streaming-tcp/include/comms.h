#ifndef _COMMS_H
#define _COMMS_H

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define COMMS_MSG_ADC_MEASUREMENT (0x01)
#define COMMS_MSG_CMD (0x02)

#define COMMS_CMD_EXIT (0xFF)

typedef struct
{
    uint8_t msg_kind;
    union {
        int8_t adc_measurement;
        uint8_t cmd;
    } data;
} comms_msg_s;

void comms_init(void);

void comms_send_adc_measurement(int8_t adc_measurement);

void comms_send_cmd(uint8_t cmd);

comms_msg_s comms_recv(void);

#ifdef __cplusplus
}
#endif

#endif /* _COMMS_H */
