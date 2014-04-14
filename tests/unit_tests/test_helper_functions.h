/*
 * test_helper_functions.h
 *
 *  Created on: Mar 18, 2014
 *      Author: spastor
 */

#ifndef _TEST_HELPER_FUNCTIONS_H_
#define _TEST_HELPER_FUNCTIONS_H_

#define CCAPI_CONST_PROTECTION_UNLOCK

#define TH_DEVICE_TYPE_STRING      "Device type"
#define TH_DEVICE_CLOUD_URL_STRING "login.etherios.com"

#include "CppUTest/CommandLineTestRunner.h"
#include "mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

extern ccapi_bool_t ccapi_tcp_keepalives_cb_called;
extern ccapi_keepalive_status_t ccapi_tcp_keepalives_cb_argument;
extern ccapi_bool_t ccapi_tcp_close_cb_called;
extern ccapi_tcp_close_cause_t ccapi_tcp_close_cb_argument;

void th_fill_start_structure_with_good_parameters(ccapi_start_t * start);
void th_start_ccapi(void);
void th_fill_tcp_wan_ipv4_callbacks_info(ccapi_tcp_info_t * tcp_start);
void th_fill_tcp_lan_ipv4(ccapi_tcp_info_t * tcp_start);

void th_start_tcp_wan_ipv4_with_callbacks(void);
void th_start_tcp_lan_ipv4(void);
void th_start_tcp_lan_ipv6_password_keepalives(void);
void th_stop_ccapi(ccapi_data_t * const ccapi_data);
pthread_t th_aux_ccapi_start(void * argument);
int th_stop_aux_thread(pthread_t pthread);

#endif
