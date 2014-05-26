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

TEST_GROUP(test_ccapi_sms_stop)
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

TEST(test_ccapi_sms_stop, testCCAPINotStarted)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {CCAPI_TRANSPORT_STOP_GRACEFULLY};

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_NOT_STARTED, sms_stop_error);
}

TEST(test_ccapi_sms_stop, testSMSNotStarted)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {CCAPI_TRANSPORT_STOP_GRACEFULLY};

    th_start_ccapi();

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_NOT_STARTED, sms_stop_error);
}

TEST(test_ccapi_sms_stop, testSmsStopOK)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {CCAPI_TRANSPORT_STOP_GRACEFULLY};

    th_start_ccapi();
    th_start_sms();

    connector_initiate_stop_request_t stop_data = {connector_transport_sms, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_NONE, sms_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_sms.started);
}

TEST(test_ccapi_sms_stop, testSMSStopCcfsmError)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {CCAPI_TRANSPORT_STOP_IMMEDIATELY};

    th_start_ccapi();
    th_start_sms();

    connector_initiate_stop_request_t stop_data = {connector_transport_sms, connector_stop_immediately, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_CCFSM, sms_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_sms.started);
}
