/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configASSERT_DEFINED                             1
extern void vAssertCalled( void );
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled()
#define configQUEUE_REGISTRY_SIZE                        20

#define configUSE_PREEMPTION                             1
#define configUSE_TIME_SLICING                           1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION          0

#define configUSE_IDLE_HOOK                              0
#define configUSE_TICK_HOOK                              0
#define configUSE_DAEMON_TASK_STARTUP_HOOK               0
#define configCPU_CLOCK_HZ                               ( ( unsigned long ) 20000000 )
#define configTICK_RATE_HZ                               ( ( TickType_t ) 1000 )
#define configMINIMAL_STACK_SIZE                         ( ( unsigned short ) 256 )
#define configTOTAL_HEAP_SIZE                            ( ( size_t ) ( 279000 ) )
#define configMAX_TASK_NAME_LEN                          ( 12 )
#define configUSE_16_BIT_TICKS                           0
#define configIDLE_SHOULD_YIELD                          1
#define configUSE_CO_ROUTINES                            0
#define configUSE_APPLICATION_TASK_TAG                   1
#define configCHECKFORSTACK_OVERFLOW                     2
#define configUSE_TRACE_FACILITY                         1

/* NOTE: we're already configuring the timer for tracing in TRC_PORT_SPECIFIC_INIT
 * which happens earlier than vTaskStartScheduler().
 * So portCONFIGURE_TIMER_FOR_RUN_TIME_STATS is a no-op.
 * See timer.h
 */
#define configGENERATE_RUN_TIME_STATS                    1
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() (timer_read())
#define configRECORD_STACK_HIGH_ADDRESS                  0

#define configMAX_PRIORITIES                             ( 10 )
#define configMAX_CO_ROUTINE_PRIORITIES                  ( 2 )
#define configTIMER_QUEUE_LENGTH                         20
#define configTIMER_TASK_PRIORITY                        ( configMAX_PRIORITIES - 1 )
#define configUSE_COUNTING_SEMAPHORES                    1
#define configSUPPORT_DYNAMIC_ALLOCATION                 1
#define configSUPPORT_STATIC_ALLOCATION                  0
#define configNUM_TX_DESCRIPTORS                         15

#define configUSE_MALLOC_FAILED_HOOK              1
#define configUSE_MUTEXES                         1
#define configUSE_RECURSIVE_MUTEXES               1
#define configUSE_TIMERS                          1
#define configTIMER_TASK_STACK_DEPTH              ( configMINIMAL_STACK_SIZE * 2 )

#define INCLUDE_vTaskPrioritySet                  1
#define INCLUDE_uxTaskPriorityGet                 1
#define INCLUDE_vTaskDelete                       0
#define INCLUDE_vTaskCleanUpResources             0
#define INCLUDE_vTaskSuspend                      0
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_xTaskDelayUntil                   1
#define INCLUDE_uxTaskGetStackHighWaterMark       0
#define INCLUDE_uxTaskGetStackHighWaterMark2      0
#define INCLUDE_xTaskGetSchedulerState            0
#define INCLUDE_xTaskGetIdleTaskHandle            0
#define INCLUDE_xTaskGetHandle                    1
#define INCLUDE_eTaskGetState                     0
#define INCLUDE_xSemaphoreGetMutexHolder          0
#define INCLUDE_xTimerPendFunctionCall            0
#define INCLUDE_xTaskAbortDelay                   0
#define INCLUDE_xTaskGetCurrentTaskHandle         1

#define projCOVERAGE_TEST                         0

#define configKERNEL_INTERRUPT_PRIORITY           252

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    5 /* equivalent to 0xa0, or priority 5. */
#define configMAC_INTERRUPT_PRIORITY            2

/* networking definitions */
#define configMAC_ISR_SIMULATOR_PRIORITY     ( configMAX_PRIORITIES - 2 )
#define ipconfigUSE_NETWORK_EVENT_HOOK 1
#define configNETWORK_INTERFACE_TO_USE 1L

#define nwRX_TASK_STACK_SIZE                   ( configMINIMAL_STACK_SIZE * 2 )
#define nwETHERNET_RX_HANDLER_TASK_PRIORITY    ( configMAX_PRIORITIES - 3 )

/* Default MAC address configuration.  The demo creates a virtual network
connection that uses this MAC address by accessing the raw Ethernet/WiFi data
to and from a real network connection on the host PC.  See the
configNETWORK_INTERFACE_TO_USE definition above for information on how to
configure the real network connection to use. */
#define configMAC_ADDR0    0x52
#define configMAC_ADDR1    0x54
#define configMAC_ADDR2    0x00
#define configMAC_ADDR3    0x12
#define configMAC_ADDR4    0x34
#define configMAC_ADDR5    0xAD

/* Default IP address configuration, 192.0.2.80. */
#define configIP_ADDR0      192
#define configIP_ADDR1      0
#define configIP_ADDR2      2
#define configIP_ADDR3      80

/* Default gateway IP address configuration, 192.0.2.254. */
#define configGATEWAY_ADDR0 192
#define configGATEWAY_ADDR1 0
#define configGATEWAY_ADDR2 2
#define configGATEWAY_ADDR3 254

/* Default netmask configuration. */
#define configNET_MASK0     255
#define configNET_MASK1     255
#define configNET_MASK2     255
#define configNET_MASK3     0

/* Default DNS server configuration, 192.0.2.254. */
#define configDNS_SERVER_ADDR0  192
#define configDNS_SERVER_ADDR1  0
#define configDNS_SERVER_ADDR2  2
#define configDNS_SERVER_ADDR3  254

#define configTRACE_SERVER_PORT 8888
#define configCMD_PORT 9999

#include "trcRecorder.h"

#endif /* FREERTOS_CONFIG_H */
