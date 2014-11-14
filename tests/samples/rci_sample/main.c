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

#define DEVICE_TYPE_STRING      "Linux Application"
#define DEVICE_CLOUD_URL_STRING "test.etherios.com"

extern ccapi_rci_data_t const ccapi_rci_data;

static ccapi_firmware_target_t firmware_list[] = {
       /* version   description    filespec             maximum_size       chunk_size */
        {{1,0,0,0}, "Bootloader",  ".*\\.[bB][iI][nN]", 1 * 1024 * 1024,   128 * 1024 },  /* any *.bin files */
        {{0,0,1,0}, "Kernel",      ".*\\.a",            128 * 1024 * 1024, 128 * 1024 }   /* any *.a files */
    };
static uint8_t const firmware_count = (sizeof(firmware_list)/sizeof(firmware_list[0]));

static ccapi_fw_request_error_t app_fw_request_cb(unsigned int const target, char const * const filename, size_t const total_size)
{
    (void)target;
    (void)total_size;
    (void)filename;

    return CCAPI_FW_REQUEST_ERROR_NONE;
}

static ccapi_fw_data_error_t app_fw_data_cb(unsigned int const target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk)
{
    (void)target;
    (void)offset;
    (void)data;
    (void)size;
    (void)last_chunk;

    return CCAPI_FW_DATA_ERROR_NONE;
}

static void app_fw_cancel_cb(unsigned int const target, ccapi_fw_cancel_error_t cancel_reason)
{
    (void)target;
    (void)cancel_reason;
    return;
}

ccapi_fw_service_t  fw_service = {
                                        {
                                            2,
                                            firmware_list
                                        },
                                        {
                                            app_fw_request_cb,
                                            app_fw_data_cb,
                                            app_fw_cancel_cb
                                        }
                                    };

void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    static ccapi_rci_service_t rci_service;
    uint8_t device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x9D, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA};
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    
    rci_service.rci_data = &ccapi_rci_data;
    start->vendor_id = 0x2001371; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = &fw_service;
    start->service.rci = &rci_service;
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
    ccapi_start_error_t start_error = CCAPI_START_ERROR_NONE;
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    fill_start_structure_with_good_parameters(&start);

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

    tcp_info.connection.type = CCAPI_CONNECTION_LAN;
    memcpy(tcp_info.connection.ip.address.ipv4, ipv4, sizeof tcp_info.connection.ip.address.ipv4);
    memcpy(tcp_info.connection.info.lan.mac_address, mac, sizeof tcp_info.connection.info.lan.mac_address);

    tcp_info.callback.close = ccapi_tcp_close_cb;
    tcp_info.callback.keepalive = NULL;

    tcp_start_error = ccapi_start_transport_tcp(&tcp_info);
    if (tcp_start_error == CCAPI_TCP_START_ERROR_NONE)
    {
        printf("ccapi_start_transport_tcp success\n");
        printf("Waiting for ever\n");
        for(;;);
    }
    else
    {
        printf("ccapi_start_transport_tcp failed with error %d\n", tcp_start_error);
        goto done;
    }

done:
    return 0;
}
