#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_tcp_stop)
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

TEST(test_ccapi_tcp_stop, testCCAPINotStarted)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {CCAPI_STOP_GRACEFULLY};

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NOT_STARTED, tcp_stop_error);
}

TEST(test_ccapi_tcp_stop, testTCPNotStarted)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {CCAPI_STOP_GRACEFULLY};

    th_start_ccapi();

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NOT_STARTED, tcp_stop_error);
}

TEST(test_ccapi_tcp_stop, testTCPStopGracefullyOK)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {CCAPI_STOP_GRACEFULLY};

    th_start_ccapi();
    th_start_tcp_wan_ipv4_with_callbacks();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NONE, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(test_ccapi_tcp_stop, testTCPStopImmediatelyOK)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {CCAPI_STOP_IMMEDIATELY};

    th_start_ccapi();
    th_start_tcp_wan_ipv4_with_callbacks();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_stop_immediately, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NONE, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(test_ccapi_tcp_stop, testTCPStopCcfsmError)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {CCAPI_STOP_GRACEFULLY};

    th_start_ccapi();
    th_start_tcp_wan_ipv4_with_callbacks();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_CCFSM, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}
