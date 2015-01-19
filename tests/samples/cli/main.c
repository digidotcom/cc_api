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
#include <time.h>
#include <unistd.h>
#include "ccapi/ccapi.h"

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

#define TEST_UDP 1
#define TEST_SMS 0

void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
	
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x030000DB; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

#define FIX_RESPONSE "This is the command echo: "

static void app_cli_request_cb(ccapi_transport_t const transport, char const * const command, char const * * const output)
{
    char * response;
    printf("app_cli_request_cb: transport = %d, command = '%s'\n", transport, command);

    /* Provide response to the cloud */
    if (output != NULL)
    {
        response = malloc(sizeof(FIX_RESPONSE) + strlen(command));
        printf("app_cli_request_cb: Providing response in buffer at %p\n", response);

        sprintf(response, FIX_RESPONSE "'%s'", command);

        *output = response;
    }
    else
    {
        printf("app_cli_request_cb: Response not requested by cloud\n");
    }

    return;
}


static void app_cli_finished_cb(char * const output, ccapi_cli_error_t cli_error)
{
    printf("app_cli_finished_cb: Error = %d\n", cli_error);

    if (output != NULL)
    {
        printf("app_cli_finished_cb: Freeing response buffer at %p\n", output);
        free(output);
    }
}

int main (void)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t start_error = CCAPI_START_ERROR_NONE;
    ccapi_cli_service_t cli_service;

    fill_start_structure_with_good_parameters(&start);

    start.service.cli = &cli_service;
    cli_service.request = app_cli_request_cb;
    cli_service.finished = app_cli_finished_cb;

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

#if (TEST_UDP == 1)
    /* Start UDP trasnport */
    {
        ccapi_udp_start_error_t udp_start_error;
        ccapi_udp_info_t udp_info = {{0}};

        udp_info.start_timeout = CCAPI_UDP_START_WAIT_FOREVER;
        udp_info.limit.max_sessions = 1;
        udp_info.limit.rx_timeout = CCAPI_UDP_RX_TIMEOUT_INFINITE;

        udp_info.callback.close = NULL;

        udp_start_error = ccapi_start_transport_udp(&udp_info);
        if (udp_start_error == CCAPI_UDP_START_ERROR_NONE)
        {
            printf("ccapi_start_transport_udp success\n");
        }
        else
        {
            printf("ccapi_start_transport_udp failed with error %d\n", udp_start_error);
            goto done;
        }
    }
#endif

#if (TEST_SMS == 1)
    /* Start SMS trasnport */
    {
        ccapi_sms_start_error_t sms_start_error;
        ccapi_sms_info_t sms_info = {{0}};

        sms_info.start_timeout = CCAPI_TCP_START_WAIT_FOREVER;
        sms_info.limit.max_sessions = 1;
        sms_info.limit.rx_timeout = CCAPI_SMS_RX_TIMEOUT_INFINITE;

        sms_info.callback.close = NULL;

        sms_info.cloud_config.phone_number = "32075";
        sms_info.cloud_config.service_id = "idgp";

        sms_start_error = ccapi_start_transport_sms(&sms_info);
        if (sms_start_error == CCAPI_SMS_START_ERROR_NONE)
        {
            printf("ccapi_start_transport_sms success\n");
        }
        else
        {
            printf("ccapi_start_transport_sms failed with error %d\n", sms_start_error);
            goto done;
        }
    }
#endif

#if (TEST_UDP == 1)
    printf("Send UDP traffic periodically to the cloud so it send us queued requests\n");
	for(;;)
    {   
        /* TODO: send ping instead of data*/
        ccapi_send_error_t send_error;
        #define SEND_DATA_UDP         "ping"
        send_error = ccapi_send_data(CCAPI_TRANSPORT_UDP, "ping.txt", "text/plain", SEND_DATA_UDP, strlen(SEND_DATA_UDP), CCAPI_SEND_BEHAVIOR_OVERWRITE);
        if (send_error == CCAPI_SEND_ERROR_NONE)
        {
            printf("ccapi_send_data for udp success\n");
        }
        else
        {
            printf("ccapi_send_data for udp failed with error %d\n", send_error);
        }
        
        sleep(5);
    }
#else
    printf("Endless loop\n");
    for(;;);
#endif

done:
    return 0;
}
