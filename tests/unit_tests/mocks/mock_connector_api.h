/*
 * Mock_connector_api.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CONNECTOR_API_H_
#define _MOCK_CONNECTOR_API_H_

#include "CppUTestExt/MockSupport.h"
#include "CppUTest/CommandLineTestRunner.h"

extern "C" {
#include "connector_api.h"
#include "ccxapi/ccxapi.h"
}

#define MOCK_CONNECTOR_INIT_ENABLED 1
#define MOCK_CONNECTOR_RUN_ENABLED 1

#define WAIT_FOR_ASSERT()   {do ccimp_os_yield(); while (assert_buffer == NULL);}

#define ASSERT_IF_NOT_HIT_DO(label, file, function, code) \
                       ON_FALSE_DO_(assert_buffer != NULL && \
                                    (!strcmp(assert_buffer, label) && (!strcmp(assert_file, file))) \
                                    , {printf("Didn't hit assert: %s\n", label); code;})
#define ASSERT_CLEAN()                      assert_buffer = NULL

#define MAX_INFO 10

typedef struct  {
    connector_bool_t used;
    ccapi_handle_t ccapi_handle;
    connector_handle_t connector_handle;
    connector_status_t connector_run_retval;
    struct {
        ccapi_bool_t init_transport;
    } connector_initiate_transport_start_info;
} mock_connector_api_info_t;

mock_connector_api_info_t * mock_connector_api_info_get(connector_handle_t connector_handle);

void Mock_connector_init_create(void);
void Mock_connector_init_destroy(void);
void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval, void * const context);

void Mock_connector_run_create(void);
void Mock_connector_run_destroy(void);

void Mock_connector_initiate_action_create(void);
void Mock_connector_initiate_action_destroy(void);
void Mock_connector_initiate_action_expectAndReturn(connector_handle_t handle, connector_initiate_request_t request, void * request_data, connector_status_t retval);

#endif /* MOCK_CONNECTOR_API_H_ */
