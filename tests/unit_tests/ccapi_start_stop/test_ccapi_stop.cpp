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

#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_stop)
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

TEST(test_ccapi_stop, testCcapiNotStarted)
{
    ccapi_stop_error_t stop_error;

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NOT_STARTED);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NOT_STARTED);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccapi_stop, testCcapiStartedBadly)
{
    ccapi_stop_error_t stop_error;
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    th_fill_start_structure_with_good_parameters(&start);
    start.vendor_id = 0;
    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_INVALID_VENDORID);

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NOT_STARTED);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccapi_stop, testCcapiStopGracefully)
{
    ccapi_stop_error_t stop_error;

    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof TH_DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof TH_DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;


    Mock_ccimp_os_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_os_malloc_expectAndReturn(sizeof(TH_DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_os_malloc_expectAndReturn(sizeof(TH_DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);

    th_start_ccapi();

    Mock_ccimp_os_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_device_cloud_url, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(&mem_for_thread_connector_run, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccapi_stop, testCcapiStopImmediately)
{
    ccapi_stop_error_t stop_error;

    th_start_ccapi();
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccapi_stop, testCcapiStopTCPOk)
{
    ccapi_stop_error_t stop_error;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_stop_immediately, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccapi_stop, testCcapiStopTCPFail)
{
    ccapi_stop_error_t stop_error;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_stop_immediately, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}
