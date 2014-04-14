#include "test_helper_functions.h"
#include <stdio.h>
#include <pthread.h>

ccapi_bool_t ccapi_tcp_keepalives_cb_called;
ccapi_keepalive_status_t ccapi_tcp_keepalives_cb_argument;

ccapi_bool_t ccapi_tcp_close_cb_called;
ccapi_tcp_close_cause_t ccapi_tcp_close_cb_argument;

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

static void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    ccapi_tcp_keepalives_cb_called = CCAPI_TRUE;
    ccapi_tcp_keepalives_cb_argument = status;
    return;
}

void th_fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = TH_DEVICE_CLOUD_URL_STRING;
    char const * const device_type = TH_DEVICE_TYPE_STRING;
    start->vendor_id = 0x12345678; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

void th_start_ccapi(void)
{
    ccapi_start_t start;
    ccapi_start_error_t start_error;

    th_fill_start_structure_with_good_parameters(&start);
    start_error = ccapi_start(&start);
    CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);
}

void th_stop_ccapi(ccapi_data_t * const ccapi_data)
{
    ccapi_stop_error_t stop_error;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);
    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);
}

void th_fill_tcp_wan_ipv4_callbacks_info(ccapi_tcp_info_t * tcp_start)
{
    char phone_number[] = "+54-3644-421921";
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */

    tcp_start->connection.type = CCAPI_CONNECTION_WAN;
    tcp_start->connection.info.wan.phone_number = phone_number;
    tcp_start->connection.info.wan.link_speed = 115200;
    memcpy(tcp_start->connection.ip.address.ipv4, ipv4, sizeof tcp_start->connection.ip.address.ipv4);

    tcp_start->callback.close = ccapi_tcp_close_cb;
    tcp_start->callback.keepalive = ccapi_tcp_keepalives_cb;
    ccapi_tcp_keepalives_cb_called = CCAPI_FALSE;
    ccapi_tcp_close_cb_called = CCAPI_FALSE;
}

void th_start_tcp_wan_ipv4_with_callbacks(void)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    th_fill_tcp_wan_ipv4_callbacks_info(&tcp_start);
    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
}

void th_fill_tcp_lan_ipv4(ccapi_tcp_info_t * tcp_start)
{
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start->connection.type = CCAPI_CONNECTION_LAN;
    tcp_start->connection.ip.type = CCAPI_IPV4;
    memcpy(tcp_start->connection.ip.address.ipv4, ipv4, sizeof tcp_start->connection.ip.address.ipv4);
    memcpy(tcp_start->connection.info.lan.mac_address, mac, sizeof tcp_start->connection.info.lan.mac_address);

    tcp_start->callback.close = NULL;
    tcp_start->callback.keepalive = NULL;
}

void th_start_tcp_lan_ipv4(void)
{
    ccapi_tcp_start_error_t tcp_start_error;
    connector_transport_t connector_transport = connector_transport_tcp;
    ccapi_tcp_info_t tcp_start = {{0}};

    th_fill_tcp_lan_ipv4(&tcp_start);
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,connector_success);
    tcp_start_error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, tcp_start_error);
}

void th_start_tcp_lan_ipv6_password_keepalives(void)
{
    ccapi_tcp_start_error_t tcp_error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t ipv6[] = {0x00, 0x00, 0x00, 0x00, 0xFE, 0x80, 0x00, 0x00, 0x02, 0x25, 0x64, 0xFF, 0xFE, 0x9B, 0xAF, 0x03}; /* fe80::225:64ff:fe9b:af03 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
    connector_transport_t connector_transport = connector_transport_tcp;
    char password[] = "CCAPI Rules!";

    tcp_start.keepalives.rx = 90;
    tcp_start.keepalives.tx = 100;
    tcp_start.keepalives.wait_count = 10;

    tcp_start.connection.password = password;
    tcp_start.connection.max_transactions = 10;
    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.ip.type = CCAPI_IPV6;
    memcpy(tcp_start.connection.ip.address.ipv6, ipv6, sizeof tcp_start.connection.ip.address.ipv6);
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    tcp_start.callback.close = NULL;
    tcp_start.callback.keepalive = NULL;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport,connector_success);
    tcp_error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, tcp_error);
}

static void * thread_wrapper(void * argument)
{

    ccapi_start((ccapi_start_t *)argument);

    return NULL;
}

pthread_t th_aux_ccapi_start(void * argument)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, argument);

    if (ccode != 0)
    {
        printf("aux_ccapi_start() error %d\n", ccode);
    }

    return pthread;
}

int th_stop_aux_thread(pthread_t pthread)
{
    int error;

    error = pthread_cancel(pthread);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }

    error = pthread_join(pthread, NULL);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }
done:
    return error;
}
