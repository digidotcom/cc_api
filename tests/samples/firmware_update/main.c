/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include <stdio.h>
#include "ccapi/ccapi.h"

#define DEVICE_TYPE             "Device type"
#define DEVICE_CLOUD_URL        "login.etherios.com"
#define VENDOR_ID               0x030000DB

const uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF};
const uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01};

static firmware_target_t firmware_list[] = {
       /* version   description           filespec                    maximum_size       chunk_size */
        {{1,0,0,0}, (char*)"Bootloader",  (char*)".*\\.[bB][iI][nN]", 1 * 1024 * 1024,   128 * 1024 },  /* any *.bin files */
        {{0,0,1,0}, (char*)"Kernel",      (char*)".*\\.a",            128 * 1024 * 1024, 128 * 1024 }   /* any *.a files */
    };
static uint8_t firmware_count = (sizeof(firmware_list)/sizeof(firmware_list[0]));

static void get_device_id_from_mac(uint8_t * const device_id, uint8_t const * const mac_addr)
{
    memset(device_id, 0x00, 16);
    device_id[8] = mac_addr[0];
    device_id[9] = mac_addr[1];
    device_id[10] = mac_addr[2];
    device_id[11] = 0xFF;
    device_id[12] = 0xFF;
    device_id[13] = mac_addr[3];
    device_id[14] = mac_addr[4];
    device_id[15] = mac_addr[5];
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

FILE * fp;

static ccapi_firmware_update_error_t test_firmware_update_request_cb(unsigned int const target, char const * const filename, size_t const total_size)
{
    (void)target;
    (void)total_size;

    fp = fopen(filename, "wb+");
    if (fp == NULL)
    {
        printf("Unable to create %s file\n", filename );
        return CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD;
    }

    return CCAPI_FIRMWARE_UPDATE_ERROR_NONE;
}

static ccapi_firmware_update_error_t test_firmware_update_data_cb(unsigned int const target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk)
{
    size_t const max_bytes_to_print = 4;
    size_t const bytes_to_print = (size > max_bytes_to_print) ? max_bytes_to_print : size;
    size_t i;

    (void)target;

    printf("offset = 0x%" PRIx32 "\n", offset);

    printf("data = ");
    for (i=0; i < bytes_to_print; i++)
    {
        printf("0x%02X ", ((uint8_t*)data)[i]);
    }
    printf("...\n");

    printf("length = %" PRIsize " last_chunk=%d\n", size, last_chunk);

    fwrite(data, 1, size, fp);

    if (last_chunk)
	{
        fclose(fp);
    }

    return CCAPI_FIRMWARE_UPDATE_ERROR_NONE;
}

static ccapi_start_error_t app_start_ccapi(void)
{
    ccapi_start_t start = {0};
    ccapi_firmware_update_service_t fw_service = {
                                                     {
                                                         firmware_list, 
                                                         firmware_count
                                                     }, 
                                                     {
                                                         test_firmware_update_request_cb, 
                                                         test_firmware_update_data_cb, 
                                                         NULL
                                                     }
                                                 };

    ccapi_start_error_t start_error;

    get_device_id_from_mac(start.device_id, mac);
    start.vendor_id = VENDOR_ID;
    start.device_cloud_url = DEVICE_CLOUD_URL;
    start.device_type = DEVICE_TYPE;

    start.service.firmware = &fw_service;

    start_error = ccapi_start(&start);
    if (start_error == CCAPI_START_ERROR_NONE)
    {
        printf("ccapi_start success\n");
    }
    else
    {
        printf("ccapi_start error %d\n", start_error);
        goto done;
    }

done:
    return start_error;
}

static ccapi_tcp_start_error_t app_start_tcp_transport(void)
{
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};

    tcp_info.connection.type = CCAPI_CONNECTION_LAN;
    memcpy(tcp_info.connection.ip.address.ipv4, ipv4, sizeof tcp_info.connection.ip.address.ipv4);
    memcpy(tcp_info.connection.info.lan.mac_address, mac, sizeof tcp_info.connection.info.lan.mac_address);

    tcp_info.callback.close = ccapi_tcp_close_cb;
    tcp_info.callback.keepalive = NULL;
    tcp_start_error = ccapi_start_transport_tcp(&tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("ccapi_start_transport_tcp success\n");
    }
    else
    {
        printf("ccapi_start_transport_tcp failed with error %d\n", tcp_start_error);
        goto done;
    }
done:
    return tcp_start_error;
}


int main (void)
{
    if (app_start_ccapi() != CCAPI_START_ERROR_NONE)
    {
        goto done;
    }

    if (app_start_tcp_transport() != CCAPI_TCP_START_ERROR_NONE)
    {
        goto done;
    }

    printf("Waiting for ever\n");
    for(;;);

done:
    return 0;
}
