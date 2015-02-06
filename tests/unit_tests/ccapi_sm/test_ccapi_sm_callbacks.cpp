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

static ccapi_transport_t ccapi_sm_request_connect_expected_transport;
static ccapi_bool_t ccapi_sm_request_connect_cb_called;

static void test_sm_request_connect_cb(ccapi_transport_t const transport)
{
    CHECK_EQUAL(ccapi_sm_request_connect_expected_transport, transport);

    ccapi_sm_request_connect_cb_called = CCAPI_TRUE;

    return;
}

TEST_GROUP(test_ccapi_sm_callback_NoSmSupport)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);

        ccapi_sm_request_connect_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_sm_callback_NoSmSupport, testNoRequestConnectCallback)
{
    connector_request_id_t request;
    connector_sm_request_connect_t ccfsm_sm_request_connect;
    connector_callback_status_t status;

    ccfsm_sm_request_connect.transport = connector_transport_udp;
    ccfsm_sm_request_connect.allow = connector_false;

    request.sm_request = connector_request_id_sm_request_connect;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_sm_request_connect, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_sm_request_connect_cb_called);
}

TEST_GROUP(test_ccapi_sm_request_connect_callback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_sm_service_t sm_service = {test_sm_request_connect_cb, NULL, NULL, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.sm = &sm_service;

        ccapi_sm_request_connect_expected_transport = CCAPI_TRANSPORT_UDP;
        ccapi_sm_request_connect_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(sm_service.request_connect, ccapi_data_single_instance->service.sm.user_callback.request_connect);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
        CHECK(ccapi_data_single_instance == NULL);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_sm_request_connect_callback, testRequestConnect)
{
    connector_request_id_t request;
    connector_sm_request_connect_t ccfsm_sm_request_connect;
    connector_callback_status_t status;

    ccapi_sm_request_connect_expected_transport = CCAPI_TRANSPORT_UDP;    
    ccapi_sm_request_connect_cb_called = CCAPI_FALSE;

    ccfsm_sm_request_connect.transport = connector_transport_udp;
    ccfsm_sm_request_connect.allow = connector_false;

    request.sm_request = connector_request_id_sm_request_connect;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_sm_request_connect, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_sm_request_connect_cb_called);
}

