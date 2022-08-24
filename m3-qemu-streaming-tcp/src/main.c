#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <math.h>

#include "config.h"
#include "logging.h"
#include "ip.h"
#include "comms.h"
#include "stats.h"
#include "sysexit.h"

#define LED_BLINK_PERIOD_MS pdMS_TO_TICKS(1000)

#define SENSOR_QUEUE_LENGTH (1)
#define SENSOR_PERIOD_MS pdMS_TO_TICKS(100)

static void led_task(void* params);
static void sensor_task(void* params);
static void actuator_task(void* params);
static void busy_loop(unsigned long iters);

static uint8_t SINE_WAVE[256];

int main(void)
{
    vTraceEnable(TRC_INIT);

    logging_init();
    ip_init();
    comms_init();
    stats_init();

    xTaskCreate(led_task, LED_NAME, LED_STACK_SIZE, NULL, LED_PRIO, NULL);
    xTaskCreate(sensor_task, SENSOR_NAME, SENSOR_STACK_SIZE, NULL, SENSOR_PRIO, NULL);
    xTaskCreate(actuator_task, ACTUATOR_NAME, ACTUATOR_STACK_SIZE, NULL, ACTUATOR_PRIO, NULL);

    INFO("System initialized");
    vTaskStartScheduler();

    while(1)
    {
        portNOP();
    }

    return 0;
}

static void led_task(void* params)
{
    TickType_t next_wake;
    (void) params;

    next_wake = xTaskGetTickCount();
    while(1)
    {
        vTaskDelayUntil(&next_wake, LED_BLINK_PERIOD_MS);
        INFO("blink");
    }
}

static void sensor_task(void* params)
{
    int8_t adc_value;
    int i;
    traceString ch;
    TickType_t next_wake;
    (void) params;

    i = 0;
    ch = xTraceRegisterString("adc");
    next_wake = xTaskGetTickCount();
    while(1)
    {
        const BaseType_t was_delayed = xTaskDelayUntil(&next_wake, SENSOR_PERIOD_MS);
        if(was_delayed == pdFALSE)
        {
            WARN("sensor deadline missed");
        }
        busy_loop(50000);
        adc_value = SINE_WAVE[i];
        vTracePrintF(ch, "%d", adc_value);
        comms_send_adc_measurement(adc_value);
        i += 1;
    }
}

static void actuator_task(void* params)
{
    comms_msg_s msg;
    int8_t adc_value;
    int8_t pwm_value;
    traceString pwm_ch;
    traceString act_ch;
    unsigned long busy_loop_iters = 100000;
    (void) params;

    pwm_ch = xTraceRegisterString("pwm");
    act_ch = xTraceRegisterString("act-cmd");
    while(1)
    {
        msg = comms_recv();
        if(msg.msg_kind == COMMS_MSG_ADC_MEASUREMENT)
        {
            adc_value = msg.data.adc_measurement;
            pwm_value = -1 * adc_value;
            vTracePrintF(pwm_ch, "%d", pwm_value);
            busy_loop(busy_loop_iters);
        }
        else if(msg.msg_kind == COMMS_MSG_CMD)
        {
            INFO("Actuator process command %u", msg.data.cmd);
            vTracePrintF(act_ch, "%u", msg.data.cmd);
            if(msg.data.cmd <= 100)
            {
                busy_loop_iters = 10000;
            }
            else if(msg.data.cmd <= 200)
            {
                /* the "bad" cmd */
                volatile size_t stack_waste[150];
                stack_waste[10] = 2;
                stack_waste[1] = stack_waste[10] + 1;
                busy_loop_iters = 100000;
            }
            else if(msg.data.cmd != COMMS_CMD_EXIT)
            {
                busy_loop_iters = 900000;
            }
            else if(msg.data.cmd == COMMS_CMD_EXIT)
            {
                WARN("Exiting!\n");
                vTaskDelay(pdMS_TO_TICKS(200));
                sysexit(0);
            }
        }
    }
}

static void busy_loop(unsigned long iters)
{
    unsigned long i;
    volatile float a, b, c;

    a = 42.0f;
    b = a;
    for(i = 0; i < iters; i += 1)
    {
        a /= M_PI;
        b *= M_PI;
        c = a - b;
    }
    a = c + b;
}

/* http://aquaticus.info/pwm-sine-wave */
static uint8_t SINE_WAVE[256] =
{
    0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
    0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
    0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
    0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
    0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
    0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
    0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
    0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
    0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
    0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
    0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
    0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
    0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
    0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,
    0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
    0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
    0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
    0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
    0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
    0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
    0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
    0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
    0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
    0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
    0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
    0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
    0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
    0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
    0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};
