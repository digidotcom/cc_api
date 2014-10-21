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

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"
#include "ccapi/ccxapi.h"

static ccapi_start_error_t check_params(ccapi_start_t const * const start)
{
    static uint8_t const invalid_device_id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;

    if (start->vendor_id == 0x00)
    {
        error = CCAPI_START_ERROR_INVALID_VENDORID;
        goto done;
    }

    if (memcmp(start->device_id, invalid_device_id, sizeof invalid_device_id) == 0)
    {
        error = CCAPI_START_ERROR_INVALID_DEVICEID;
        goto done;
    }

    if (start->device_cloud_url == NULL || start->device_cloud_url[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_URL;
        goto done;
    }

    if (start->device_type == NULL || start->device_type[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_DEVICETYPE;
        goto done;
    }

done:
    return error;
}

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
static void free_filesystem_dir_entry_list(ccapi_data_t * const ccapi_data)
{
    ccapi_fs_virtual_dir_t * dir_entry = ccapi_data->service.file_system.virtual_dir_list;

    do {
        ccapi_fs_virtual_dir_t * const next_dir_entry = dir_entry->next;
        ccapi_free(dir_entry->local_dir);
        ccapi_free(dir_entry->virtual_dir);
        ccapi_free(dir_entry);
        dir_entry = next_dir_entry;
    } while (dir_entry != NULL);
}
#endif

#if (defined CCIMP_DATA_SERVICE_ENABLED)
static void free_receive_target_list(ccapi_data_t * const ccapi_data)
{
    ccimp_status_t ccimp_status;
    ccapi_receive_target_t * target_entry;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.receive.receive_syncr);
    ASSERT_MSG(ccimp_status == CCIMP_STATUS_OK);

    target_entry = ccapi_data->service.receive.target_list;

    while (target_entry != NULL)
    {
        ccapi_receive_target_t * const next_target_entry = target_entry->next;

        ccapi_free(target_entry->target);
        ccapi_free(target_entry);
        target_entry = next_target_entry;
    }

    ccimp_status = ccapi_syncr_release(ccapi_data->service.receive.receive_syncr);
    ASSERT_MSG(ccimp_status == CCIMP_STATUS_OK);
}
#endif

static void free_ccapi_data_internal_resources(ccapi_data_t * const ccapi_data)
{
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    if (ccapi_data->file_system_syncr != NULL)
    {
        ccimp_status_t const ccimp_status = ccapi_syncr_destroy(ccapi_data->file_system_syncr);

        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                ASSERT_MSG(ccimp_status == CCIMP_STATUS_OK);
        }
    }

    if (ccapi_data->config.filesystem_supported)
    {
        if (ccapi_data->service.file_system.virtual_dir_list != NULL)
        {
            free_filesystem_dir_entry_list(ccapi_data);
        }
    }
#endif

#if (defined CCIMP_DATA_SERVICE_ENABLED)
    if (ccapi_data->config.receive_supported)
    {
        if (ccapi_data->service.receive.target_list != NULL)
        {
            free_receive_target_list(ccapi_data);
        }

        if (ccapi_data->service.receive.receive_syncr != NULL)
        {
            ccimp_status_t const ccimp_status = ccapi_syncr_destroy(ccapi_data->service.receive.receive_syncr);

            ASSERT_MSG(ccimp_status == CCIMP_STATUS_OK);
        }
    }
#endif
#if (defined CCIMP_FIRMWARE_SERVICE_ENABLED)
    if (ccapi_data->service.firmware_update.config.target.count && ccapi_data->service.firmware_update.config.target.item != NULL)
    {
        unsigned char target_num;

        for (target_num = 0; target_num < ccapi_data->service.firmware_update.config.target.count; target_num++)
        {
            if (ccapi_data->service.firmware_update.config.target.item[target_num].description != NULL)
            {
                ccapi_free((void *)ccapi_data->service.firmware_update.config.target.item[target_num].description);
            }

            if (ccapi_data->service.firmware_update.config.target.item[target_num].filespec != NULL)
            {
                ccapi_free((void*)ccapi_data->service.firmware_update.config.target.item[target_num].filespec);
            }
        }

        ccapi_free(ccapi_data->service.firmware_update.config.target.item);

        ccapi_data->service.firmware_update.config.target.count = 0;
        ccapi_data->service.firmware_update.config.target.item = NULL;
    }
#endif

    reset_heap_ptr(&ccapi_data->config.device_type);
    reset_heap_ptr(&ccapi_data->config.device_cloud_url);
    reset_heap_ptr(&ccapi_data->thread.connector_run);

    if (ccapi_data->initiate_action_syncr != NULL)
    {   
        ccimp_status_t const ccimp_status = ccapi_syncr_destroy(ccapi_data->initiate_action_syncr);

        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                ASSERT_MSG(ccimp_status == CCIMP_STATUS_OK);
        }
    }

