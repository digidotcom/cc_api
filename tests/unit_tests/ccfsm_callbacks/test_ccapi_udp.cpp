#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_udp)
{
    void setup()
    {
        Mock_create_all();
        th_start_ccapi();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_udp, testUdpOpen)
{
    connector_request_id_t request;
    connector_network_open_t connector_open_data = {{"login.etherios.com"}, NULL};
    connector_callback_status_t status;
    ccimp_network_open_t ccimp_open_data = {{"login.etherios.com"}, NULL};

    Mock_ccimp_network_udp_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_open;
    status = ccapi_connector_callback(connector_class_id_network_udp, request, &connector_open_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccapi_udp, testUdpSend)
{
    connector_network_handle_t handle = &handle;
    uint8_t buffer[] = "Binary buffer";
    size_t bytes_available = sizeof buffer;
    size_t bytes_used = 0;
    connector_request_id_t request;
    connector_network_send_t connector_send_data = {handle, buffer, bytes_available, bytes_used};
    ccimp_network_send_t ccimp_send_data = {handle, buffer, bytes_available, bytes_used};
    connector_callback_status_t status;
    Mock_ccimp_network_udp_send_expectAndReturn(&ccimp_send_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_send;
    status = ccapi_connector_callback(connector_class_id_network_udp, request, &connector_send_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccapi_udp, testUdpReceive)
{
    connector_network_handle_t handle = &handle;
    uint8_t buffer[1024] = {0};
    size_t bytes_available = sizeof buffer;
    size_t bytes_used = 0;
    connector_request_id_t request;
    connector_network_receive_t connector_receive_data = {handle, buffer, bytes_available, bytes_used};
    ccimp_network_receive_t ccimp_receive_data = {handle, buffer, bytes_available, bytes_used};
    connector_callback_status_t status;

    Mock_ccimp_network_udp_receive_expectAndReturn(&ccimp_receive_data, CCIMP_STATUS_OK);

    request.network_request = connector_request_id_network_receive;
    status = ccapi_connector_callback(connector_class_id_network_udp, request, &connector_receive_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
}
