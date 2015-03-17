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


/* Device request */
extern void ccapi_device_request_status_callback(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error);
extern void ccapi_device_request_data_callback(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info);
extern ccapi_bool_t ccapi_device_request_accept_callback(char const * const target, ccapi_transport_t const transport);
extern void fill_device_request_service(ccapi_start_t * start);



typedef enum {
    TEST_PUT_REQUEST_SEND_OPTION_APPEND_FLAG,
    TEST_PUT_REQUEST_SEND_OPTION_OVERWRITE_FLAG,
    TEST_DO_NOTHING_FLAG
} test_flag_start_action_t;

extern test_flag_start_action_t device_request_start_test_action;

extern char * test_data_buffer;