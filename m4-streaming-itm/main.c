#include "stm32f4xx_conf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#if (TRC_CFG_INCLUDE_USER_EVENTS != 1)
#error "TRC_CFG_INCLUDE_USER_EVENTS"
#endif

#define SWO_BAUD (12000000)

// active-low
#define LED_PORT GPIOA
#define LED0_PIN GPIO_Pin_8
#define LED1_PIN GPIO_Pin_9
#define LED2_PIN GPIO_Pin_10

#define LED0_PRIO (tskIDLE_PRIORITY + 3)
#define LED0_BLINK_PERIOD_MS pdMS_TO_TICKS(500)

static void led12_task(void* params);
static void led0_task(void* params);

static traceString ch0;

int main(void)
{
    // NOTE: startup_stm32f40_41xxx.s calls SystemInit() before main

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    GPIO_InitTypeDef gpio_init =
    {
        .GPIO_Pin = (LED0_PIN | LED1_PIN | LED2_PIN),
        .GPIO_Mode = GPIO_Mode_OUT,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_OType = GPIO_OType_PP,
        .GPIO_PuPd = GPIO_PuPd_NOPULL,
    };
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_Init(LED_PORT, &gpio_init);
    GPIO_SetBits(LED_PORT, LED0_PIN | LED1_PIN | LED2_PIN);

    //vTraceEnable(TRC_START);
    vTraceEnable(TRC_START_AWAIT_HOST);

    xTaskCreate(led12_task, "LED12", configMINIMAL_STACK_SIZE, NULL, LED0_PRIO + 1, NULL);
    xTaskCreate(led0_task, "LED0", configMINIMAL_STACK_SIZE, NULL, LED0_PRIO, NULL);

    vTaskStartScheduler();

    while(1)
    {
        portNOP();
    }

    return 0;
}

static void led0_task(void* params)
{
    (void) params;
    uint8_t num = 0;
    TickType_t next_wake;

    ch0 = xTraceRegisterString("my channel");

    next_wake = xTaskGetTickCount();
    while(1)
    {
        vTaskDelayUntil(&next_wake, LED0_BLINK_PERIOD_MS);

        GPIO_ToggleBits(LED_PORT, LED0_PIN);

        vTracePrintF(ch0, "num = %u", num);
        vTracePrintF(ch0, "my string = %s", xTraceRegisterString("foobar"));

        num += 1;
    }
}

static void led12_task(void* params)
{
    (void) params;
    int i;

    GPIO_ResetBits(LED_PORT, LED1_PIN);
    for(i = 0; i < 10; i += 1)
    {
        GPIO_ToggleBits(LED_PORT, LED1_PIN);
        GPIO_ToggleBits(LED_PORT, LED2_PIN);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    GPIO_SetBits(LED_PORT, LED1_PIN | LED2_PIN);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    while(1)
    {
        portNOP();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char* name)
{
    (void) task;
    (void) name;

    taskDISABLE_INTERRUPTS();
    while(1)
    {
        portNOP();
    }
}
