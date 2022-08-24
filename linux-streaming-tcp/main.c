/* https://www.freertos.org/Hardware-independent-RTOS-example.html */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <inttypes.h>
#include <time.h>
#include <signal.h>

/* Kernel includes. */
#include "FreeRTOS.h" /* Must come first. */
#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

/* Priorities at which the tasks are created.  The event semaphore task is
given the maximum priority of ( configMAX_PRIORITIES - 1 ) to ensure it runs as
soon as the semaphore is given. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )
#define mainEVENT_SEMAPHORE_TASK_PRIORITY   ( configMAX_PRIORITIES - 1 )

/* The rate at which data is sent to the queue, specified in milliseconds, and
converted to ticks using the pdMS_TO_TICKS() macro. */
#define mainQUEUE_SEND_PERIOD_MS            pdMS_TO_TICKS( 200 )

/* The period of the example software timer, specified in milliseconds, and
converted to ticks using the pdMS_TO_TICKS() macro. */
#define mainSOFTWARE_TIMER_PERIOD_MS        pdMS_TO_TICKS( 1000 )

/* The number of items the queue can hold.  This is 1 as the receive task
has a higher priority than the send task, so will remove items as they are added,
meaning the send task should always find the queue empty. */
#define mainQUEUE_LENGTH                    ( 1 )

/*-----------------------------------------------------------*/

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */
static void prvSetupHardware( void );

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*
 * The callback function assigned to the example software timer as described at
 * the top of this file.
 */
static void vExampleTimerCallback( TimerHandle_t xTimer );

/*
 * The event semaphore task as described at the top of this file.
 */
static void prvEventSemaphoreTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* Global signal flag, triggered by SIGINT */
static volatile sig_atomic_t g_signaled = 0;

/* Time at start of day (in ns). */
static unsigned long ulStartTimeNs;

/* The queue used by the queue send and queue receive tasks. */
static QueueHandle_t xQueue = NULL;

/* The semaphore (in this case binary) that is used by the FreeRTOS tick hook
 * function and the event semaphore task.
 */
static SemaphoreHandle_t xEventSemaphore = NULL;

/* The counters used by the various examples.  The usage is described in the
 * comments at the top of this file.
 */
static volatile uint32_t ulCountOfTimerCallbackExecutions = 0;
static volatile uint32_t ulCountOfItemsReceivedOnQueue = 0;
static volatile uint32_t ulCountOfReceivedSemaphores = 0;

static traceString uechannel2;
static traceString uechannel3;

traceHandle traceHandleIsrTimer1 = 0;

/*-----------------------------------------------------------*/

static void sigint_handler(int sig)
{
    (void) sig; /* Assumed it's SIGINT, that's all we register for */
    if(g_signaled == 0)
    {
        g_signaled = 1;
    }
    else
    {
        exit(-1);
    }
}

static void sigint_register(void)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_RESTART;
    act.sa_handler = sigint_handler;
    if(sigaction(SIGINT, &act, 0) < 0)
    {
        perror("sigaction(SIGINT)");
        exit(1);
    }
    printf("Registered a signal handler for SIGINT (control-c)\n");
}

