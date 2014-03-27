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
    connector_network_open_t connector_open_data = {"login.etherios.com", NULL};
    connector_callback_status_t status;
    ccimp_network_open_t ccimp_open_data = {"login.etherios.com", NULL};

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

TEST(ccapi_tcp_test, testTcpClose)
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
    CHECK_EQUAL(connector_false, connector_close_data.reconnect);
}
