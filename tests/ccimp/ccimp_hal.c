/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include "ccimp/ccimp_hal.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#if (defined UNIT_TEST)
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

