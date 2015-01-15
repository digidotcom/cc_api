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

#if (defined CCIMP_FIRMWARE_SERVICE_ENABLED)

static void free_and_stop_service(ccapi_data_t * const ccapi_data)
{
    unsigned char chunk_pool_index;

    for (chunk_pool_index = 0; chunk_pool_index < CCAPI_CHUNK_POOL_SIZE; chunk_pool_index++)
    {
        if (ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].data != NULL)
        {
            ccapi_free(ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].data);
            ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].data = NULL;
        }
    }

    ccapi_data->service.firmware_update.processing.update_started = CCAPI_FALSE;
}

static connector_callback_status_t ccapi_process_firmware_update_request(connector_firmware_download_start_t * const start_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;
    ccapi_fw_request_error_t ccapi_fw_request_error;
    ccapi_firmware_target_t * fw_target_item = NULL;
    unsigned char chunk_pool_index;

    ASSERT_MSG_GOTO(start_ptr->target_number < ccapi_data->service.firmware_update.config.target.count, done);

    connector_status = connector_callback_continue;
    start_ptr->status = connector_firmware_status_success;

    ccapi_logging_line("ccapi_process_firmware_update_request for target_number='%d'. code_size='%d'", start_ptr->target_number, start_ptr->code_size);

    if (ccapi_data->service.firmware_update.processing.update_started)
    {
        free_and_stop_service(ccapi_data);

        start_ptr->status = connector_firmware_status_encountered_error;
        goto done;
    }

    fw_target_item = &ccapi_data->service.firmware_update.config.target.item[start_ptr->target_number];

    if (fw_target_item->maximum_size != 0 && 
        fw_target_item->maximum_size < start_ptr->code_size)
    {
        start_ptr->status = connector_firmware_status_download_invalid_size;
        goto done;
    }

    if (ccapi_data->service.firmware_update.config.callback.request_cb != NULL)
    {
        ccapi_fw_request_error = ccapi_data->service.firmware_update.config.callback.request_cb(start_ptr->target_number, start_ptr->filename, start_ptr->code_size);

        switch (ccapi_fw_request_error)
        {
            case CCAPI_FW_REQUEST_ERROR_NONE:
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_DENIED:
                start_ptr->status = connector_firmware_status_download_denied;
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_INVALID_SIZE:
                start_ptr->status = connector_firmware_status_download_invalid_size;
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_INVALID_VERSION:
                start_ptr->status = connector_firmware_status_download_invalid_version;
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_UNAUTHENTICATED:
                start_ptr->status = connector_firmware_status_download_unauthenticated;
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_NOT_ALLOWED:
                start_ptr->status = connector_firmware_status_download_not_allowed;
                break;
            case CCAPI_FW_REQUEST_ERROR_DOWNLOAD_CONFIGURED_TO_REJECT:
                start_ptr->status = connector_firmware_status_download_configured_to_reject;
                break;
            case CCAPI_FW_REQUEST_ERROR_ENCOUNTERED_ERROR:
                start_ptr->status = connector_firmware_status_encountered_error;
                break;
        }
    } 

    if (fw_target_item->chunk_size == 0)
    {
        fw_target_item->chunk_size = 1024;
    }

    for (chunk_pool_index = 0; chunk_pool_index < CCAPI_CHUNK_POOL_SIZE; chunk_pool_index++)
    {
        ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].in_use = CCAPI_FALSE;
        ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].offset = 0;

        ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].data = ccapi_malloc(fw_target_item->chunk_size);
        if (ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].data == NULL)
        {
            start_ptr->status = connector_firmware_status_encountered_error;
            goto done;
        }

        ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].size = 0;
        ccapi_data->service.firmware_update.processing.chunk_pool[chunk_pool_index].last = CCAPI_FALSE;
    }
    ccapi_data->service.firmware_update.processing.current_chunk = 0;

    ccapi_data->service.firmware_update.processing.target = start_ptr->target_number;
    ccapi_data->service.firmware_update.processing.total_size = start_ptr->code_size;
    ccapi_data->service.firmware_update.processing.tail_offset = 0;
    ccapi_data->service.firmware_update.processing.head_offset = 0;
    ccapi_data->service.firmware_update.processing.source_bytes_processed = 0;
    ccapi_data->service.firmware_update.processing.data_error = CCAPI_FW_DATA_ERROR_NONE;
    ccapi_data->service.firmware_update.processing.update_started = CCAPI_TRUE;

