#include "CMSIS/CMSDK_CM3.h"
#include "timer.h"

#define PRESCALE_256 (0x2)

/*
 * Setup a 32-bit timer for trace timestamping.
 * pclk == sysclk (configCPU_CLOCK_HZ).
 * Using a timer instead of TRC_CFG_ARM_CM_USE_SYSTICK (SysTick) to make
 * it count up instead of down.
 *
 * Use the following in trcConfig.h:
 * #include "timer.h"
 * #define TRC_CFG_HARDWARE_PORT TRC_HARDWARE_PORT_APPLICATION_DEFINED
 *
 * #define TRC_PORT_SPECIFIC_INIT() timer_init()
 */
void timer_init(void)
{
    CMSDK_DUALTIMER1->TimerControl = 0;
    CMSDK_DUALTIMER1->TimerLoad = 0xFFFFFFFF;
    CMSDK_DUALTIMER1->TimerIntClr |= 1;
    CMSDK_DUALTIMER1->TimerControl |=
        (CMSDK_DUALTIMER_CTRL_EN_Msk | (PRESCALE_256 << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos) | CMSDK_DUALTIMER_CTRL_SIZE_Msk);
}

uint32_t timer_read(void)
{
    /* Convert count-down to count-up */
    return ~(CMSDK_DUALTIMER1->TimerValue);
}