int main(void)
{
    printf("*********** starting ***********\n");

    TimerHandle_t xExampleSoftwareTimer = NULL;

    /* Configure the system ready to run the demo.  The clock configuration
    can be done here if it was not done before main() was called. */
    prvSetupHardware();

    vTraceEnable(TRC_START_AWAIT_HOST);

    traceString uechannel1 = xTraceRegisterString("Custom Stuff chan");
    printf("uechannel1 = %s\n", uechannel1);
    vTracePrintF(uechannel1, "My thing %d = %d", 2, -5);
    vTracePrint(uechannel1, "just a string");
    vTracePrintF(uechannel1, "my string = %s", xTraceRegisterString("foobar"));

    //traceString uechannel2 = xTraceRegisterString("More foo bar chan");
    uechannel2 = xTraceRegisterString("More foo bar chan");
    printf("uechannel2 = %s\n", uechannel2);
    vTracePrintF(uechannel2, "Biz Blah = %d", -10);
    vTracePrintF(uechannel2, "Biz Blah sign = %bd", -10);
    vTracePrintF(uechannel2, "just 8bits = %bu", 0xF);
    vTracePrintF(uechannel2, "just 16bits = %hu", 0xAAAA);
    vTracePrintF(uechannel2, "2 8bits %bu = %bu", 0xBB, 0xCC);

    uechannel3 = xTraceRegisterString("voltage");


#define PRIO_OF_ISR_TIMER1 1
    traceHandleIsrTimer1 = xTraceSetISRProperties("ISRTimer1", PRIO_OF_ISR_TIMER1);
    //traceHandleIsrTimer1 = xTraceSetISRProperties("ISRTimer2", PRIO_OF_ISR_TIMER1);

    // these aren't available in snapshot mode...
    //vTraceConsoleChannelPrintF("Console message A %d", 245);
    //vTraceConsoleChannelPrintF("Console message B %d", 45);

    /* Create the queue used by the queue send and queue receive tasks. */
    xQueue = xQueueCreate(     /* The number of items the queue can hold. */
                            mainQUEUE_LENGTH,
                            /* The size of each item the queue holds. */
                            sizeof( uint32_t ) );


    /* Create the semaphore used by the FreeRTOS tick hook function and the
    event semaphore task.  NOTE: A semaphore is used for example purposes,
    using a direct to task notification will be faster! */
    xEventSemaphore = xSemaphoreCreateBinary();


    /* Create the queue receive task as described in the comments at the top
    of this file. */
    xTaskCreate(     /* The function that implements the task. */
                    prvQueueReceiveTask,
                    /* Text name for the task, just to help debugging. */
                    "Rx",
                    /* The size (in words) of the stack that should be created
                    for the task. */
                    configMINIMAL_STACK_SIZE,
                    /* A parameter that can be passed into the task.  Not used
                    in this simple demo. */
                    NULL,
                    /* The priority to assign to the task.  tskIDLE_PRIORITY
                    (which is 0) is the lowest priority.  configMAX_PRIORITIES - 1
                    is the highest priority. */
                    mainQUEUE_RECEIVE_TASK_PRIORITY,
                    /* Used to obtain a handle to the created task.  Not used in
                    this simple demo, so set to NULL. */
                    NULL );


    /* Create the queue send task in exactly the same way.  Again, this is
    described in the comments at the top of the file. */
    xTaskCreate(     prvQueueSendTask,
                    //"TX",
                    NULL,
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    mainQUEUE_SEND_TASK_PRIORITY,
                    NULL );


    /* Create the task that is synchronised with an interrupt using the
    xEventSemaphore semaphore. */
    xTaskCreate(     prvEventSemaphoreTask,
                    "Sem",
                    configMINIMAL_STACK_SIZE,
                    NULL,
                    mainEVENT_SEMAPHORE_TASK_PRIORITY,
                    NULL );


    /* Create the software timer as described in the comments at the top of
    this file. */
    xExampleSoftwareTimer = xTimerCreate(     /* A text name, purely to help
                                            debugging. */
                                            ( const char * ) "LEDTimer",
                                            /* The timer period, in this case
                                            1000ms (1s). */
                                            mainSOFTWARE_TIMER_PERIOD_MS,
                                            /* This is a periodic timer, so
                                            xAutoReload is set to pdTRUE. */
                                            pdTRUE,
                                            /* The ID is not used, so can be set
                                            to anything. */
                                            ( void * ) 0,
                                            /* The callback function that switches
                                            the LED off. */
                                            vExampleTimerCallback
                                        );

    /* Start the created timer.  A block time of zero is used as the timer
    command queue cannot possibly be full here (this is the first timer to
    be created, and it is not yet running). */
    xTimerStart( xExampleSoftwareTimer, 0 );

    /* Start the tasks and timer running. */
    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details.  */
    for( ;; );
}
/*-----------------------------------------------------------*/

static void vExampleTimerCallback( TimerHandle_t xTimer )
{
    /* The timer has expired.  Count the number of times this happens.  The
    timer that calls this function is an auto re-load timer, so it will
    execute periodically. */
    ulCountOfTimerCallbackExecutions++;
    printf("vExampleTimerCallback %u\n", ulCountOfTimerCallbackExecutions);
    //vTraceConsoleChannelPrintF("ulCountOfTimerCallbackExecutions = %u", ulCountOfTimerCallbackExecutions);
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
    TickType_t xNextWakeTime;
    const uint32_t ulValueToSend = 100UL;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, the constant used converts ticks
        to ms.  The task will not consume any CPU time while it is in the
        Blocked state. */
        vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_PERIOD_MS );

        /* Send to the queue - causing the queue receive task to unblock and
        increment its counter.  0 is used as the block time so the sending
        operation will not block - it shouldn't need to block as the queue
        should always be empty at this point in the code. */
        printf("Sending %" PRIu32 "\n", ulValueToSend);
        xQueueSend( xQueue, &ulValueToSend, 0 );
    }
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
    uint32_t ulReceivedValue;

    for( ;; )
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
        printf("Received %" PRIu32 "\n", ulReceivedValue);

        /*  To get here something must have been received from the queue, but
        is it the expected value?  If it is, increment the counter. */
        if( ulReceivedValue == 100UL )
        {
            /* Count the number of items that have been received correctly. */
            ulCountOfItemsReceivedOnQueue++;
        }
    }
}
/*-----------------------------------------------------------*/

