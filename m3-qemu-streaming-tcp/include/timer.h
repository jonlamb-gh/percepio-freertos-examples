#ifndef _TIMER_H
#define _TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#define TRC_HWTC_TYPE TRC_FREE_RUNNING_32BIT_INCR
#define TRC_HWTC_COUNT (timer_read())
#define TRC_HWTC_PERIOD 0
#define TRC_HWTC_DIVISOR 4
#define TRC_HWTC_FREQ_HZ (TRACE_CPU_CLOCK_HZ / 256)
#define TRC_IRQ_PRIORITY_ORDER 0

#define TRC_PORT_SPECIFIC_INIT() timer_init()

void timer_init(void);

uint32_t timer_read(void);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H */
