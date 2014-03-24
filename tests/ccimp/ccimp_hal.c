/*
 * ccimp_hal.c
 *
 *  Created on: Mar 27, 2014
 *      Author: hbujanda
 */

#include "ccapi_definitions.h"

#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#ifdef UNIT_TEST
#define ccimp_hal_assertion_hit       ccimp_hal_assertion_hit_real
#define ccimp_hal_reset               ccimp_hal_reset_real

extern char * assert_buffer;
#endif

/******************** LINUX IMPLEMENTATION ********************/

#if (defined CCIMP_DEBUG_ENABLED)
ccimp_status_t ccimp_hal_assertion_hit(char const * const message)
{
#if (defined UNIT_TEST)

    assert_buffer = (char *)message;
#else

    printf("ccimp_hal_assertion_hit: %s!!!!\n", message);

    assert(0);

    /* Should not get here */
#endif

    return CCIMP_STATUS_OK;
}
#endif

ccimp_status_t ccimp_hal_reset(void)
{
    printf("ccimp_hal_reset!!!!\n");

    /* Note: we must be running as the superuser to reboot the system */
    sync();
    reboot(LINUX_REBOOT_CMD_RESTART);

    /* Should not get here */
    return CCIMP_STATUS_OK;
}

