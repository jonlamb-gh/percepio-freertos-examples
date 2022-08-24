#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include <stdio.h>
#include <time.h>

#include "ip.h"

/* Define a name that will be used for LLMNR and NBNS searches. */
#define mainHOST_NAME           "m3-qmeu"
#define mainDEVICE_NICK_NAME    "m3-qmeu"

const uint8_t MAC_ADDR[6] = {
    configMAC_ADDR0,
    configMAC_ADDR1,
    configMAC_ADDR2,
    configMAC_ADDR3,
    configMAC_ADDR4,
    configMAC_ADDR5
};

static const uint8_t IP_ADDR[4] = {
    configIP_ADDR0,
    configIP_ADDR1,
    configIP_ADDR2,
    configIP_ADDR3
};

static const uint8_t NET_MASK[4] = {
    configNET_MASK0,
    configNET_MASK1,
    configNET_MASK2,
    configNET_MASK3
};

static const uint8_t GW_ADDR[4] = {
    configGATEWAY_ADDR0,
    configGATEWAY_ADDR1,
    configGATEWAY_ADDR2,
    configGATEWAY_ADDR3
};

static const uint8_t DNS_ADDR[4] = {
    configDNS_SERVER_ADDR0,
    configDNS_SERVER_ADDR1,
    configDNS_SERVER_ADDR2,
    configDNS_SERVER_ADDR3
};

/* Needed by MPS2_AN385/NetworkInterface.c */
const uint8_t* ucMACAddress = MAC_ADDR;

static UBaseType_t g_next_rand = 0;

void ip_init(void)
{
    time_t now;

    printf("hostname: %s\n", mainHOST_NAME);

    /* Seed the random number generator. */
    time(&now);

    printf("Seed for randomiser: %ld\n", (unsigned long) now);
    g_next_rand = (UBaseType_t) now;
    printf("Random numbers: %08lX %08lX %08lX %08lX\n",
            ipconfigRAND32(),
            ipconfigRAND32(),
            ipconfigRAND32(),
            ipconfigRAND32());

    FreeRTOS_IPInit(
            IP_ADDR,
            NET_MASK,
            GW_ADDR,
            DNS_ADDR,
            MAC_ADDR);
}

UBaseType_t uxRand(void)
{
    const uint32_t mult = 0x015a4e35UL, inc = 1UL;

    /* Utility function to generate a pseudo random number. */
    g_next_rand = ( mult * g_next_rand ) + inc;
    return (int) (g_next_rand >> 16UL) & 0x7FFFUL;
}

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
uint32_t ulApplicationGetNextSequenceNumber(
        uint32_t src_addr,
        uint16_t src_port,
        uint32_t dst_addr,
        uint16_t dst_port)
{
    (void) src_addr;
    (void) src_port;
    (void) dst_addr;
    (void) dst_port;

    return uxRand();
}

/*
 * Supply a random number to FreeRTOS+TCP stack.
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER
 * SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
 */
BaseType_t xApplicationGetRandomNumber(uint32_t* num)
{
    *num = uxRand();
    return pdTRUE;
}

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
const char * pcApplicationHostnameHook(void)
{
    /* Assign the name "FreeRTOS" to this network node.  This function will
     * be called during the DHCP: the machine will be registered with an IP
     * address plus this name. */
    return mainHOST_NAME;
}
#endif

#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )
BaseType_t xApplicationDNSQueryHook(const char* name)
{
    BaseType_t xReturn;

    /* Determine if a name lookup is for this node.  Two names are given
     * to this node: that returned by pcApplicationHostnameHook() and that set
     * by mainDEVICE_NICK_NAME. */
    if(strcasecmp(name, pcApplicationHostnameHook()) == 0)
    {
        xReturn = pdPASS;
    }
    else if(strcasecmp(name, mainDEVICE_NICK_NAME) == 0)
    {
        xReturn = pdPASS;
    }
    else
    {
        xReturn = pdFAIL;
    }

    return xReturn;
}
#endif

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t network_event)
{
    uint32_t ip_addr;
    uint32_t net_mask;
    uint32_t gw_addr;
    uint32_t dns_addr;
    char cBuffer[16];

    printf("*******************************\n");
    if(network_event == eNetworkUp)
    {
        FreeRTOS_GetAddressConfiguration(&ip_addr, &net_mask, &gw_addr, &dns_addr);
        FreeRTOS_inet_ntoa(ip_addr, cBuffer);
        printf("IP Address: %s\n", cBuffer);

        FreeRTOS_inet_ntoa(net_mask, cBuffer);
        printf("Subnet Mask: %s\n", cBuffer);

        FreeRTOS_inet_ntoa(gw_addr, cBuffer);
        printf("Gateway Address: %s\n", cBuffer);

        FreeRTOS_inet_ntoa(dns_addr, cBuffer);
        printf("DNS Server Address: %s\n", cBuffer);
    }
    else
    {
        printf("Network down\n");
    }
    printf("*******************************\n");
}
