/*
 * test_ccapi_stop.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: spastor
 */

#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

#include "test_helper_functions.h"

TEST_GROUP(ccapi_stop_test)
{
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(ccapi_stop_test, testCcapiNotStarted)
{
    ccapi_stop_error_t error;

    error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(error == CCAPI_STOP_ERROR_NOT_STARTED);

    error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(error == CCAPI_STOP_ERROR_NOT_STARTED);
}

TEST(ccapi_stop_test, testCcapiStartedBadly)
{
    ccapi_stop_error_t stop_error;
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    /*Mock_ccimp_free_expectAndReturn(NULL, CCIMP_STATUS_OK);*/
    fill_start_structure_with_good_parameters(&start);
    start.vendor_id = 0;
    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_INVALID_VENDORID);

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NOT_STARTED);
}

TEST(ccapi_stop_test, testCcapiStopGracefully)
{
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;

    ccapi_stop_error_t stop_error;
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    fill_start_structure_with_good_parameters(&start);

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);

    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_NONE);

    Mock_ccimp_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_device_cloud_url, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(&mem_for_thread_connector_run, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
}

TEST(ccapi_stop_test, testCcapiStopImmediately)
{
    ccapi_stop_error_t stop_error;
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    fill_start_structure_with_good_parameters(&start);

    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_NONE);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
}
