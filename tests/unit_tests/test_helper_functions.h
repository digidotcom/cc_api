/*
 * test_helper_functions.h
 *
 *  Created on: Mar 18, 2014
 *      Author: spastor
 */

#ifndef _TEST_HELPER_FUNCTIONS_H_
#define _TEST_HELPER_FUNCTIONS_H_

extern "C" {
#include "ccapi/ccapi.h"
}

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

void fill_start_structure_with_good_parameters(ccapi_start_t * start);
pthread_t aux_ccapi_start(void * argument);
int stop_aux_thread(pthread_t pthread);

#endif