done:
    return; 
}

static ccapi_start_error_t check_malloc(void const * const p)
{
    if (p == NULL)
        return CCAPI_START_ERROR_INSUFFICIENT_MEMORY;
    else
        return CCAPI_START_ERROR_NONE;
}

/* This function allocates ccapi_data_t so other ccXapi_* functions can use it as a handler */
ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;
    ccapi_data_t * ccapi_data = NULL;

    if (ccapi_handle == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    if (*ccapi_handle != NULL)
    {
        error = CCAPI_START_ERROR_ALREADY_STARTED;
        goto done;
    }

    ccapi_data = ccapi_malloc(sizeof *ccapi_data);
    *ccapi_handle = (ccapi_handle_t)ccapi_data;

    error = check_malloc(ccapi_data);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->initiate_action_syncr = NULL;
    ccapi_data->service.file_system.virtual_dir_list = NULL;
    ccapi_data->file_system_syncr = NULL;

    /* Initialize one single time for all connector instances the logging syncr object */
    if (logging_syncr == NULL)
    {
        ccimp_os_syncr_create_t create_data;
    
        if (ccimp_os_syncr_create(&create_data) != CCIMP_STATUS_OK || ccapi_syncr_release(create_data.syncr_object) != CCIMP_STATUS_OK)
        {
            error = CCAPI_START_ERROR_SYNCR_FAILED;
            goto done;
        }

        logging_syncr = create_data.syncr_object;
    }

    ccapi_data->config.device_type = NULL;
    ccapi_data->config.device_cloud_url = NULL;
    ccapi_data->thread.connector_run = NULL;
    ccapi_data->initiate_action_syncr = NULL;

    ccapi_data->config.firmware_supported = CCAPI_FALSE;
    ccapi_data->service.firmware_update.config.target.count = 0;
    ccapi_data->service.firmware_update.config.target.item = NULL;
    ccapi_data->config.receive_supported = CCAPI_FALSE;
    ccapi_data->config.cli_supported = CCAPI_FALSE;

    if (start == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    error = check_params(start);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->config.vendor_id = start->vendor_id;
    memcpy(ccapi_data->config.device_id, start->device_id, sizeof ccapi_data->config.device_id);

    ccapi_data->config.device_type = ccapi_malloc(strlen(start->device_type) + 1);
    error = check_malloc(ccapi_data->config.device_type);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_type, start->device_type);

    ccapi_data->config.device_cloud_url = ccapi_malloc(strlen(start->device_cloud_url) + 1);
    error = check_malloc(ccapi_data->config.device_cloud_url);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_cloud_url, start->device_cloud_url);

    ccapi_data->config.cli_supported = start->service.cli == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.rci_supported = start->service.rci == NULL ? CCAPI_FALSE : CCAPI_TRUE;

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    if (start->service.file_system != NULL)
    {
        ccapi_data->config.filesystem_supported = CCAPI_TRUE;
        ccapi_data->service.file_system.user_callbacks.access_cb = start->service.file_system->access_cb;
        ccapi_data->service.file_system.user_callbacks.changed_cb = start->service.file_system->changed_cb;
        ccapi_data->service.file_system.imp_context = NULL;
        ccapi_data->service.file_system.virtual_dir_list = NULL;
    }
    else
#endif
    {
        ccapi_data->config.filesystem_supported = CCAPI_FALSE;
    }

