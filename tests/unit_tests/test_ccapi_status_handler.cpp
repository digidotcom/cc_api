#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_bool_t ccapi_tcp_keepalives_cb_called;
static ccapi_keepalive_status_t ccapi_tcp_keepalives_cb_argument;

static void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    ccapi_tcp_keepalives_cb_called = CCAPI_TRUE;
    ccapi_tcp_keepalives_cb_argument = status;
    return;
}

TEST_GROUP(ccapi_status_handler_test)
{
    ccapi_data_t * ccapi_data;

    void setup()
    {
        ccapi_start_error_t start_error;
        ccapi_start_t start;
        ccapi_tcp_start_error_t error;
        ccapi_tcp_info_t tcp_start = {{0}};
        char phone_number[] = "+54-3644-421921";
        uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */

        Mock_create_all();

        fill_start_structure_with_good_parameters(&start);
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);

        ccapi_data = ccapi_data_single_instance;
        {
            mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data->connector_handle); 
            mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
            mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
        }

        tcp_start.connection.type = CCAPI_CONNECTION_WAN;
        tcp_start.connection.info.wan.phone_number = phone_number;
        tcp_start.connection.info.wan.link_speed = 115200;
        memcpy(tcp_start.connection.ip.address.ipv4, ipv4, sizeof tcp_start.connection.ip.address.ipv4);

        ccapi_tcp_keepalives_cb_called = CCAPI_FALSE;
        tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

        connector_transport_t connector_transport = connector_transport_tcp;
        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

        error = ccapi_start_transport_tcp(&tcp_start);
        CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
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

TEST(ccapi_status_handler_test, testStatusTCPStarted)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_communication_started};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_status_handler_test, testStatusTCPKeepaliveMissed)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_missed};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_keepalives_cb_called);
    CHECK_EQUAL(CCAPI_KEEPALIVE_MISSED, ccapi_tcp_keepalives_cb_argument);
}

TEST(ccapi_status_handler_test, testStatusTCPKeepaliveRestored)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_restored};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data->transport_tcp.connected);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_keepalives_cb_called);
    CHECK_EQUAL(CCAPI_KEEPALIVE_RESTORED, ccapi_tcp_keepalives_cb_argument);
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


TEST_GROUP(ccapi_status_handler_test_no_callbacks)
{
    /* Checks that no segmentation fault is thrown because of calling a NULL function pointer */
    mock_connector_api_info_t * mock_info;

    void setup()
    {
        ccapi_start_error_t start_error;
        ccapi_start_t start;
        ccapi_tcp_start_error_t error;
        ccapi_tcp_info_t tcp_start = {{0}};
        char phone_number[] = "+54-3644-421921";
        uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */

        Mock_create_all();

        fill_start_structure_with_good_parameters(&start);
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);

        {
            mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
            mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data_single_instance;
            mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
        }

        tcp_start.connection.type = CCAPI_CONNECTION_WAN;
        tcp_start.connection.info.wan.phone_number = phone_number;
        tcp_start.connection.info.wan.link_speed = 115200;
        memcpy(tcp_start.connection.ip.address.ipv4, ipv4, sizeof tcp_start.connection.ip.address.ipv4);

        ccapi_tcp_keepalives_cb_called = CCAPI_FALSE;
        tcp_start.callback.close = NULL;
        tcp_start.callback.keepalive = NULL;

        connector_transport_t connector_transport = connector_transport_tcp;
        Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

        error = ccapi_start_transport_tcp(&tcp_start);
        CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        Mock_destroy_all();
    }
};

TEST(ccapi_status_handler_test_no_callbacks, testStatusTCPKeepaliveMissedNoCallback)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_missed};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_keepalives_cb_called);
}

TEST(ccapi_status_handler_test_no_callbacks, testStatusTCPKeepaliveRestoredNoCallback)
{
    connector_request_id_t request;
    connector_status_tcp_event_t tcp_status = {connector_tcp_keepalive_restored};
    connector_callback_status_t connector_status;

    request.status_request = connector_request_id_status_tcp;
    connector_status = ccapi_connector_callback(connector_class_id_status, request, &tcp_status, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, connector_status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->transport_tcp.connected);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_keepalives_cb_called);
}
