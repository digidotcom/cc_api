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
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

void th_fill_start_structure_with_good_parameters(ccapi_start_t * start);
void th_start_ccapi(void);
void th_start_tcp_wan_ipv4(void);
mock_connector_api_info_t * th_setup_mock_info_single_instance(void);
void th_stop_ccapi(ccapi_data_t * const ccapi_data);
pthread_t th_aux_ccapi_start(void * argument);
int th_stop_aux_thread(pthread_t pthread);

#endif
