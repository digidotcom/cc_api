/*
 * test_helper_functions.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: spastor
 */

#include <stdio.h>
#include <pthread.h>
#include "test_helper_functions.h"

static ccapi_bool_t ccapi_tcp_close_cb(ccapi_tcp_close_cause_t cause)
{
    UNUSED_ARGUMENT(cause);
    return CCAPI_TRUE;
}

static void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    UNUSED_ARGUMENT(status);
    return;
}

void th_fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
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

mock_connector_api_info_t * th_setup_mock_info_single_instance(void)
{
    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data_single_instance;

    return mock_info;
}

void th_stop_ccapi(ccapi_data_t * const ccapi_data)
{
    ccapi_stop_error_t stop_error;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);
    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);
}

void th_start_tcp_wan_ipv4(void)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;
    memcpy(tcp_start.connection.ip.address.ipv4, ipv4, sizeof tcp_start.connection.ip.address.ipv4);

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
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