#if (defined CCIMP_FIRMWARE_SERVICE_ENABLED)
    if (start->service.firmware != NULL)
    {
        /* Check required callbacks */
        if (start->service.firmware->callback.data_cb == NULL)
        {
            error = CCAPI_START_ERROR_INVALID_FIRMWARE_DATA_CALLBACK;
            goto done;
        }

        /* If target info is wrong, we won't let CCAPI start */ 
        if (start->service.firmware->target.item == NULL || start->service.firmware->target.count == 0)
        {
            error = CCAPI_START_ERROR_INVALID_FIRMWARE_INFO;
            goto done;
        }

        {
            size_t const list_size = start->service.firmware->target.count * sizeof *start->service.firmware->target.item;
            unsigned char target_num;

            ccapi_data->service.firmware_update.config.target.count = start->service.firmware->target.count;

            ccapi_data->service.firmware_update.config.target.item = ccapi_malloc(list_size);
            error = check_malloc(ccapi_data->service.firmware_update.config.target.item);
            if (error != CCAPI_START_ERROR_NONE)
                goto done;

            memcpy(ccapi_data->service.firmware_update.config.target.item, start->service.firmware->target.item, list_size);

            for (target_num = 0; target_num < start->service.firmware->target.count; target_num++)
            {
                size_t const description_size = strlen(start->service.firmware->target.item[target_num].description) + 1;
                size_t const filespec_size = strlen(start->service.firmware->target.item[target_num].filespec) + 1;
                char * description;
                char * filespec;

                description = ccapi_malloc(description_size);
                error = check_malloc(description);
                if (error != CCAPI_START_ERROR_NONE)
                    goto done;
                memcpy(description, start->service.firmware->target.item[target_num].description, description_size);
                ccapi_data->service.firmware_update.config.target.item[target_num].description = description;

                filespec = ccapi_malloc(filespec_size);
                error = check_malloc(filespec);
                if (error != CCAPI_START_ERROR_NONE)
                    goto done;
                memcpy(filespec, start->service.firmware->target.item[target_num].filespec, filespec_size);
                ccapi_data->service.firmware_update.config.target.item[target_num].filespec = filespec;
            }

            ccapi_data->service.firmware_update.config.callback.request_cb = start->service.firmware->callback.request_cb;
            ccapi_data->service.firmware_update.config.callback.data_cb = start->service.firmware->callback.data_cb;
            ccapi_data->service.firmware_update.config.callback.cancel_cb = start->service.firmware->callback.cancel_cb;

            ccapi_data->service.firmware_update.processing.chunk_data = NULL;
            ccapi_data->service.firmware_update.processing.update_started = CCAPI_FALSE;
            ccapi_data->config.firmware_supported = CCAPI_TRUE;
        }
    }
#endif

#if (defined CCIMP_DATA_SERVICE_ENABLED)
    if (start->service.receive != NULL)
    {
        ccapi_data->service.receive.receive_syncr = ccapi_syncr_create_and_release();
        if (ccapi_data->service.receive.receive_syncr == NULL)
        {
            error = CCAPI_START_ERROR_SYNCR_FAILED;
            goto done;
        }

        ccapi_data->config.receive_supported = CCAPI_TRUE;
        ccapi_data->service.receive.user_callbacks.accept_cb = start->service.receive->accept_cb;
        ccapi_data->service.receive.user_callbacks.data_cb = start->service.receive->data_cb;
        ccapi_data->service.receive.user_callbacks.status_cb = start->service.receive->status_cb;
        ccapi_data->service.receive.target_list = NULL;
    }
#endif

#if (defined CCIMP_UDP_TRANSPORT_ENABLED || defined CCIMP_SMS_TRANSPORT_ENABLED)
#if (defined CONNECTOR_SM_CLI)
    if (start->service.cli != NULL)
    {

        /* Check required callbacks */
        if (start->service.cli->request_cb == NULL)
        {
            error = CCAPI_START_ERROR_INVALID_CLI_REQUEST_CALLBACK;
            goto done;
        }

        ccapi_data->config.cli_supported = CCAPI_TRUE;
        ccapi_data->service.cli.user_callbacks.request_cb = start->service.cli->request_cb;
        ccapi_data->service.cli.user_callbacks.finished_cb = start->service.cli->finished_cb;
    }
#endif
#endif

    ccapi_data->connector_handle = connector_init(ccapi_connector_callback, ccapi_data);
    error = check_malloc(ccapi_data->connector_handle);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
#if (defined CCIMP_UDP_TRANSPORT_ENABLED)
    ccapi_data->transport_udp.started = CCAPI_FALSE;
#endif
#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
    ccapi_data->transport_sms.started = CCAPI_FALSE;