done:
    return connector_status;
}

void ccapi_firmware_thread(void * const argument)
{
    ccapi_data_t * const ccapi_data = argument;

    /* ccapi_data is corrupted, it's likely the implementer made it wrong passing argument to the new thread */
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

    ccapi_data->thread.firmware->status = CCAPI_THREAD_RUNNING;
    while (ccapi_data->thread.firmware->status == CCAPI_THREAD_RUNNING)
    {
        ccapi_fw_data_error_t ccapi_fw_data_error;
        ccapi_fw_chunk_info * const chunk = &ccapi_data->service.firmware_update.processing.chunk_pool[ccapi_data->service.firmware_update.processing.current_chunk];

        if (chunk->in_use == CCAPI_TRUE)
        {
            ccapi_logging_line("+ccapi_firmware_thread: processing offset=0x%x, size=%d, last=%d", chunk->offset, chunk->size, chunk->last);
            ccapi_fw_data_error = ccapi_data->service.firmware_update.config.callback.data_cb(ccapi_data->service.firmware_update.processing.target,
                                                                                                     chunk->offset, chunk->data, chunk->size, chunk->last);
            switch (ccapi_fw_data_error)
            {
                case CCAPI_FW_DATA_ERROR_NONE:
                    break;    
                case CCAPI_FW_DATA_ERROR_INVALID_DATA:
                    ccapi_data->service.firmware_update.processing.data_error = ccapi_fw_data_error;
                    break;    
            }

            chunk->in_use = CCAPI_FALSE;

            ccapi_logging_line("-ccapi_firmware_thread: processing offset=0x%x, size=%d, last=%d", chunk->offset, chunk->size, chunk->last);
        }

        ccimp_os_yield();
    }
    ASSERT_MSG_GOTO(ccapi_data->thread.firmware->status == CCAPI_THREAD_REQUEST_STOP, done);

    ccapi_data->thread.firmware->status = CCAPI_THREAD_NOT_STARTED;
done:
    return;
}

