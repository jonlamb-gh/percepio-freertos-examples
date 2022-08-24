/* NOTE: requires the `-semihosting-config enable=on,target=native` argument */

#include "sysexit.h"

#define ADP_STOPPED_APP_EXIT (0x20026)
#define SYS_EXIT (0x18)

/* Make QEMU exit with a zero or one exit code */
void sysexit(int exit_code)
{
    register int reg0 __asm("r0");
    register int reg1 __asm("r1");
    reg0 = SYS_EXIT;
    reg1 = exit_code == 0 ? ADP_STOPPED_APP_EXIT : 1;
    (void) reg0;
    (void) reg1;
    __asm__ volatile("bkpt 0xAB");
}
