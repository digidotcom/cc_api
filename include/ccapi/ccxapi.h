/*
 * ccxapi.h
 *
 *  Created on: Mar 31, 2014
 *      Author: hbujanda
 */

#ifndef _CCXAPI_H_
#define _CCXAPI_H_

/* 
 * Cloud Connector Multi-instance API
 *
 * Don't distribute this file. It's only for internal usage.
 *
 * This header is the multi-instance variant of ccapi.h
 * include "ccapi/ccxapi.h" instead of "ccapi/ccapi.h" from your applications if you pretend to start several 
 * instances of Cloud Connector.
 * Then use the ccxapi_* functions declared below instead of the ccapi_* functions declared in "ccapi/ccapi.h"
 */

#include "ccapi/ccapi.h"

typedef struct ccapi_handle * ccapi_handle_t;

ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start);
ccapi_stop_error_t ccxapi_stop(ccapi_handle_t const ccapi_handle, ccapi_stop_t const behavior);

ccapi_tcp_start_error_t ccxapi_start_transport_tcp(ccapi_handle_t const ccapi_handle, ccapi_tcp_info_t const * const tcp_start);
ccapi_tcp_stop_error_t ccxapi_stop_transport_tcp(ccapi_handle_t const ccapi_handle, ccapi_tcp_stop_t const * const tcp_stop);

#ifdef CCIMP_DATA_SERVICE_ENABLED
ccapi_send_error_t ccxapi_send_data(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior);
ccapi_send_error_t ccxapi_send_data_with_reply(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint);
ccapi_send_error_t ccxapi_send_file(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior);
ccapi_send_error_t ccxapi_send_file_with_reply(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint);
#endif

ccapi_fs_error_t ccxapi_fs_add_virtual_dir(ccapi_handle_t const ccapi_handle, char const * const virtual_dir, char const * const actual_dir);
ccapi_fs_error_t ccxapi_fs_remove_virtual_dir(ccapi_handle_t const ccapi_data, char const * const virtual_dir);

#endif
