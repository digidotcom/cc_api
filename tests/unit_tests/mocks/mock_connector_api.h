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
}

#define MOCK_CONNECTOR_INIT_ENABLED 1
#define MOCK_CONNECTOR_RUN_ENABLED 1

#define ASSERT_WAIT(timeout_sec)            { ccimp_os_system_up_time_t system_up_time; \
                                              unsigned long time_end; \
                                              ccimp_os_get_system_time(&system_up_time); \
                                              time_end= (system_up_time.sys_uptime + timeout_sec); \
                                              do \
                                              { \
                                                  ccimp_os_get_system_time(&system_up_time); \
                                              } while (assert_buffer == NULL && system_up_time.sys_uptime <= time_end); \
                                            }
#define ASSERT_IF_NOT_HIT_DO(label, file, function, code) \
                       ON_FALSE_DO_(assert_buffer != NULL && \
                                    (!strcmp(assert_buffer, label) && (!strcmp(assert_file, file)) && (!strcmp(assert_function, function))) \
                                    , {printf("Didn't hit assert: %s\n", label); code;})
#define ASSERT_CLEAN()                      assert_buffer = NULL


void Mock_connector_init_create(void);
void Mock_connector_init_destroy(void);
void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval, void * const context);

void Mock_connector_run_create(void);
void Mock_connector_run_destroy(void);
void Mock_connector_run_returnInNextLoop(connector_status_t retval);

void Mock_connector_initiate_action_create(void);
void Mock_connector_initiate_action_destroy(void);
void Mock_connector_initiate_action_expectAndReturn(connector_handle_t handle, connector_initiate_request_t request, void * request_data, connector_status_t retval);

#endif /* MOCK_CONNECTOR_API_H_ */
