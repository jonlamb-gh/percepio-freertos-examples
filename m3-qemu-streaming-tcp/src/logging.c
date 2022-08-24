#include "FreeRTOS.h"

#include <stdio.h>
#include <stdarg.h>

#include "logging.h"

static traceString g_info_ch = NULL;
static traceString g_warn_ch = NULL;
static traceString g_err_ch = NULL;

void logging_init(void)
{
    g_info_ch = xTraceRegisterString("info");
    configASSERT(g_info_ch != NULL);

    g_warn_ch = xTraceRegisterString("warn");
    configASSERT(g_warn_ch != NULL);

    g_err_ch = xTraceRegisterString("error");
    configASSERT(g_err_ch != NULL);
}

void logging_info(const char* fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    vTraceVPrintF(g_info_ch, fmt, vl);
    va_end(vl);

#ifdef LOGGING_STDOUT_MIRROR
    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
    printf("\n");
#endif
}

void logging_warn(const char* fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    vTraceVPrintF(g_warn_ch, fmt, vl);
    va_end(vl);

#ifdef LOGGING_STDOUT_MIRROR
    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
    printf("\n");
#endif
}

void logging_error(const char* fmt, ...)
{
    va_list vl;

    va_start(vl, fmt);
    vTraceVPrintF(g_err_ch, fmt, vl);
    va_end(vl);

#ifdef LOGGING_STDOUT_MIRROR
    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);
    printf("\n");
#endif
}
