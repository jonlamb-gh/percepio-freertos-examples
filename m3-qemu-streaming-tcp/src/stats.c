#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"

//#define PRINT_STATS
#ifdef PRINT_STATS
#include <stdio.h>
#endif
#include <string.h>

#include "config.h"
#include "stats.h"

#define STATS_PERIOD_MS pdMS_TO_TICKS(2000)
#define MAX_TASKS (12)

typedef struct
{
    traceString sym;
    const char* name;
} task_name_s;

static void stats_task(void* params);
static void log_task_stats(const TaskStatus_t* s, unsigned long total_runtime_percentage);
static traceString task_name_sym(const char* name);
static uint32_t task_stack_size(const char* name);

static traceString g_stats_chn = NULL;
static TaskStatus_t g_task_stats[MAX_TASKS] = {0};
static task_name_s g_task_names[MAX_TASKS] = {0};

void stats_init(void)
{
    xTaskCreate(stats_task, STATS_NAME, STATS_STACK_SIZE, NULL, STATS_PRIO, NULL);
}

static void stats_task(void* params)
{
    TickType_t next_wake;
    UBaseType_t i;
    UBaseType_t num_tasks;
    unsigned long total_runtime;
    unsigned long total_runtime_percentage;
    (void) params;

    g_stats_chn = xTraceRegisterString("stats");
    next_wake = xTaskGetTickCount();
    while(1)
    {
        vTaskDelayUntil(&next_wake, STATS_PERIOD_MS);

        num_tasks = uxTaskGetNumberOfTasks();
        configASSERT(num_tasks <= MAX_TASKS);

        num_tasks = uxTaskGetSystemState(g_task_stats, num_tasks, &total_runtime);

        total_runtime /= 100;
        if(total_runtime != 0)
        {
            for(i = 0; i < num_tasks; i += 1)
            {
                total_runtime_percentage = g_task_stats[i].ulRunTimeCounter / total_runtime;

                /* If the percentage is zero here then the task has
                consumed less than 1% of the total run time. */
                if(total_runtime_percentage == 0)
                {
                    total_runtime_percentage = 1;
                }

                log_task_stats(&g_task_stats[i], total_runtime_percentage);
            }
        }
    }
}

static void log_task_stats(const TaskStatus_t* s, unsigned long total_runtime_percentage)
{
#ifdef PRINT_STATS
    printf("[%s] %lu %d %lu %lu%%\n",
            s->pcTaskName,
            task_stack_size(s->pcTaskName),
            s->usStackHighWaterMark,
            s->ulRunTimeCounter,
            total_runtime_percentage);
#endif
    vTracePrintF(
            g_stats_chn,
            "%s %u %d %u %u",
            task_name_sym(s->pcTaskName),
            task_stack_size(s->pcTaskName),
            s->usStackHighWaterMark,
            s->ulRunTimeCounter,
            total_runtime_percentage);

}

static traceString task_name_sym(const char* name)
{
    int i;
    traceString sym = NULL;

    for(i = 0; i < MAX_TASKS; i += 1)
    {
        if(g_task_names[i].sym != NULL)
        {
            if(strncmp(g_task_names[i].name, name, configMAX_TASK_NAME_LEN) == 0)
            {
                sym = g_task_names[i].sym;
                break;
            }
        }
    }

    if(sym == NULL)
    {
        for(i = 0; i < MAX_TASKS; i += 1)
        {
            if(g_task_names[i].sym == NULL)
            {
                configASSERT(g_task_names[i].name == NULL);
                sym = xTraceRegisterString(name);
                g_task_names[i].sym = sym;
                g_task_names[i].name = name;
                break;
            }
        }
    }

    configASSERT(sym != NULL);

    return sym;
}

static uint32_t task_stack_size(const char* name)
{
    uint32_t stack_size = 0;

    if(strncmp(name, STATS_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = STATS_STACK_SIZE;
    }
    else if(strncmp(name, LED_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = LED_STACK_SIZE;
    }
    else if(strncmp(name, SENSOR_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = SENSOR_STACK_SIZE;
    }
    else if(strncmp(name, ACTUATOR_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = ACTUATOR_STACK_SIZE;
    }
    else if(strncmp(name, COMMS_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = COMMS_STACK_SIZE;
    }
    else if(strncmp(name, IDLE_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = IDLE_STACK_SIZE;
    }
    else if(strncmp(name, TIMER_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = TIMER_STACK_SIZE;
    }
    else if(strncmp(name, TZCTRL_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = TZCTRL_STACK_SIZE;
    }
    else if(strncmp(name, IP_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = IP_STACK_SIZE;
    }
    else if(strncmp(name, EMAC_NAME, configMAX_TASK_NAME_LEN) == 0)
    {
        stack_size = EMAC_STACK_SIZE;
    }

    configASSERT(stack_size != 0);
    return stack_size;
}
