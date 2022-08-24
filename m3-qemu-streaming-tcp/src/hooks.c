#include "FreeRTOS.h"
#include "task.h"

#include <stdarg.h>
#include <stdio.h>

void vLoggingPrintf(const char * fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vprintf(fmt, arg);
    va_end(arg);
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    printf("vApplicationMallocFailedHook\n");
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
    printf("vApplicationStackOverflowHook task='%s'\n", name);
    while(1)
    {
        portNOP();
    }
}

void vAssertCalled(void)
{
    taskDISABLE_INTERRUPTS();
    printf("vAssertCalled\n");
    while(1)
    {
        portNOP();
    }
}
