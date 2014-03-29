/*
 * ccimp_hal.c
 *
 *  Created on: Mar 27, 2014
 *      Author: hbujanda
 */
#include "ccimp/ccimp_hal.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#ifdef UNIT_TEST
#define ccimp_hal_halt       ccimp_hal_halt_real
#define ccimp_hal_reset      ccimp_hal_reset_real
#endif

/******************** LINUX IMPLEMENTATION ********************/

#if (defined CCIMP_DEBUG_ENABLED)
ccimp_status_t ccimp_hal_halt(void)
{
    printf("ccimp_hal_halt!!!!\n");

    assert(0);

    /* Should not get here */

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

