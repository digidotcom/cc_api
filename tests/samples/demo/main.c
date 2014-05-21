
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "ccapi/ccapi.h"
#include "device_id_utils.h"
#include "simulated_tank.h"

#define DEVICE_TYPE             "Simulated Tank (CCAPI)"
#define DEVICE_CLOUD_URL        "login.etherios.com"
#define IPV4_ADDRESS_STRING     "192.168.1.2"

//#error "Define your Deice MAC Address and your account's Vendor ID and comment this error line"
#define MAC_ADDRESS_STRING      "00049D:ABCDFF"
#define VENDOR_ID               0x030000BD

#define SAMPLE_DATA_PATH        "test/ccapi_tank_demo.txt"
#define SAMPLE_DATA_CONTENT     "CCAPI tank simulation sample file"

#define DATA_POINTS_PER_UPLOAD  10
#define SECONDS_BETWEEN_DPS     1

static void tank_valves_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    char const * valveIN_token = NULL;
    char const * valveOUT_token = NULL;

    assert(transport == CCAPI_TRANSPORT_TCP);

    printf("\t *** Received request from Device Cloud\n");
    if (receive_error != CCAPI_RECEIVE_ERROR_NONE)
    {
        response_buffer_info->length = sprintf(response_buffer_info->buffer, "Error %d while handling target %s", receive_error, target);
        return;
    }

    valveIN_token = strstr(request_buffer_info->buffer, "valveIN=");
    valveOUT_token = strstr(request_buffer_info->buffer, "valveOUT=");

    if (valveIN_token != NULL)
    {
        char const * const value = valveIN_token + strlen("valveIN=");

        printf("\tValveIN: ");

        if (*value == '0')
        {
            printf("Opened\n");
            set_valveIN_status(VALVE_OPENED);
        }
        else if (*value == '1')
        {
            printf("Closed\n");
            set_valveIN_status(VALVE_CLOSED);
        }
        else
        {
            printf("Invalid value, status not changed!!!\n");
        }
    }

    if (valveOUT_token != NULL)
    {
        char const * const value = valveOUT_token + strlen("valveOUT=");

        printf("\tValveOUT: ");

        if (*value == '0')
        {
            printf("Opened\n");
            set_valveOUT_status(VALVE_OPENED);
        }
        else if (*value == '1')
        {
            printf("Closed\n");
            set_valveIN_status(VALVE_CLOSED);
        }
        else
        {
            printf("Invalid value, status not changed!!!\n");
        }
    }

    return;
}

static void tank_demo_loop(void)
{
    ccapi_dp_collection_handle_t dp_collection;
    ccapi_timestamp_t timestamp;

    timestamp.epoch.milliseconds = 0;

    ccapi_dp_create_collection(&dp_collection);
    ccapi_dp_add_data_stream_to_collection(dp_collection, "tank/temperature", "double ts_epoch");
    ccapi_dp_add_data_stream_to_collection(dp_collection, "tank/level", "int32 ts_epoch");
    ccapi_dp_add_data_stream_to_collection(dp_collection, "tank/valveIN", "int32");
    ccapi_dp_add_data_stream_to_collection(dp_collection, "tank/valveOUT", "int32");

    for(;;)
    {
        int i;

        for (i = 0; i < DATA_POINTS_PER_UPLOAD; i++)
        {
            timestamp.epoch.seconds = time(NULL);

            update_simulated_tank_status();

            ccapi_dp_add(dp_collection, "tank/temperature", get_tank_temperature(), &timestamp);
            ccapi_dp_add(dp_collection, "tank/level", get_tank_fill_percentage(), &timestamp);
            ccapi_dp_add(dp_collection, "tank/valveIN", get_valveIN_status());
            ccapi_dp_add(dp_collection, "tank/valveOUT", get_valveOUT_status());

            sleep(SECONDS_BETWEEN_DPS);
        }

        printf("\tSending Data Point collection\n");
        print_tank_status();

        ccapi_dp_send_collection(CCAPI_TRANSPORT_TCP, dp_collection);
    }
}

int main (void)
{
    uint8_t mac_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t ipv4[] = {0x00, 0x00, 0x00, 0x00};
    ccapi_filesystem_service_t filesystem_service;
    ccapi_receive_service_t receive_service;
    ccapi_start_t start = {0};
    ccapi_start_error_t start_error;
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};
    ccapi_receive_error_t receive_error;

    get_mac_from_string(mac_address, MAC_ADDRESS_STRING);
    get_ipv4_from_string(ipv4, IPV4_ADDRESS_STRING);

    filesystem_service.access_cb = NULL;
    filesystem_service.changed_cb = NULL;

    receive_service.accept_cb = NULL;
    receive_service.data_cb = tank_valves_cb;
    receive_service.status_cb = NULL;

    start.device_cloud_url = DEVICE_CLOUD_URL;
    get_device_id_from_mac(start.device_id, mac_address);

    start.device_type = DEVICE_TYPE;
    start.service.file_system = &filesystem_service;
    start.service.receive = &receive_service;
    start.status_callback = NULL;
    start.vendor_id = VENDOR_ID;

    printf("\tStarting CCAPI\n");
    start_error = ccapi_start(&start);

    if (start_error == CCAPI_START_ERROR_NONE)
    {
        printf("\tccapi_start success\n");
    }
    else
    {
        printf("\tccapi_start error %d\n", start_error);
        goto done;
    }

    memcpy(tcp_info.connection.ip.address.ipv4, ipv4, sizeof tcp_info.connection.ip.address.ipv4);
    memcpy(tcp_info.connection.info.lan.mac_address, mac_address, sizeof tcp_info.connection.info.lan.mac_address);
    tcp_info.connection.type = CCAPI_CONNECTION_LAN;
    tcp_info.callback.close = NULL;
    tcp_info.callback.keepalive = NULL;

    printf("\tStarting TCP transport\n");
    tcp_start_error = ccapi_start_transport_tcp(&tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("\tccapi_start_transport_tcp success\n");
    }
    else
    {
        printf("\tccapi_start_transport_tcp failed with error %d\n", tcp_start_error);
        goto done;
    }

    printf("\tRegistering Receive Service target\n");
    receive_error = ccapi_receive_add_target("tank", tank_valves_cb, NULL, 100);
    if (receive_error == CCAPI_RECEIVE_ERROR_NONE)
    {
        printf("\tccapi_receive_add_target success\n");
    }
    else
    {
        printf("\tccapi_receive_add_target failed with error %d\n", receive_error);
    }

    printf("\tUploading sample data to %s\n", SAMPLE_DATA_PATH);
    ccapi_send_data(CCAPI_TRANSPORT_TCP, SAMPLE_DATA_PATH, "text/plain", SAMPLE_DATA_CONTENT, (sizeof SAMPLE_DATA_CONTENT) - 1, CCAPI_SEND_BEHAVIOR_OVERWRITE);

    printf("\tEntering application's infinite loop\n");
    tank_demo_loop();

done:
    return 0;
}
