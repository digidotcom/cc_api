/*
 * test_ccapi_stop.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: spastor
 */

#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

static ccapi_bool_t ccapi_tcp_close_cb(ccapi_tcp_close_cause_t cause)
{
    UNUSED_ARGUMENT(cause);
    return CCAPI_TRUE;
}

static void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    UNUSED_ARGUMENT(status);
    return;
}

mock_connector_api_info_t * mock_info;

static void start_ccapi(void)
{
    ccapi_start_error_t start_error;
    ccapi_start_t start;

    Mock_create_all();

    fill_start_structure_with_good_parameters(&start);
    start_error = ccapi_start(&start);
    CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);

    {
        mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data_single_instance;
        mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
    }
}

static void start_tcp(void)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;
    memcpy(tcp_start.connection.ip.address.ipv4, ipv4, sizeof tcp_start.connection.ip.address.ipv4);

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
}

TEST_GROUP(ccapi_tcp_stop)
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

TEST(ccapi_tcp_stop, testCCAPINotStarted)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NOT_STARTED, tcp_stop_error);
}

TEST(ccapi_tcp_stop, testTCPNotStarted)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    start_ccapi();
    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NOT_STARTED, tcp_stop_error);
}

TEST(ccapi_tcp_stop, testTCPStopOK)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    start_ccapi();
    start_tcp();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_TRUE;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NONE, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(ccapi_tcp_stop, testTCPStopCcfsmError)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    start_ccapi();
    start_tcp();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_TRUE;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_CCFSM, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(ccapi_tcp_stop, testTCPStopTimeout)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    start_ccapi();
    start_tcp();

    tcp_stop.timeout = 10;

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_FALSE;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    Mock_ccimp_os_get_system_time_return(0);
    Mock_ccimp_os_get_system_time_return(10);
    Mock_ccimp_os_get_system_time_return(15);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_TIMEOUT, tcp_stop_error);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_tcp.connected);
}
