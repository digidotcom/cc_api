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
    ccapi_sms_stop_t sms_stop = {0};

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_NOT_STARTED, sms_stop_error);
}

TEST(test_ccapi_sms_stop, testSMSNotStarted)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {0};

    th_start_ccapi();

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_NOT_STARTED, sms_stop_error);
}

TEST(test_ccapi_sms_stop, testSmsStopOK)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {0};

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
    ccapi_sms_stop_t sms_stop = {0};

    th_start_ccapi();
    th_start_sms();

    connector_initiate_stop_request_t stop_data = {connector_transport_sms, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_device_error);

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_CCFSM, sms_stop_error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_sms.started);
}

TEST(test_ccapi_sms_stop, testSMSStopRX_Timeout)
{
    ccapi_sms_stop_error_t sms_stop_error;
    ccapi_sms_stop_t sms_stop = {0};

    th_start_ccapi();
    th_start_sms();

    sms_stop.timeout = 10;

    connector_initiate_stop_request_t stop_data = {connector_transport_sms, connector_wait_sessions_complete, NULL};
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_stop, &stop_data, connector_success);

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_transport_stop_info.stop_transport = CCAPI_FALSE;
    }

    Mock_ccimp_os_get_system_time_return(0);
    Mock_ccimp_os_get_system_time_return(10);
    Mock_ccimp_os_get_system_time_return(15);

    sms_stop_error = ccapi_stop_transport_sms(&sms_stop);
    CHECK_EQUAL(CCAPI_SMS_STOP_ERROR_TIMEOUT, sms_stop_error);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_sms.started);
}