static connector_callback_status_t ccapi_process_firmware_update_data(connector_firmware_download_data_t * const data_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(data_ptr->target_number == ccapi_data->service.firmware_update.processing.target, done);

    ASSERT_MSG_GOTO(ccapi_data->service.firmware_update.processing.update_started, done);

    connector_status = connector_callback_continue;
    data_ptr->status = connector_firmware_status_success;

    /* ccapi_logging_line("ccapi_process_fw_data target_number=%d, offset=0x%x, size=%d", data_ptr->target_number, data_ptr->image.offset, data_ptr->image.bytes_used); */

    if (ccapi_data->service.firmware_update.processing.data_error == CCAPI_FW_DATA_ERROR_INVALID_DATA)
    {
        ccapi_logging_line("Invalid data!");

        data_ptr->status = connector_firmware_status_invalid_data;
        goto done;
    }

    if (data_ptr->image.offset != ccapi_data->service.firmware_update.processing.tail_offset - ccapi_data->service.firmware_update.processing.source_bytes_processed)
    {
        ccapi_logging_line("Out of order packet: offset 0x%x != 0x%x !", data_ptr->image.offset, ccapi_data->service.firmware_update.processing.tail_offset);

        data_ptr->status = connector_firmware_status_invalid_offset;
        goto done;
    }

    if (ccapi_data->service.firmware_update.processing.chunk_pool[ccapi_data->service.firmware_update.processing.current_chunk].in_use == CCAPI_FALSE)
    {
        uint32_t const chunk_size = ccapi_data->service.firmware_update.config.target.item[data_ptr->target_number].chunk_size;
        uint32_t const tail_offset = ccapi_data->service.firmware_update.processing.tail_offset;
        uint32_t const room_in_chunk = chunk_size - tail_offset % chunk_size;
        uint8_t const * const source_data = (uint8_t *)data_ptr->image.data + ccapi_data->service.firmware_update.processing.source_bytes_processed;
        size_t const source_bytes_remaining = data_ptr->image.bytes_used - ccapi_data->service.firmware_update.processing.source_bytes_processed;
        uint32_t const bytes_to_copy = source_bytes_remaining > room_in_chunk ? room_in_chunk : source_bytes_remaining;
        uint32_t const next_head_offset = tail_offset + bytes_to_copy;
        ccapi_bool_t const last_chunk = CCAPI_BOOL(next_head_offset == ccapi_data->service.firmware_update.processing.total_size);

        ASSERT_MSG(bytes_to_copy != 0);

        if (tail_offset >= ccapi_data->service.firmware_update.processing.head_offset)
        {
            memcpy(&ccapi_data->service.firmware_update.processing.chunk_pool[ccapi_data->service.firmware_update.processing.current_chunk].data[tail_offset % chunk_size], source_data, bytes_to_copy);

            if (last_chunk || next_head_offset % chunk_size == 0)
            {
                ccapi_fw_chunk_info * const chunk = &ccapi_data->service.firmware_update.processing.chunk_pool[ccapi_data->service.firmware_update.processing.current_chunk];

                chunk->offset = ccapi_data->service.firmware_update.processing.head_offset;
                chunk->size = next_head_offset - ccapi_data->service.firmware_update.processing.head_offset;
                ccapi_data->service.firmware_update.processing.head_offset = next_head_offset;
                chunk->last = last_chunk;
                ccapi_logging_line("ccapi_process_fw_data queued offset=0x%x, size=%d, last=%d", chunk->offset, chunk->size, chunk->last);
                chunk->in_use = CCAPI_TRUE;
            }

            ccapi_data->service.firmware_update.processing.source_bytes_processed += bytes_to_copy;
            if (ccapi_data->service.firmware_update.processing.source_bytes_processed == data_ptr->image.bytes_used)
            {
                ccapi_data->service.firmware_update.processing.source_bytes_processed = 0;
            }
            else
            {
                connector_status = connector_callback_busy;
            }
        }
        ccapi_data->service.firmware_update.processing.tail_offset = next_head_offset;
    } 
    else
    {
        connector_status = connector_callback_busy;
    }

done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_firmware_update_complete(connector_firmware_download_complete_t * const complete_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(complete_ptr->target_number == ccapi_data->service.firmware_update.processing.target, done);

    ASSERT_MSG_GOTO(ccapi_data->service.firmware_update.processing.update_started, done);

    connector_status = connector_callback_continue;
    complete_ptr->status = connector_firmware_download_success;

    ccapi_logging_line("ccapi_process_firmware_update_complete for target_number='%d'", complete_ptr->target_number);

    if (ccapi_data->service.firmware_update.processing.head_offset != ccapi_data->service.firmware_update.processing.total_size)
    {
        ccapi_logging_line("update_complete arrived before all firmware data arrived!");

        complete_ptr->status = connector_firmware_download_not_complete;
    }

    free_and_stop_service(ccapi_data);
    
done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_firmware_update_abort(connector_firmware_download_abort_t * const abort_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(abort_ptr->target_number == ccapi_data->service.firmware_update.processing.target, done);

    connector_status = connector_callback_continue;

    ccapi_logging_line("ccapi_process_firmware_update_abort for target_number='%d'. status='%d'", abort_ptr->target_number, abort_ptr->status);

    if (ccapi_data->service.firmware_update.config.callback.cancel_cb != NULL)
    {
        ccapi_fw_cancel_error_t cancel_reason;

        switch (abort_ptr->status)
        {
            case connector_firmware_status_user_abort:
                cancel_reason = CCAPI_FW_CANCEL_USER_ABORT;
                break;
            case connector_firmware_status_device_error:
                cancel_reason = CCAPI_FW_CANCEL_DEVICE_ERROR;
                break;
            case connector_firmware_status_invalid_offset:
                cancel_reason = CCAPI_FW_CANCEL_INVALID_OFFSET;
                break;
            case connector_firmware_status_invalid_data:
                cancel_reason = CCAPI_FW_CANCEL_INVALID_DATA;
                break;
            case connector_firmware_status_hardware_error:
                cancel_reason = CCAPI_FW_CANCEL_HARDWARE_ERROR;
                break;
            case connector_firmware_status_success:
            case connector_firmware_status_download_denied:
            case connector_firmware_status_download_invalid_size:
            case connector_firmware_status_download_invalid_version:
            case connector_firmware_status_download_unauthenticated:
            case connector_firmware_status_download_not_allowed:
            case connector_firmware_status_download_configured_to_reject:
            case connector_firmware_status_encountered_error:
                connector_status = connector_callback_error;
                ASSERT_MSG_GOTO(abort_ptr->status >= connector_firmware_status_user_abort, done);
        }

        ccapi_data->service.firmware_update.config.callback.cancel_cb(abort_ptr->target_number, cancel_reason);
    }

    free_and_stop_service(ccapi_data);

done:
    return connector_status;
}

connector_callback_status_t ccapi_firmware_service_handler(connector_request_id_firmware_t const firmware_service_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    switch (firmware_service_request)
    {
        case connector_request_id_firmware_target_count:
        {
            connector_firmware_count_t * const count_ptr = data;

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported, done);

            count_ptr->count = ccapi_data->service.firmware_update.config.target.count;
           
            connector_status = connector_callback_continue;

            break;
        }
        case connector_request_id_firmware_info:
        {
            connector_firmware_info_t * const info_ptr = data;

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported, done);

            ASSERT_MSG_GOTO(info_ptr->target_number < ccapi_data->service.firmware_update.config.target.count, done);

            info_ptr->version.major = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].version.major;
            info_ptr->version.minor = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].version.minor;
            info_ptr->version.revision = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].version.revision;
            info_ptr->version.build = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].version.build;

            info_ptr->description = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].description;
            info_ptr->filespec = ccapi_data->service.firmware_update.config.target.item[info_ptr->target_number].filespec;
           
            connector_status = connector_callback_continue;

            break;
        }
        case connector_request_id_firmware_download_start:
        {
            connector_firmware_download_start_t * const start_ptr = data;

            connector_status = ccapi_process_firmware_update_request(start_ptr, ccapi_data);

            break;
        }
        case connector_request_id_firmware_download_data:
        {
            connector_firmware_download_data_t * const data_ptr = data;

            connector_status = ccapi_process_firmware_update_data(data_ptr, ccapi_data);

            break;
        }
        case connector_request_id_firmware_download_complete:
        {
            connector_firmware_download_complete_t * const complete_ptr = data;

            connector_status = ccapi_process_firmware_update_complete(complete_ptr, ccapi_data);

            break;
        }
        case connector_request_id_firmware_download_abort:
        {
            connector_firmware_download_abort_t * const abort_ptr = data;

            connector_status = ccapi_process_firmware_update_abort(abort_ptr, ccapi_data);

            break;
        }
        case connector_request_id_firmware_target_reset:
        {
            ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

            ccapi_logging_line("ccapi_process_firmware_update_reset");

            UNUSED_ARGUMENT(data);

            ccimp_status = ccimp_hal_reset();
            connector_status = connector_callback_continue;
            connector_status = connector_callback_status_from_ccimp_status(ccimp_status);

            break;  
        }
    }

    ASSERT_MSG_GOTO(connector_status != connector_callback_unrecognized, done);

done:
    return connector_status;
}
#endif