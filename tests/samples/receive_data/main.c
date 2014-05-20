/*
 * Copyright (c) 2014 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include <stdio.h>
#include <time.h>
#include "ccapi/ccapi.h"

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

#define DESIRED_MAX_RESPONSE_SIZE 400

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

static ccapi_bool_t app_receive_default_accept_cb(char const * const target, ccapi_transport_t const transport)
{
    ccapi_bool_t accept_target = CCAPI_TRUE;

    printf("app_receive_default_accept_cb: target = '%s'. transport = %d\n", target, transport);

    if (transport == CCAPI_TRANSPORT_SMS)
    {
        /* Don't accept requests comming throught SMS for not registered targets */
        accept_target = CCAPI_FALSE;
    }

    return accept_target;
}

static void app_receive_default_data_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    printf("app_receive_default_data_cb: target = '%s'. transport = %d. Error = %d\n", target, transport, receive_error);

    /* Print data */
    if (receive_error == CCAPI_RECEIVE_ERROR_NONE)
    {
        size_t i;

        for (i=0 ; i < request_buffer_info->length ; i++)
        {
            printf("%c", ((char*)request_buffer_info->buffer)[i]);
        }
        printf("\napp_receive_default_data_cb received total %d bytes\n", request_buffer_info->length);
    }

    /* Provide response */
    if (response_buffer_info != NULL)
    {
        response_buffer_info->buffer = malloc(DESIRED_MAX_RESPONSE_SIZE);
        printf("app_receive_default_data_cb: Providing response in buffer at %p\n", response_buffer_info->buffer);

        if (receive_error != CCAPI_RECEIVE_ERROR_NONE)
        {
            response_buffer_info->length = sprintf(response_buffer_info->buffer, "Error %d while handling target %s", receive_error, target);
        } 
        else
        {
            response_buffer_info->length = sprintf(response_buffer_info->buffer, "Thanks for the info");
        }
    }

    return;
}

static void app_get_time_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    size_t max_length;

    printf("app_get_time_cb: transport = %d. Error = %d\n", transport, receive_error);

    if (transport == CCAPI_TRANSPORT_SMS)
        max_length = 80;
    else
        max_length = DESIRED_MAX_RESPONSE_SIZE;

    response_buffer_info->buffer = malloc(max_length);

    assert(response_buffer_info != NULL);

    if (receive_error != CCAPI_RECEIVE_ERROR_NONE)
    {
        response_buffer_info->length = sprintf(response_buffer_info->buffer, "Error %d while handling target %s", receive_error, target);
    } 
    else if (request_buffer_info->length != 0)
    {
        response_buffer_info->length = sprintf(response_buffer_info->buffer, "Invalid argument, this call does not take arguments");
    }
    else
    {
        time_t t = time(NULL);
        response_buffer_info->length = snprintf(response_buffer_info->buffer, max_length, "time: %s", ctime(&t));
    }

    return;
}

static void app_receive_default_status_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    printf("app_receive_default_status_cb: target = '%s'. transport = %d. Error = %d\n", target, transport, receive_error);

    if (response_buffer_info != NULL)
    {
        printf("Freeing response buffer at %p\n", response_buffer_info->buffer);
        free(response_buffer_info->buffer);
    }

    (void)response_buffer_info;
}

int main (void)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t start_error = CCAPI_START_ERROR_NONE;
    ccapi_tcp_start_error_t tcp_start_error;
    ccapi_tcp_info_t tcp_info = {{0}};
    uint8_t ipv4[] = {0xC0, 0xA8, 0x01, 0x01}; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
    ccapi_receive_service_t receive_service = {app_receive_default_accept_cb, app_receive_default_data_cb, app_receive_default_status_cb};

    fill_start_structure_with_good_parameters(&start);
    start.service.receive = &receive_service;

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
    }
    else
    {
        printf("ccapi_start_transport_tcp failed with error %d\n", tcp_start_error);
        goto done;
    }

    {
        ccapi_receive_error_t receive_error;
        
        receive_error = ccapi_receive_add_target("get_system_time", app_get_time_cb, app_receive_default_status_cb);
        if (receive_error == CCAPI_RECEIVE_ERROR_NONE)
        {
            printf("ccapi_receive_add_target success\n");
        }
        else
        {
            printf("ccapi_receive_add_target failed with error %d\n", receive_error);
            goto done;
        }
    }

    printf("loop forever\n");
    
	for(;;);

done:
    return 0;
}
