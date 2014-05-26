#include <stdio.h>
#include <unistd.h>
#include "ccapi/ccxapi.h"

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x0300009F; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

static ccapi_bool_t ccapi_tcp_close_cb(ccapi_tcp_close_cause_t cause)
{
    ccapi_bool_t reconnect;
    switch (cause)
    {
        case CCAPI_TCP_CLOSE_DISCONNECTED:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_DISCONNECTED\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_REDIRECTED:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_REDIRECTED\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_NO_KEEPALIVE:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_NO_KEEPALIVE\n");
            reconnect = CCAPI_TRUE;
            break;
        case CCAPI_TCP_CLOSE_DATA_ERROR:
            printf("ccapi_tcp_close_cb cause CCAPI_TCP_CLOSE_DATA_ERROR\n");
            reconnect = CCAPI_TRUE;
            break;
    }
    return reconnect;
}

int main (void)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t start_error;
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};

    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    ccapi_handle_t ccapi_handle_devA = NULL;
    uint8_t device_id_devA[] = {0x00, 0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x9F, 0xEC, 0x46, 0x19, 0x0E}; /* Auto-provisioned */

    ccapi_handle_t ccapi_handle_devB = NULL;
    uint8_t device_id_devB[] = {0x00, 0x08, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x9F, 0x78, 0x44, 0x33, 0x37}; /* Auto-provisioned */

    fill_start_structure_with_good_parameters(&start);

    /* Start devA */
    memcpy(start.device_id, device_id_devA, sizeof start.device_id);

    start_error = ccxapi_start(&ccapi_handle_devA, &start);
    if (start_error == CCAPI_START_ERROR_NONE)
    {
        printf("ccxapi_start success for devA (handle=%p)\n", (void *)ccapi_handle_devA);
    }
    else
    {
        printf("ccxapi_start error %d for devA\n", start_error);
        goto done;
    }

    /* Start devB */
    memcpy(start.device_id, device_id_devB, sizeof start.device_id);

    start_error = ccxapi_start(&ccapi_handle_devB, &start);
    if (start_error == CCAPI_START_ERROR_NONE)
    {
        printf("ccxapi_start success for devB (handle=%p)\n", (void *)ccapi_handle_devB);
    }
    else
    {
        printf("ccxapi_start error %d for devB\n", start_error);
        goto done;
    }

    tcp_info.connection.type = CCAPI_CONNECTION_LAN;
    memcpy(tcp_info.connection.ip.address.ipv4, ipv4, sizeof tcp_info.connection.ip.address.ipv4);
    memcpy(tcp_info.connection.info.lan.mac_address, mac, sizeof tcp_info.connection.info.lan.mac_address);

    tcp_info.callback.close = ccapi_tcp_close_cb;
    tcp_info.callback.keepalive = NULL;

    /* Start devA tcp transport*/
    tcp_start_error = ccxapi_start_transport_tcp(ccapi_handle_devA, &tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("ccxapi_start_transport_tcp success for handle %p \n", (void *)ccapi_handle_devA);
    }
    else
    {
        printf("ccxapi_start_transport_tcp failed with error %d for handle %p\n", tcp_start_error, (void *)ccapi_handle_devA);
        goto done;
    }

    /* Start devB tcp transport*/
    tcp_start_error = ccxapi_start_transport_tcp(ccapi_handle_devB, &tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("ccapi_start_transport_tcp success for handle %p \n", (void *)ccapi_handle_devB);
    }
    else
    {
        printf("ccapi_start_transport_tcp failed with error %d for handle %p\n", tcp_start_error, (void *)ccapi_handle_devB);
        goto done;
    }

    printf("Waiting for ever\n");
    for(;;);

done:
    return 0;
}
