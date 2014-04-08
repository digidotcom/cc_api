#include "test_helper_functions.h"

TEST_GROUP(ccapi_tcp_test_close)
{
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
        th_setup_mock_info_single_instance();
        th_start_tcp_wan_ipv4_with_callbacks();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_DISCONNECTED, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_true, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_REDIRECTED, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_NO_KEEPALIVE, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_true, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(CCAPI_TCP_CLOSE_DATA_ERROR, ccapi_tcp_close_cb_argument);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}

TEST(ccapi_tcp_test_close, testTcpCloseNoCallback)
{
    connector_network_handle_t handle = &handle;
    connector_request_id_t request;
    connector_network_close_t connector_close_data = {handle, connector_close_status_cloud_disconnected, connector_false};
    ccimp_network_close_t ccimp_close_data = {handle};
    connector_callback_status_t status;

    ccapi_data_single_instance->transport_tcp.info->callback.close = NULL;

    Mock_ccimp_network_tcp_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);
    request.network_request = connector_request_id_network_close;
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
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
    status = ccapi_connector_callback(connector_class_id_network_tcp, request, &connector_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_tcp_close_cb_called);
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->transport_tcp.connected);
}
