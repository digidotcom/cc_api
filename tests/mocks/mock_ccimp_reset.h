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

#ifndef _MOCK_CCIMP_RESET_H_
#define _MOCK_CCIMP_RESET_H_

enum {
    MOCK_RESET_DISABLED,
    MOCK_RESET_ENABLED	/* Mock disabled. Do reset normally */
};

void Mock_ccimp_hal_reset_create(void);
void Mock_ccimp_hal_reset_destroy(void);
void Mock_ccimp_hal_reset_expectAndReturn(void * retval);

extern "C" {
ccimp_status_t ccimp_hal_reset_real(void);

}

#endif