#endif
    {
        ccapi_data->thread.connector_run = ccapi_malloc(sizeof *ccapi_data->thread.connector_run);
        error = check_malloc(ccapi_data->thread.connector_run);
        if (error != CCAPI_START_ERROR_NONE)
            goto done;

        ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_START;
        ccapi_data->thread.connector_run->ccimp_info.argument = ccapi_data;
        ccapi_data->thread.connector_run->ccimp_info.start = ccapi_connector_run_thread;
        ccapi_data->thread.connector_run->ccimp_info.type = CCIMP_THREAD_CONNECTOR_RUN;

        if (ccimp_os_create_thread(&ccapi_data->thread.connector_run->ccimp_info) != CCIMP_STATUS_OK)
        {
            error = CCAPI_START_ERROR_THREAD_FAILED;
            goto done;
        }

        do
        {
            ccimp_os_yield();
        } while (ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_START);
    }

    ccapi_data->initiate_action_syncr = ccapi_syncr_create_and_release();
    if (ccapi_data->initiate_action_syncr == NULL)
    {
        error = CCAPI_START_ERROR_SYNCR_FAILED;
        goto done;
    }

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
    ccapi_data->file_system_syncr = ccapi_syncr_create_and_release();
    if (ccapi_data->file_system_syncr == NULL)
    {
        error = CCAPI_START_ERROR_SYNCR_FAILED;
        goto done;
    }
#endif

done:
    switch (error)
    {
        case CCAPI_START_ERROR_NONE:
            break;
        case CCAPI_START_ERROR_NULL_PARAMETER:
        case CCAPI_START_ERROR_INVALID_VENDORID:
        case CCAPI_START_ERROR_INVALID_DEVICEID:
        case CCAPI_START_ERROR_INVALID_URL:
        case CCAPI_START_ERROR_INVALID_DEVICETYPE:
        case CCAPI_START_ERROR_INVALID_CLI_REQUEST_CALLBACK:
        case CCAPI_START_ERROR_INVALID_FIRMWARE_INFO:
        case CCAPI_START_ERROR_INVALID_FIRMWARE_DATA_CALLBACK:
        case CCAPI_START_ERROR_INSUFFICIENT_MEMORY:
        case CCAPI_START_ERROR_THREAD_FAILED:
        case CCAPI_START_ERROR_SYNCR_FAILED:
        case CCAPI_START_ERROR_ALREADY_STARTED:
            if (ccapi_data != NULL)
            {
                free_ccapi_data_internal_resources(ccapi_data);
                ccapi_free(ccapi_data);
            }
            break;
    }

    ccapi_logging_line("ccapi_start ret %d", error);

    return error;
}

static ccapi_transport_stop_t ccapi_stop_to_ccapi_transport_stop(ccapi_stop_t const stop_behavior)
{
    ccapi_transport_stop_t transport_stop_behavior;

    switch(stop_behavior)
    {
        case CCAPI_STOP_GRACEFULLY:
            transport_stop_behavior = CCAPI_TRANSPORT_STOP_GRACEFULLY;
            break;
        case CCAPI_STOP_IMMEDIATELY:
            transport_stop_behavior = CCAPI_TRANSPORT_STOP_IMMEDIATELY;
            break;  
    }

    return transport_stop_behavior;
}

