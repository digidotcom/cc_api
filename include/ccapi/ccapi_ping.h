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

#ifndef _CCAPI_PING_H_
#define _CCAPI_PING_H_

#define CCAPI_SEND_PING_WAIT_FOREVER ((unsigned long) 0)

typedef enum {
    CCAPI_PING_ERROR_NONE,
    CCAPI_PING_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_PING_ERROR_TRANSPORT_NOT_VALID,
    CCAPI_PING_ERROR_TRANSPORT_NOT_STARTED,
    CCAPI_PING_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_PING_ERROR_LOCK_FAILED,
    CCAPI_PING_ERROR_INITIATE_ACTION_FAILED,
    CCAPI_PING_ERROR_RESPONSE_CANCEL,
    CCAPI_PING_ERROR_RESPONSE_TIMEOUT,
    CCAPI_PING_ERROR_RESPONSE_ERROR
} ccapi_ping_error_t;

ccapi_ping_error_t ccapi_send_ping(ccapi_transport_t const transport);
ccapi_ping_error_t ccapi_send_ping_with_reply(ccapi_transport_t const transport, unsigned long const timeout);

#endif
