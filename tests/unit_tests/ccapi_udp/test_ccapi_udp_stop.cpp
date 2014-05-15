#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_udp_stop)
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

TEST(test_ccapi_udp_stop, testCCAPINotStarted)
{
    ccapi_udp_stop_error_t udp_stop_error;
    ccapi_udp_stop_t udp_stop = {0};

    udp_stop_error = ccapi_stop_transport_udp(&udp_stop);
    CHECK_EQUAL(CCAPI_UDP_STOP_ERROR_NOT_STARTED, udp_stop_error);
}

TEST(test_ccapi_udp_stop, testUDPNotStarted)
{
    ccapi_udp_stop_error_t udp_stop_error;
    ccapi_udp_stop_t udp_stop = {0};

    th_start_ccapi();

    udp_stop_error = ccapi_stop_transport_udp(&udp_stop);
    CHECK_EQUAL(CCAPI_UDP_STOP_ERROR_NOT_STARTED, udp_stop_error);
}

TEST(test_ccapi_udp_stop, testUDPStopOK)
{
    ccapi_udp_stop_error_t udp_stop_error;
    ccapi_udp_stop_t udp_stop = {0};

    th_start_ccapi();
    th_start_udp();

    connector_initiate_stop_request_t stop_data = {connector_transport_udp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    udp_stop_error = ccapi_stop_transport_udp(&udp_stop);
    CHECK_EQUAL(CCAPI_UDP_STOP_ERROR_NONE, udp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_udp.started);
}

TEST(test_ccapi_udp_stop, testUDPStopCcfsmError)
{
    ccapi_udp_stop_error_t udp_stop_error;
    ccapi_udp_stop_t udp_stop = {0};

    th_start_ccapi();
    th_start_udp();

    connector_initiate_stop_request_t stop_data = {connector_transport_udp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    udp_stop_error = ccapi_stop_transport_udp(&udp_stop);
    CHECK_EQUAL(CCAPI_UDP_STOP_ERROR_CCFSM, udp_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_udp.started);
}

TEST(test_ccapi_udp_stop, testUDPStopRX_Timeout)
{
    ccapi_udp_stop_error_t udp_stop_error;
    ccapi_udp_stop_t udp_stop = {0};

    th_start_ccapi();
    th_start_udp();

    udp_stop.timeout = 10;

    connector_initiate_stop_request_t stop_data = {connector_transport_udp, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_FALSE;
    }

    Mock_ccimp_os_get_system_time_return(0);
    Mock_ccimp_os_get_system_time_return(10);
    Mock_ccimp_os_get_system_time_return(15);

    udp_stop_error = ccapi_stop_transport_udp(&udp_stop);
    CHECK_EQUAL(CCAPI_UDP_STOP_ERROR_TIMEOUT, udp_stop_error);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_udp.started);
}
