/*
 * ccxapi_init.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCXAPI_INIT_H_
#define _CCXAPI_INIT_H_

ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start);
ccapi_stop_error_t ccxapi_stop(ccapi_handle_t const ccapi_handle, ccapi_stop_t const behavior);
ccapi_tcp_start_error_t ccxapi_start_transport_tcp(ccapi_handle_t const ccapi_handle, ccapi_tcp_info_t const * const tcp_start);

#endif