static void prvEventSemaphoreTask( void *pvParameters )
{
    for( ;; )
    {
        /* Block until the semaphore is 'given'.  NOTE:
        A semaphore is used for example purposes.  In a real application it might
        be preferable to use a direct to task notification, which will be faster
        and use less RAM. */
        xSemaphoreTake( xEventSemaphore, portMAX_DELAY );

        /* Count the number of times the semaphore is received. */
        ulCountOfReceivedSemaphores++;
        printf("Got semaphore %u\n", ulCountOfReceivedSemaphores);
    }
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint32_t ulCount = 0;
    vTraceStoreISRBegin(traceHandleIsrTimer1);

    vTracePrintF(uechannel3, "%u", ulCount);

    /* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
    1 in FreeRTOSConfig.h.

    "Give" the semaphore on every 500th tick interrupt. */
    ulCount++;
    if( ulCount >= 500UL )
    {
        /* This function is called from an interrupt context (the RTOS tick
        interrupt),    so only ISR safe API functions can be used (those that end
        in "FromISR()".

        xHigherPriorityTaskWoken was initialised to pdFALSE, and will be set to
        pdTRUE by xSemaphoreGiveFromISR() if giving the semaphore unblocked a
        task that has equal or higher priority than the interrupted task.
        NOTE: A semaphore is used for example purposes.  In a real application it
        might be preferable to use a direct to task notification,
        which will be faster and use less RAM. */
        xSemaphoreGiveFromISR( xEventSemaphore, &xHigherPriorityTaskWoken );
        ulCount = 0UL;

        //g_signaled = 1; // TODO
    }

    /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).  The syntax required to context switch from
    an interrupt is port dependent, so check the documentation of the port you
    are using.

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
    vTraceStoreISREnd(0);
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) xTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    for( ;; );
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char * const pcFileName, unsigned long ulLine)
{
    printf("vAssertCalled( %s, %lu )\n", pcFileName, ulLine);
    volatile unsigned long looping = 0;

    taskENTER_CRITICAL();
    {
        /* Use the debugger to set ul to a non-zero value in order to step out
         *      of this function to determine why it was called. */
        while( looping == 0LU )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
    //static const char TRACE_FILE[] = "trace.snapshot";

    if(g_signaled != 0)
    {
        printf("*********** stopping ***********\n");

        //vTraceConsoleChannelPrintF("blah blah foo");
        vTracePrintF(uechannel2, "all done marker = %u", 0xAAAAAAAA);

        vTraceStop();

        /*
        printf("Writing trace snapshot to file '%s'\n", TRACE_FILE);
        printf("sizeof(RecorderDataType) == %zu\n", sizeof(RecorderDataType));
        printf("sizeof(ObjectPropertyTableType) == %zu\n", sizeof(ObjectPropertyTableType));
        printf("offsetof(RecorderDataType, ObjectPropertyTable) == %zu\n", offsetof(RecorderDataType, ObjectPropertyTable));
        printf("RecorderDataPtr->ObjectPropertyTable.ObjectPropertyTableSizeInBytes == %" PRIu32 "\n", RecorderDataPtr->ObjectPropertyTable.ObjectPropertyTableSizeInBytes);
        printf("offsetof(RecorderDataType, debugMarker1) == %zu\n", offsetof(RecorderDataType, debugMarker1));
        printf("TRACE_NCLASSES = %u\n", TRACE_NCLASSES);
        printf("4*((TRACE_NCLASSES+3)/4) = %u\n", 4*((TRACE_NCLASSES+3)/4));
        printf("2*((TRACE_NCLASSES+1)/2) = %u\n", 2*((TRACE_NCLASSES+1)/2));

        FILE * const f = fopen(TRACE_FILE, "wb");
        configASSERT(f != NULL);
        configASSERT(RecorderDataPtr != NULL);
        const size_t num_items = fwrite(RecorderDataPtr, sizeof(RecorderDataType), 1, f);
        configASSERT(num_items == 1);
        (void) fflush(f);
        (void) fclose(f);
        */

        exit(0);
    }
}
/*-----------------------------------------------------------*/

void vConfigureTimerForRunTimeStats( void )
{
    struct timespec xNow;

    clock_gettime(CLOCK_MONOTONIC, &xNow);
    ulStartTimeNs = xNow.tv_sec * 1000000000ul + xNow.tv_nsec;
}

unsigned long ulGetRunTimeCounterValue( void )
{
    struct timespec xNow;

    /* Time at start. */
    clock_gettime(CLOCK_MONOTONIC, &xNow);

    return xNow.tv_sec * 1000000000ul + xNow.tv_nsec - ulStartTimeNs;
}

static void prvSetupHardware( void )
{
    /* Ensure all priority bits are assigned as preemption priority bits
    if using a ARM Cortex-M microcontroller. */
    /* NVIC_SetPriorityGrouping( 0 ); */

    /* TODO: Setup the clocks, etc. here, if they were not configured before
    main() was called. */

    if(0)sigint_register();
    vConfigureTimerForRunTimeStats();
}
