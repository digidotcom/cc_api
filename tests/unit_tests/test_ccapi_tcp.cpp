#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_tcp_test)
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

TEST(ccapi_tcp_test, testTcpOpen)
{
    connector_request_id_t request;
    connector_network_open_t connector_open_data = {{"login.etherios.com"}, NULL};
    connector_callback_status_t status;
    ccimp_network_open_t ccimp_open_data = {{"login.etherios.com"}, NULL};

    Mock_ccimp_network_tcp_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_open;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_open_data, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(ccapi_tcp_test, testTcpSend)
{
    connector_network_handle_t handle = &handle;
    uint8_t buffer[] = "Binary buffer";
    size_t bytes_available = sizeof buffer;
    size_t bytes_used = 0;
    connector_request_id_t request;
    connector_network_send_t connector_send_data = {handle, buffer, bytes_available, bytes_used};
    ccimp_network_send_t ccimp_send_data = {handle, buffer, bytes_available, bytes_used};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_send_expectAndReturn(&ccimp_send_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_send;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_send_data, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(ccapi_tcp_test, testTcpReceive)
{
    connector_network_handle_t handle = &handle;
    uint8_t buffer[1024] = {0};
    size_t bytes_available = sizeof buffer;
    size_t bytes_used = 0;
    connector_request_id_t request;
    connector_network_receive_t connector_receive_data = {handle, buffer, bytes_available, bytes_used};
    ccimp_network_receive_t ccimp_receive_data = {handle, buffer, bytes_available, bytes_used};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_receive_expectAndReturn(&ccimp_receive_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_receive;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_receive_data, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, status);
}

static ccapi_bool_t ccapi_tcp_close_cb_called;
static ccapi_tcp_close_cause_t ccapi_tcp_close_cb_argument;

static ccapi_bool_t ccapi_tcp_close_cb(ccapi_tcp_close_cause_t cause)
{
    ccapi_bool_t reconnect;
    ccapi_tcp_close_cb_argument = cause;
    ccapi_tcp_close_cb_called = CCAPI_TRUE;
    switch (cause)
    {
        case CCAPI_TCP_CLOSE_DISCONNECTED:
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_REDIRECTED:
            reconnect = CCAPI_FALSE;
            break;
        case CCAPI_TCP_CLOSE_NO_KEEPALIVE:
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_DATA_ERROR:
            reconnect = CCAPI_FALSE;
            break;
    }
    return reconnect;
}

TEST_GROUP(ccapi_tcp_test_close)
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

        ccapi_data = *spy_ccapi_data;
        {
            mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data->connector_handle);
            mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
            mock_info->connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
        }

        tcp_start.connection.type = CCAPI_CONNECTION_WAN;
        tcp_start.connection.info.wan.phone_number = phone_number;
        tcp_start.connection.info.wan.link_speed = 115200;
        memcpy(tcp_start.connection.ip.address.ipv4, ipv4, sizeof tcp_start.connection.ip.address.ipv4);

        ccapi_tcp_close_cb_called = CCAPI_FALSE;
        tcp_start.callback.close = ccapi_tcp_close_cb;

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

TEST(ccapi_tcp_test_close, testTcpCloseCallbackCloudDisconnected)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_cloud_disconnected, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_DISCONNECTED, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_true, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseCallbackCloudRedirected)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_cloud_redirected, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_REDIRECTED, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseCallbackNoKeepalive)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_no_keepalive, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_NO_KEEPALIVE, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_true, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseDeviceError)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_device_error, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_DATA_ERROR, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseNoCallback)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_cloud_disconnected, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    ccapi_data->transport_tcp.info->callback.close = NULL;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);
    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseAbort)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_abort, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseTerminated)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_device_terminated, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseStopped)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_device_stopped, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, (*spy_ccapi_data));

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data->transport_tcp.connected);
}
