#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_status_handler_test)
{
    ccapi_data_t * ccapi_data;

    void setup()
    {
        ccapi_start_error_t start_error;
        ccapi_start_t start;

        Mock_create_all();

        fill_start_structure_with_good_parameters(&start);
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);

        ccapi_data = *spy_ccapi_data;
        {
            mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data->connector_handle); 
            mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
            mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
        }
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        Mock_destroy_all();
    }
};

TEST(ccapi_status_handler_test, testStatusTCP)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_communication_started};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_status_handler_test, testStatusStop)
{
    connector_request_id_t request;
    connector_initiate_stop_request_t stop_status = {connector_transport_tcp, connector_stop_immediately, NULL};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_stop_completed;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &stop_status, ccapi_data);
    CHECK_EQUAL(connector_callback_continue, connector_status);
}
