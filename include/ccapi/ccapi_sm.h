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

#ifndef _CCAPI_SM_H_
#define _CCAPI_SM_H_

typedef void (*ccapi_sm_request_connect_cb_t)(ccapi_transport_t const transport);
typedef void (*ccapi_sm_ping_request_cb_t)(ccapi_transport_t const transport, ccapi_bool_t const response_required);
typedef void (*ccapi_sm_unsequenced_response_cb_t)(ccapi_transport_t const transport, uint32_t const id, void const * const data, size_t const bytes_used, ccapi_bool_t error);
typedef void (*ccapi_sm_pending_data_cb_t)(ccapi_transport_t const transport);
typedef void (*ccapi_sm_phone_provisioning_cb_t)(ccapi_transport_t const transport, char const * const phone_number, char const * const service_id, ccapi_bool_t const response_required);

typedef struct {
    ccapi_sm_request_connect_cb_t request_connect;
    ccapi_sm_ping_request_cb_t ping_request;
    ccapi_sm_unsequenced_response_cb_t unsequenced_response;
    ccapi_sm_pending_data_cb_t pending_data;
    ccapi_sm_phone_provisioning_cb_t phone_provisioning;
} ccapi_sm_service_t;

#endif
