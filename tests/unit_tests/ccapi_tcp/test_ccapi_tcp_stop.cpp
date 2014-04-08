#include "test_helper_functions.h"

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

    th_start_ccapi();
    th_setup_mock_info_single_instance();

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NOT_STARTED, tcp_stop_error);
}

TEST(ccapi_tcp_stop, testTCPStopOK)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    th_start_ccapi();
    th_setup_mock_info_single_instance();
    th_start_tcp_wan_ipv4_with_callbacks();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_NONE, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(ccapi_tcp_stop, testTCPStopCcfsmError)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};

    th_start_ccapi();
    th_setup_mock_info_single_instance();
    th_start_tcp_wan_ipv4_with_callbacks();

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_CCFSM, tcp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(ccapi_tcp_stop, testTCPStopTimeout)
{
    ccapi_tcp_stop_error_t tcp_stop_error;
    ccapi_tcp_stop_t tcp_stop = {0};
    mock_connector_api_info_t * mock_info;

    th_start_ccapi();
    mock_info = th_setup_mock_info_single_instance();
    th_start_tcp_wan_ipv4_with_callbacks();

    tcp_stop.timeout = 10;

    connector_initiate_stop_request_t stop_data = {connector_transport_tcp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);
    mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_FALSE;

    Mock_ccimp_os_get_system_time_return(0);
    Mock_ccimp_os_get_system_time_return(10);
    Mock_ccimp_os_get_system_time_return(15);

    tcp_stop_error = ccapi_stop_transport_tcp(&tcp_stop);
    CHECK_EQUAL(CCAPI_TCP_STOP_ERROR_TIMEOUT, tcp_stop_error);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_tcp.connected);
}
