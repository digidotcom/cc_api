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

#define TEST_COMMAND "help"
#define TEST_COMMAND_SIZE (sizeof(TEST_COMMAND))

static ccapi_transport_t ccapi_cli_request_expected_transport;
static char const * ccapi_cli_request_expected_command;
static ccapi_bool_t ccapi_cli_request_expected_output_null;
static ccapi_bool_t ccapi_cli_request_cb_called;

static void test_cli_request_cb(ccapi_transport_t const transport, char const * const command, char const * * const output)
{
    CHECK_EQUAL(ccapi_cli_request_expected_transport, transport);
    STRCMP_EQUAL(ccapi_cli_request_expected_command, command);
    CHECK(ccapi_cli_request_expected_output_null ? output == NULL : output != NULL);

    ccapi_cli_request_cb_called = CCAPI_TRUE;

    return;
}

TEST_GROUP(test_ccapi_cli_request_callback_NoCliSupport)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);

        ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;
        ccapi_cli_request_expected_command = NULL;
        ccapi_cli_request_expected_output_null = CCAPI_TRUE;
        ccapi_cli_request_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_cli_request_callback_NoCliSupport, testNoCliSupport)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = TEST_COMMAND;
    ccfsm_cli_request.bytes_used = TEST_COMMAND_SIZE;
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NO_CLI_SUPPORT);
    }

    CHECK_EQUAL(CCAPI_FALSE, ccapi_cli_request_cb_called);
}

TEST_GROUP(test_ccapi_cli_request_callback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_cli_service_t cli_service = {test_cli_request_cb, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.cli = &cli_service;

        ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;
        ccapi_cli_request_expected_command = NULL;
        ccapi_cli_request_expected_output_null = CCAPI_TRUE;
        ccapi_cli_request_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(cli_service.request_cb, ccapi_data_single_instance->service.cli.user_callbacks.request_cb);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_cli_request_callback, testRequestEmpty)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;

    #define REQUEST_EMPTY ""

    ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;    
    ccapi_cli_request_expected_command = REQUEST_EMPTY;
    ccapi_cli_request_expected_output_null = CCAPI_TRUE;
    ccapi_cli_request_cb_called = CCAPI_FALSE;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = REQUEST_EMPTY;
    ccfsm_cli_request.bytes_used = sizeof(REQUEST_EMPTY);
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NONE);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_cli_request_cb_called);
}

/* Cloud does not allow to send a NULL request. test it does not harm anyway */
TEST(test_ccapi_cli_request_callback, testRequestNULL)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;

    #define TARGET_NULL NULL

    ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;    
    ccapi_cli_request_expected_command = TARGET_NULL;
    ccapi_cli_request_expected_output_null = CCAPI_TRUE;
    ccapi_cli_request_cb_called = CCAPI_FALSE;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = TARGET_NULL;
    ccfsm_cli_request.bytes_used = 0;
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    CHECK(ccfsm_cli_request.user_context == NULL);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_cli_request_cb_called);
}

TEST(test_ccapi_cli_request_callback, testOneRequest)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;
    
    ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_cli_request_expected_command = TEST_COMMAND;
    ccapi_cli_request_expected_output_null = CCAPI_TRUE;
    ccapi_cli_request_cb_called = CCAPI_FALSE;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = TEST_COMMAND;
    ccfsm_cli_request.bytes_used = TEST_COMMAND_SIZE;
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NONE);
        CHECK_EQUAL(svc_cli->response_required, CCAPI_FALSE);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_cli_request_cb_called);
}

#define TEST_2_COMMAND "1234567890"
#define TEST_2_COMMAND_SIZE (sizeof(TEST_2_COMMAND))
#define TEST_2_COMMAND_A "12345"
#define TEST_2_COMMAND_A_SIZE 5
#define TEST_2_COMMAND_B "67890"
#define TEST_2_COMMAND_B_SIZE (5 + 1)

TEST(test_ccapi_cli_request_callback, testTwoRequests)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;
    
    ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_cli_request_expected_command = TEST_2_COMMAND;
    ccapi_cli_request_expected_output_null = CCAPI_TRUE;
    ccapi_cli_request_cb_called = CCAPI_FALSE;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = TEST_2_COMMAND_A;
    ccfsm_cli_request.bytes_used = TEST_2_COMMAND_A_SIZE;
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_true;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NONE);
        CHECK_EQUAL(svc_cli->response_required, CCAPI_FALSE);
    }

    CHECK_EQUAL(CCAPI_FALSE, ccapi_cli_request_cb_called);

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = ccfsm_cli_request.user_context;
    ccfsm_cli_request.buffer = TEST_2_COMMAND_B;
    ccfsm_cli_request.bytes_used = TEST_2_COMMAND_B_SIZE;
    ccfsm_cli_request.response_required = connector_false;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NONE);
        CHECK_EQUAL(svc_cli->response_required, CCAPI_FALSE);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_cli_request_cb_called);


}

TEST(test_ccapi_cli_request_callback, testResponseRequired)
{
    connector_request_id_t request;
    connector_sm_cli_request_t ccfsm_cli_request;
    connector_callback_status_t status;
    
    ccapi_cli_request_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_cli_request_expected_command = TEST_COMMAND;
    ccapi_cli_request_expected_output_null = CCAPI_FALSE;
    ccapi_cli_request_cb_called = CCAPI_FALSE;

    ccfsm_cli_request.transport = connector_transport_tcp;
    ccfsm_cli_request.user_context = NULL;
    ccfsm_cli_request.buffer = TEST_COMMAND;
    ccfsm_cli_request.bytes_used = TEST_COMMAND_SIZE;
    ccfsm_cli_request.response_required = connector_true;
    ccfsm_cli_request.more_data = connector_false;

    request.sm_request = connector_request_id_sm_cli_request;
    status = ccapi_connector_callback(connector_class_id_short_message, request, &ccfsm_cli_request, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_cli_request.user_context != NULL);

    {
        ccapi_svc_cli_t * svc_cli = (ccapi_svc_cli_t *)ccfsm_cli_request.user_context;
        CHECK_EQUAL(svc_cli->cli_error, CCAPI_CLI_ERROR_NONE);
        CHECK_EQUAL(svc_cli->response_required, CCAPI_TRUE);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_cli_request_cb_called);
}
