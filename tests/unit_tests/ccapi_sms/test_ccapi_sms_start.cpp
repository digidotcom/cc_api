#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_sms_start)
{
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);
        Mock_destroy_all();
    }
};

static ccapi_bool_t ccapi_sms_close_cb(ccapi_sms_close_cause_t cause)
{
    UNUSED_ARGUMENT(cause);
    return CCAPI_TRUE;
}


TEST(test_ccapi_sms_start, testConnectorInitiateActionOK)
{
    ccapi_sms_start_error_t error;
    ccapi_sms_info_t sms_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    char service_id[] = "";

    sms_start.service_id = service_id;
    sms_start.phone_number = phone_number;
    sms_start.callback.close = ccapi_sms_close_cb;

    connector_transport_t connector_transport = connector_transport_sms;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_NONE, error);
}

TEST(test_ccapi_sms_start, testConnectorInitiateActionInitError)
{
    ccapi_sms_start_error_t error;
    ccapi_sms_info_t sms_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    char service_id[] = "";

    sms_start.service_id = service_id;
    sms_start.phone_number = phone_number;
    connector_transport_t connector_transport = connector_transport_sms;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_init_error);
    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_INIT, error);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_invalid_data);
    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_INIT, error);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_service_busy);
    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_INIT, error);
}

TEST(test_ccapi_sms_start, testConnectorInitiateActionUnknownError)
{
    ccapi_sms_start_error_t error;
    ccapi_sms_info_t sms_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    char service_id[] = "";

    sms_start.service_id = service_id;
    sms_start.phone_number = phone_number;
    connector_transport_t connector_transport = connector_transport_sms;


    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,
            connector_abort);
    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_INIT, error);
}


TEST(test_ccapi_sms_start, testSMSConnectionTimeout)
{
    ccapi_sms_start_error_t error;
    ccapi_sms_info_t sms_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    char service_id[] = "";

    sms_start.service_id = service_id;
    sms_start.phone_number = phone_number;
    connector_transport_t connector_transport = connector_transport_sms;

    sms_start.start_timeout = 10;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_FALSE;
    }

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    Mock_ccimp_os_get_system_time_return(0); /* Start time */
    Mock_ccimp_os_get_system_time_return(5);
    Mock_ccimp_os_get_system_time_return(10);
    Mock_ccimp_os_get_system_time_return(15);

    error = ccapi_start_transport_sms(&sms_start);
    CHECK_EQUAL(CCAPI_SMS_START_ERROR_TIMEOUT, error);
    CHECK_EQUAL(sms_start.start_timeout, ccapi_data_single_instance->transport_sms.info->start_timeout);
}
