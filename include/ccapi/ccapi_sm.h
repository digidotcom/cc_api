/*
* Copyright (c) 2017 Digi International Inc.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
* OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
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
