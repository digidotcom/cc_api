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

#ifndef _CCAPI_TRANSPORT_H_
#define _CCAPI_TRANSPORT_H_

typedef enum {
    CCAPI_TRANSPORT_TCP,
    CCAPI_TRANSPORT_UDP,
    CCAPI_TRANSPORT_SMS
} ccapi_transport_t;

typedef enum {
    CCAPI_TRANSPORT_STOP_GRACEFULLY,
    CCAPI_TRANSPORT_STOP_IMMEDIATELY
} ccapi_transport_stop_t;

#include "ccapi/ccapi_transport_tcp.h"
#include "ccapi/ccapi_transport_udp.h"
#include "ccapi/ccapi_transport_sms.h"

#endif
