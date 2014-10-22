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

#ifndef _CCAPI_CLI_H_
#define _CCAPI_CLI_H_

typedef enum {
    CCAPI_CLI_ERROR_NONE,
    CCAPI_CLI_ERROR_NO_CLI_SUPPORT,
    CCAPI_CLI_ERROR_INSUFFICIENT_MEMORY
} ccapi_cli_error_t;

typedef void (*ccapi_cli_request_cb_t)(ccapi_transport_t const transport, char const * const command, char const * * const output);
typedef void (*ccapi_cli_finished_cb_t)(char const * * const output);


typedef struct {
    ccapi_cli_request_cb_t request_cb;
    ccapi_cli_finished_cb_t finished_cb;
} ccapi_cli_service_t;

#endif
