#include "test_helper_functions.h"
#include <stdio.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

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

ccapi_fs_file_handle_t * th_filesystem_openfile(char const * const path, connector_file_system_open_t * const ccfsm_open_data, int flags)
{
    connector_request_id_t request;
    ccimp_fs_file_open_t ccimp_open_data;
    connector_callback_status_t status;

    ccimp_open_data.errnum.pointer = NULL;
    ccimp_open_data.imp_context = NULL;
    ccimp_open_data.handle.pointer = NULL;
    ccimp_open_data.flags = flags;
    ccimp_open_data.path = path;

    ccfsm_open_data->errnum = NULL;
    ccfsm_open_data->handle = NULL;
    ccfsm_open_data->oflag = flags;
    ccfsm_open_data->path = ccimp_open_data.path;
    ccfsm_open_data->user_context = NULL;

    Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_open;
    status = ccapi_connector_callback(connector_class_id_file_system, request, ccfsm_open_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(ccfsm_open_data->handle != NULL);
    return (ccapi_fs_file_handle_t *)ccfsm_open_data->handle;
}

void th_filesystem_prepare_ccimp_dir_open_data_call(ccimp_fs_dir_open_t * const ccimp_dir_open_data, char const * const path)
{
    ccimp_dir_open_data->errnum.pointer = NULL;
    ccimp_dir_open_data->imp_context = &my_fs_context;
    ccimp_dir_open_data->handle.pointer = NULL;
    ccimp_dir_open_data->path = path;
    Mock_ccimp_fs_dir_open_expectAndReturn(ccimp_dir_open_data, CCIMP_STATUS_OK);
}

void th_filesystem_prepare_ccimp_dir_close_call(ccimp_fs_dir_close_t * const ccimp_dir_close_data)
{
    ccimp_dir_close_data->handle.pointer = &dir_handle;
    ccimp_dir_close_data->errnum.pointer = NULL;
    ccimp_dir_close_data->imp_context = ccapi_data_single_instance->service.file_system.imp_context;
    Mock_ccimp_fs_dir_close_expectAndReturn(ccimp_dir_close_data, CCIMP_STATUS_OK);
}

void create_test_file(char const * const path, void const * const data, size_t bytes)
{
    int fd, result;

    fd = open(path, O_CREAT | O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); /* 0664 = Owner RW + Group RW + Others R */
    CHECK (fd >= 0);

    result = write(fd, data, bytes);
    CHECK (result >= 0);

    result = close(fd);
    CHECK (result >= 0);
}

void destroy_test_file(char const * const path)
{
    int result = unlink(path);
    CHECK (result >= 0);
}

void th_call_ccimp_fs_error_desc_and_check_error(void * ccfsm_errnum, connector_file_system_error_t ccfsm_fs_error)
{
    connector_request_id_t request;
    connector_callback_status_t status;
    connector_file_system_get_error_t ccfsm_error_desc_data;
    char buffer[10];

    ccfsm_error_desc_data.errnum = ccfsm_errnum;
    ccfsm_error_desc_data.user_context = NULL;
    ccfsm_error_desc_data.buffer = buffer;
    ccfsm_error_desc_data.bytes_available = sizeof buffer;
    ccfsm_error_desc_data.bytes_used = 1;

    Mock_ccimp_os_free_expectAndReturn(ccfsm_errnum, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_get_error;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_error_desc_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(NULL != ccfsm_errnum);
    CHECK_EQUAL(ccfsm_fs_error, ccfsm_error_desc_data.error_status);
    CHECK_EQUAL(0, ccfsm_error_desc_data.bytes_used);
}
