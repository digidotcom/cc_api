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

#ifndef _CCIMP_HAL_H_
#define _CCIMP_HAL_H_

#include "ccimp/ccimp_types.h"

#if (defined CCIMP_DEBUG_ENABLED)
ccimp_status_t ccimp_hal_halt(void);
#endif

ccimp_status_t ccimp_hal_reset(void);

#endif