ccapi_stop_error_t ccxapi_stop(ccapi_handle_t const ccapi_handle, ccapi_stop_t const behavior)
{
    ccapi_stop_error_t error = CCAPI_STOP_ERROR_NOT_STARTED;
    ccapi_data_t * const ccapi_data = (ccapi_data_t *)ccapi_handle;

    if (!CCAPI_RUNNING(ccapi_data))
    {
        goto done;
    }

    if (ccapi_data->transport_tcp.connected)
    {
        ccapi_tcp_stop_t tcp_stop;
        ccapi_tcp_stop_error_t tcp_stop_error;

        tcp_stop.behavior = ccapi_stop_to_ccapi_transport_stop(behavior);
        tcp_stop_error = ccxapi_stop_transport_tcp(ccapi_handle, &tcp_stop);
        switch(tcp_stop_error)
        {
            case CCAPI_TCP_STOP_ERROR_NONE:
                break;
            case CCAPI_TCP_STOP_ERROR_CCFSM:
                ccapi_logging_line("ccapi_stop: failed to stop TCP transport!");
                break;
            case CCAPI_TCP_STOP_ERROR_NOT_STARTED:
                ASSERT_MSG(tcp_stop_error != CCAPI_TCP_STOP_ERROR_NOT_STARTED);
                break;
        }
    }

#if (defined CCIMP_UDP_TRANSPORT_ENABLED)
    if (ccapi_data->transport_udp.started)
    {
        ccapi_udp_stop_t udp_stop;
        ccapi_udp_stop_error_t udp_stop_error;

        udp_stop.behavior = ccapi_stop_to_ccapi_transport_stop(behavior);
        udp_stop_error = ccxapi_stop_transport_udp(ccapi_handle, &udp_stop);
        switch(udp_stop_error)
        {
            case CCAPI_UDP_STOP_ERROR_NONE:
                break;
            case CCAPI_UDP_STOP_ERROR_CCFSM:
                ccapi_logging_line("ccapi_stop: failed to stop UDP transport!");
                break;
            case CCAPI_UDP_STOP_ERROR_NOT_STARTED:
                ASSERT_MSG(udp_stop_error != CCAPI_UDP_STOP_ERROR_NONE);
        }
    }
#endif

#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
    if (ccapi_data->transport_sms.started)
    {
        ccapi_sms_stop_t sms_stop;
        ccapi_sms_stop_error_t sms_stop_error;

        sms_stop.behavior = ccapi_stop_to_ccapi_transport_stop(behavior);
        sms_stop_error = ccxapi_stop_transport_sms(ccapi_handle, &sms_stop);
        switch(sms_stop_error)
        {
            case CCAPI_SMS_STOP_ERROR_NONE:
                break;
            case CCAPI_SMS_STOP_ERROR_CCFSM:
                ccapi_logging_line("ccapi_stop: failed to stop SMS transport!");
                break;
            case CCAPI_SMS_STOP_ERROR_NOT_STARTED:
                ASSERT_MSG(sms_stop_error != CCAPI_SMS_STOP_ERROR_NONE);
        }
    }
#endif

    {
        connector_status_t connector_status = connector_initiate_action_secure(ccapi_data, connector_initiate_terminate, NULL);
        switch(connector_status)
        {
        case connector_success:
            error = CCAPI_STOP_ERROR_NONE;
            break;
        case connector_init_error:
        case connector_invalid_data_size:
        case connector_invalid_data_range:
        case connector_invalid_data:
        case connector_keepalive_error:
        case connector_bad_version:
        case connector_device_terminated:
        case connector_service_busy:
        case connector_invalid_response:
        case connector_no_resource:
        case connector_unavailable:
        case connector_idle:
        case connector_working:
        case connector_pending:
        case connector_active:
        case connector_abort:
        case connector_device_error:
        case connector_exceed_timeout:
        case connector_invalid_payload_packet:
        case connector_open_error:
            ASSERT_MSG(connector_status != connector_success);
            break;
        }
    }

    do {
        ccimp_os_yield();
    } while (ccapi_data->thread.connector_run->status != CCAPI_THREAD_NOT_STARTED);

done:
    switch (error)
    {
        case CCAPI_STOP_ERROR_NONE:
            free_ccapi_data_internal_resources(ccapi_data);
            ccapi_free(ccapi_data);
            break;
        case CCAPI_STOP_ERROR_NOT_STARTED:
            break;
    }

    return error;
}

ccapi_start_error_t ccapi_start(ccapi_start_t const * const start)
{
	ccapi_start_error_t error;

    error = ccxapi_start((ccapi_handle_t *)&ccapi_data_single_instance, start);

    switch (error)
    {
        case CCAPI_START_ERROR_NONE:
            break;
        case CCAPI_START_ERROR_NULL_PARAMETER:
        case CCAPI_START_ERROR_INVALID_VENDORID:
        case CCAPI_START_ERROR_INVALID_DEVICEID:
        case CCAPI_START_ERROR_INVALID_URL:
        case CCAPI_START_ERROR_INVALID_DEVICETYPE:
        case CCAPI_START_ERROR_INVALID_CLI_REQUEST_CALLBACK:
        case CCAPI_START_ERROR_INVALID_FIRMWARE_INFO:
        case CCAPI_START_ERROR_INVALID_FIRMWARE_DATA_CALLBACK:
        case CCAPI_START_ERROR_INSUFFICIENT_MEMORY:
        case CCAPI_START_ERROR_THREAD_FAILED:
        case CCAPI_START_ERROR_SYNCR_FAILED:
        case CCAPI_START_ERROR_ALREADY_STARTED:
            ccapi_data_single_instance = NULL;
            break;
    }

	return error;
}

ccapi_stop_error_t ccapi_stop(ccapi_stop_t const behavior)
{
    ccapi_stop_error_t error;

    error = ccxapi_stop((ccapi_handle_t)ccapi_data_single_instance, behavior);
    switch (error)
    {
        case CCAPI_STOP_ERROR_NONE:
            ccapi_data_single_instance = NULL;
            break;
        case CCAPI_STOP_ERROR_NOT_STARTED:
            break;
    }

    return error;
}

