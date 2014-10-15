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

/* TODO: create a singe function to convert from CCAPI error to CCFSM error */

static void free_and_stop_service(ccapi_data_t * const ccapi_data)
{
    if (ccapi_data->service.firmware_update.service.chunk_data != NULL)
    {
        ccapi_free(ccapi_data->service.firmware_update.service.chunk_data);
        ccapi_data->service.firmware_update.service.chunk_data = NULL;
    }

    ccapi_data->service.firmware_update.service.update_started = CCAPI_FALSE;
}

static connector_callback_status_t ccapi_process_firmware_update_start(connector_firmware_download_start_t * const start_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;
    ccapi_firmware_update_error_t ccapi_firmware_update_error;

    ASSERT_MSG_GOTO(start_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

    connector_status = connector_callback_continue;
    start_ptr->status = connector_firmware_status_success;

    ccapi_logging_line("ccapi_process_firmware_update_start for target_number = '%d'. code_size=%d", start_ptr->target_number, start_ptr->code_size);

    if (ccapi_data->service.firmware_update.service.update_started == CCAPI_TRUE)
    {
        start_ptr->status = connector_firmware_status_device_error;
        goto done;
    }

    if (start_ptr->code_size > ccapi_data->service.firmware_update.target.list[start_ptr->target_number].maximum_size)
    {
        start_ptr->status = connector_firmware_status_download_invalid_size;
        goto done;
    }

    if (ccapi_data->service.firmware_update.user_callbacks.request_cb != NULL)
    {
        ccapi_firmware_update_error = ccapi_data->service.firmware_update.user_callbacks.request_cb(start_ptr->target_number, start_ptr->filename, start_ptr->code_size);

        switch (ccapi_firmware_update_error)
        {
            case CCAPI_FIRMWARE_UPDATE_ERROR_NONE:
                break;    
            case CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD:
            case CCAPI_FIRMWARE_UPDATE_ERROR_INVALID_DATA: /* should not happen */
                start_ptr->status = connector_firmware_status_download_denied;
                break;    
        }
    } 

    ccapi_data->service.firmware_update.service.chunk_data = ccapi_malloc(ccapi_data->service.firmware_update.target.list[start_ptr->target_number].chunk_size);
    if (ccapi_data->service.firmware_update.service.chunk_data == NULL)
    {
        start_ptr->status = connector_firmware_status_encountered_error;
        goto done;
    }

    ccapi_data->service.firmware_update.service.total_size = start_ptr->code_size;
    ccapi_data->service.firmware_update.service.head_offset = 0;
    ccapi_data->service.firmware_update.service.bottom_offset = 0;
    ccapi_data->service.firmware_update.service.update_started = CCAPI_TRUE;
    

done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_firmware_update_data(connector_firmware_download_data_t * const data_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;
    ccapi_firmware_update_error_t ccapi_firmware_update_error;

    ASSERT_MSG_GOTO(data_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

    connector_status = connector_callback_continue;
    data_ptr->status = connector_firmware_status_success;

    /*ccapi_logging_line("ccapi_process_firmware_update_data for target_number = '%d'", data_ptr->target_number);*/

    if (ccapi_data->service.firmware_update.service.update_started == CCAPI_FALSE)
    {
        data_ptr->status = connector_firmware_status_device_error;
        goto done;
    }

    {
        const uint32_t chunk_size = ccapi_data->service.firmware_update.target.list[data_ptr->target_number].chunk_size;

        /* Source */
        uint8_t * source_data = (uint8_t *)data_ptr->image.data;
        size_t source_bytes_remaining = data_ptr->image.bytes_used;

        if (data_ptr->image.offset != ccapi_data->service.firmware_update.service.head_offset)
        {
            data_ptr->status = connector_firmware_status_invalid_offset;
            goto done;
        }

        while (source_bytes_remaining)
        {
            const uint32_t room_in_chunk = chunk_size - ccapi_data->service.firmware_update.service.head_offset % chunk_size;
            const uint32_t bytes_to_copy = source_bytes_remaining > room_in_chunk ? room_in_chunk : source_bytes_remaining;
            const uint32_t next_offset = ccapi_data->service.firmware_update.service.head_offset + bytes_to_copy;
            const ccapi_bool_t last_chunk = next_offset == ccapi_data->service.firmware_update.service.total_size ? CCAPI_TRUE : CCAPI_FALSE;

            memcpy(&ccapi_data->service.firmware_update.service.chunk_data[ccapi_data->service.firmware_update.service.head_offset % chunk_size], source_data, bytes_to_copy);

            source_bytes_remaining -= bytes_to_copy;
            source_data += bytes_to_copy;

            if ((next_offset - ccapi_data->service.firmware_update.service.bottom_offset) == chunk_size)
            {
                ccapi_firmware_update_error = ccapi_data->service.firmware_update.user_callbacks.data_cb(data_ptr->target_number, 
                                                                                                         ccapi_data->service.firmware_update.service.bottom_offset, 
                                                                                                         ccapi_data->service.firmware_update.service.chunk_data, 
                                                                                                         chunk_size, 
                                                                                                         last_chunk);
                switch (ccapi_firmware_update_error)
                {
                    case CCAPI_FIRMWARE_UPDATE_ERROR_NONE:
                        break;    
                    case CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD: /* should not happen */
                    case CCAPI_FIRMWARE_UPDATE_ERROR_INVALID_DATA:
                        data_ptr->status = connector_firmware_status_download_denied;
                        break;    
                }
                ccapi_data->service.firmware_update.service.bottom_offset = next_offset;
            }
            ccapi_data->service.firmware_update.service.head_offset = next_offset;
        }
    } 

done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_firmware_update_complete(connector_firmware_download_complete_t * const complete_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;
    ccapi_firmware_update_error_t ccapi_firmware_update_error;

    ASSERT_MSG_GOTO(complete_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

    connector_status = connector_callback_continue;
    complete_ptr->status = connector_firmware_download_success;

    ccapi_logging_line("ccapi_process_firmware_update_complete for target_number = '%d'", complete_ptr->target_number);

    if (ccapi_data->service.firmware_update.service.update_started == CCAPI_FALSE)
    {
        ccapi_logging_line("update_complete arrived before all firmware data arrived!");

        complete_ptr->status = connector_firmware_download_not_complete;
        goto done;
    }


    if (ccapi_data->service.firmware_update.service.head_offset != ccapi_data->service.firmware_update.service.total_size)
    {
        ccapi_logging_line("update_complete arrived before all firmware data arrived!");

        complete_ptr->status = connector_firmware_download_not_complete;
        goto done;
    }

    {
        const uint32_t chunk_size = ccapi_data->service.firmware_update.target.list[complete_ptr->target_number].chunk_size;
        const uint32_t remaining_bytes = ccapi_data->service.firmware_update.service.head_offset % chunk_size;

        if (remaining_bytes)
        {
            ccapi_firmware_update_error = ccapi_data->service.firmware_update.user_callbacks.data_cb(complete_ptr->target_number, 
                                                                                                     ccapi_data->service.firmware_update.service.bottom_offset, 
                                                                                                     ccapi_data->service.firmware_update.service.chunk_data, 
                                                                                                     remaining_bytes,
                                                                                                     CCAPI_TRUE);
            switch (ccapi_firmware_update_error)
            {
                case CCAPI_FIRMWARE_UPDATE_ERROR_NONE:
                    break;    
                case CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD: /* should not happen */
                case CCAPI_FIRMWARE_UPDATE_ERROR_INVALID_DATA:
                    complete_ptr->status = connector_firmware_download_not_complete;
                    break;    
            }
        }
    }

    free_and_stop_service(ccapi_data);
    
done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_firmware_update_abort(connector_firmware_download_abort_t * const abort_ptr, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(abort_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

    connector_status = connector_callback_continue;

    ccapi_logging_line("ccapi_process_firmware_update_abort for target_number = '%d'. status=%d", abort_ptr->target_number, abort_ptr->status);

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

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported == CCAPI_TRUE, done);

            count_ptr->count = ccapi_data->service.firmware_update.target.count;
           
            connector_status = connector_callback_continue;

            break;
        }

        case connector_request_id_firmware_info:
        {
            connector_firmware_info_t * const info_ptr = data;

            ASSERT_MSG_GOTO(ccapi_data->config.firmware_supported == CCAPI_TRUE, done);

            ASSERT_MSG_GOTO(info_ptr->target_number < ccapi_data->service.firmware_update.target.count, done);

            info_ptr->version.major = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.major;
            info_ptr->version.minor = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.minor;
            info_ptr->version.revision = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.revision;
            info_ptr->version.build = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].version.build;

            info_ptr->description = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].description;
            info_ptr->filespec = ccapi_data->service.firmware_update.target.list[info_ptr->target_number].filespec;
           
            connector_status = connector_callback_continue;

            break;
        }

        case connector_request_id_firmware_download_start:
        {
            connector_firmware_download_start_t * const start_ptr = data;

            connector_status = ccapi_process_firmware_update_start(start_ptr, ccapi_data);

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

            (void)data;

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