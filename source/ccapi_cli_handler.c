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

#if (defined CCIMP_UDP_TRANSPORT_ENABLED || defined CCIMP_SMS_TRANSPORT_ENABLED)

#if (defined CONNECTOR_SM_CLI)
static ccapi_bool_t valid_cli_malloc(void * * ptr, size_t size, ccapi_cli_error_t * const error)
{
    ccapi_bool_t success;
 
    *ptr = ccapi_malloc(size);

    success = *ptr == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    
    if (!success)
    {
        *error = CCAPI_CLI_ERROR_INSUFFICIENT_MEMORY;
    }

    return success;
}

static connector_callback_status_t ccapi_process_cli_request(connector_sm_cli_request_t * const cli_request_ptr, ccapi_data_t * const ccapi_data)
{
    ccapi_svc_cli_t * svc_cli = NULL;
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(cli_request_ptr->buffer != NULL && cli_request_ptr->bytes_used, done);

    ccapi_logging_line("ccapi_process_cli_request");

    if(cli_request_ptr->user_context == NULL)
    {
        if (!valid_cli_malloc((void**)&svc_cli, sizeof *svc_cli, &svc_cli->cli_error))
        {
            /* We didn't manage to create a user_context. ccfsm will call response and status callbacks without it */
            goto done;
        }

        cli_request_ptr->user_context = svc_cli;

/*
        svc_cli->max_request_size = CCAPI_RECEIVE_NO_LIMIT;
*/
        svc_cli->request_string_info.string = NULL;
        svc_cli->request_string_info.length = 0;
        svc_cli->response_string_info.string = NULL;
        svc_cli->response_string_info.length = 0;
        svc_cli->response_handled_internally = CCAPI_FALSE;
        svc_cli->response_processing.string = NULL;
        svc_cli->response_processing.length = 0;
        svc_cli->response_required = CCAPI_BOOL(cli_request_ptr->response_required);
        svc_cli->more_data = CCAPI_BOOL(cli_request_ptr->more_data);
        svc_cli->cli_error = CCAPI_CLI_ERROR_NONE;
    }
    else
    {
        svc_cli = cli_request_ptr->user_context;

        ASSERT_MSG_GOTO(svc_cli->more_data, done);

        svc_cli->more_data = CCAPI_BOOL(cli_request_ptr->more_data);
    }

    if (!ccapi_data->config.cli_supported)
    {
        svc_cli->cli_error = CCAPI_CLI_ERROR_NO_CLI_SUPPORT;
        goto done;
    }

    {
        ccimp_os_realloc_t ccimp_realloc_data;

        ccimp_realloc_data.new_size = svc_cli->request_string_info.length + cli_request_ptr->bytes_used; /* buffer is already null terminated */

#if 0
        if (svc_cli->max_request_size != CCAPI_RECEIVE_NO_LIMIT && ccimp_realloc_data.new_size > svc_cli->max_request_size)
        {
            ccapi_logging_line("ccapi_process_cli_request: request excess max_request_size (%d) for this command", svc_cli->max_request_size);

            svc_cli->cli_error = CCAPI_CLI_ERROR_REQUEST_TOO_BIG;
            goto done;
        }
#endif

        ccimp_realloc_data.old_size = svc_cli->request_string_info.length;
        ccimp_realloc_data.ptr = svc_cli->request_string_info.string;
        if (ccimp_os_realloc(&ccimp_realloc_data) != CCIMP_STATUS_OK)
        {
            ccapi_logging_line("ccapi_process_cli_request: error ccimp_os_realloc for %d bytes", ccimp_realloc_data.new_size);

            svc_cli->cli_error = CCAPI_CLI_ERROR_INSUFFICIENT_MEMORY;
            goto done;
        }
        svc_cli->request_string_info.string = ccimp_realloc_data.ptr;
 
        {
            uint8_t * const dest_addr = (uint8_t *)svc_cli->request_string_info.string + svc_cli->request_string_info.length;
            memcpy(dest_addr, cli_request_ptr->buffer, cli_request_ptr->bytes_used);
        }
        svc_cli->request_string_info.length += cli_request_ptr->bytes_used;
    }

    if (cli_request_ptr->more_data == connector_false)
    {
        ccapi_data->service.cli.user_callbacks.request_cb(cli_request_ptr->transport, 
                                                          svc_cli->request_string_info.string,
                                                          svc_cli->response_required ? (char const * *)&svc_cli->response_string_info.string : NULL);

        ccapi_free(svc_cli->request_string_info.string);

        if (svc_cli->response_required && svc_cli->response_string_info.string != NULL)
        {
            svc_cli->response_string_info.length = strlen(svc_cli->response_string_info.string);

            if (svc_cli->response_string_info.length)
            {
                memcpy(&svc_cli->response_processing, &svc_cli->response_string_info, sizeof svc_cli->response_string_info);
            }
        }
    }

    connector_status = connector_callback_continue;

done:
    return connector_status;
}
#endif

#if 0
static void fill_internal_error(ccapi_svc_receive_t * svc_receive)
{
#define ERROR_MESSAGE "CCAPI Error %d (%s) while handling target '%s'"

        char const * receive_error_str = NULL;
        size_t receive_error_str_len = 0;

        switch (svc_receive->receive_error)
        {
#define ENUM_TO_CASE_ERR(name)  case name:  receive_error_str = #name; receive_error_str_len = sizeof #name - 1; break
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_CCAPI_NOT_RUNNING);
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT);
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_INSUFFICIENT_MEMORY);
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_INVALID_DATA_CB);
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_USER_REFUSED_TARGET);
            ENUM_TO_CASE_ERR(CCAPI_RECEIVE_ERROR_REQUEST_TOO_BIG);
#undef ENUM_TO_CASE_ERR

            case CCAPI_RECEIVE_ERROR_NONE:
            case CCAPI_RECEIVE_ERROR_INVALID_TARGET:
            case CCAPI_RECEIVE_ERROR_TARGET_NOT_ADDED:
            case CCAPI_RECEIVE_ERROR_TARGET_ALREADY_ADDED:
            case CCAPI_RECEIVE_ERROR_SYNCR_FAILED:
            case CCAPI_RECEIVE_ERROR_STATUS_CANCEL:
            case CCAPI_RECEIVE_ERROR_STATUS_TIMEOUT:
            case CCAPI_RECEIVE_ERROR_STATUS_SESSION_ERROR:
            {
                static char const this_receive_error_str[] = "Unexpected error";
                receive_error_str = (char *)this_receive_error_str;
                receive_error_str_len = sizeof this_receive_error_str - 1;
                break;
            }
        }

        receive_error_str_len += sizeof ERROR_MESSAGE + strlen(svc_receive->target);

        if (!valid_receive_malloc(&svc_receive->response_buffer_info.buffer, receive_error_str_len, &svc_receive->receive_error))
        {
              return;
        }
        svc_receive->response_buffer_info.length = sprintf(svc_receive->response_buffer_info.buffer, ERROR_MESSAGE, 
                                                                        svc_receive->receive_error, receive_error_str, svc_receive->target);

        ccapi_logging_line("fill_internal_error: Providing response in buffer at %p: %s", 
                                    svc_receive->response_buffer_info.buffer, (char*)svc_receive->response_buffer_info.buffer);
}

static connector_callback_status_t ccapi_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_ptr)
{
    ccapi_svc_receive_t * const svc_receive = (ccapi_svc_receive_t *)reply_ptr->user_context;
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(svc_receive != NULL, done);

    ccapi_logging_line("ccapi_process_device_request_response for target = '%s'", svc_receive->target);

    if (!svc_receive->response_required)
    {
        goto done;
    }

    /* We initialize the response buffer for internal errors just once */
    if (svc_receive->receive_error != CCAPI_RECEIVE_ERROR_NONE && svc_receive->response_handled_internally == CCAPI_FALSE)
    {
        svc_receive->response_handled_internally = CCAPI_TRUE;

        fill_internal_error(svc_receive);
        memcpy(&svc_receive->response_processing, &svc_receive->response_buffer_info, sizeof svc_receive->response_buffer_info);
    }

    {
        size_t const bytes_to_send = svc_receive->response_processing.length > reply_ptr->bytes_available ?
                                                 reply_ptr->bytes_available : svc_receive->response_processing.length;

        memcpy(reply_ptr->buffer, svc_receive->response_processing.buffer, bytes_to_send);
        svc_receive->response_processing.buffer = ((char *)svc_receive->response_processing.buffer) + bytes_to_send;
 
        reply_ptr->bytes_used = bytes_to_send;
        svc_receive->response_processing.length -= reply_ptr->bytes_used;
        reply_ptr->more_data = svc_receive->response_processing.length > 0 ? connector_true : connector_false;
    }

    connector_status = connector_callback_continue;

done:
    return connector_status;
}

static connector_callback_status_t ccapi_process_device_request_status(connector_data_service_status_t const * const status_ptr, ccapi_data_t * const ccapi_data)
{
    ccapi_svc_receive_t * const svc_receive = (ccapi_svc_receive_t *)status_ptr->user_context;
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(svc_receive != NULL, done);

    ccapi_logging_line("ccapi_process_device_request_status for target = '%s'", svc_receive->target);
    ccapi_logging_line("ccapi_process_device_request_status: ccapi_receive_error= %d,  status: %d", svc_receive->receive_error, status_ptr->status);

    /* Prior reported errors by ccapi have priority over the ones reported by the cloud */
    if (svc_receive->receive_error == CCAPI_RECEIVE_ERROR_NONE)
    {
        switch (status_ptr->status)
        {
            case connector_data_service_status_complete:
                svc_receive->receive_error = CCAPI_RECEIVE_ERROR_NONE;
                break;
            case connector_data_service_status_cancel:
                svc_receive->receive_error = CCAPI_RECEIVE_ERROR_STATUS_CANCEL;
                break;
            case connector_data_service_status_timeout:
                svc_receive->receive_error = CCAPI_RECEIVE_ERROR_STATUS_TIMEOUT;
                break;
            case connector_data_service_status_session_error:
                svc_receive->receive_error = CCAPI_RECEIVE_ERROR_STATUS_SESSION_ERROR;
                ccapi_logging_line("ccapi_process_device_request_status: session_error=%d", status_ptr->session_error);
                break;
            case connector_data_service_status_COUNT:
                ASSERT_MSG_GOTO(status_ptr->status != connector_data_service_status_COUNT, done);
                break;
        }
    }

    /* Call the user so he can free allocated response memory and handle errors  */
    if (ccapi_data->config.receive_supported && svc_receive->user_callbacks.status_cb != NULL)
    {
       const ccapi_bool_t should_user_free_response_buffer = !svc_receive->response_handled_internally && svc_receive->response_required && svc_receive->response_buffer_info.buffer != NULL;
       svc_receive->user_callbacks.status_cb(svc_receive->target, status_ptr->transport, 
                           should_user_free_response_buffer ? &svc_receive->response_buffer_info : NULL, 
                           svc_receive->receive_error);
    }

    /* Free resources */
    if (svc_receive->target != NULL)
    {
        ccapi_free(svc_receive->target);
    }
    if (svc_receive->response_handled_internally == CCAPI_TRUE)
    {
        ccapi_logging_line("ccapi_process_device_request_status: Freeing response buffer at %p", svc_receive->response_buffer_info.buffer);
        ccapi_free(svc_receive->response_buffer_info.buffer);
    }
    ccapi_free(svc_receive);

    connector_status = connector_callback_continue;

done:
    return connector_status;
}


static connector_callback_status_t ccapi_process_device_request_length(connector_data_service_length_t * const length_ptr)
{
    ccapi_svc_receive_t const * const svc_receive = (ccapi_svc_receive_t *)length_ptr->user_context;
    connector_callback_status_t connector_status = connector_callback_error;

    ASSERT_MSG_GOTO(svc_receive != NULL, done);

    length_ptr->total_bytes = svc_receive->response_processing.length;

    connector_status = connector_callback_continue;

done:
    return connector_status;
}
#endif

connector_callback_status_t ccapi_sm_service_handler(connector_request_id_sm_t const sm_service_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;

    switch (sm_service_request)
    {
#if (defined CONNECTOR_SM_CLI)
        case connector_request_id_sm_cli_request:
        {
            connector_sm_cli_request_t * const cli_request_ptr = data;

            connector_status = ccapi_process_cli_request(cli_request_ptr, ccapi_data);

            break;
        }
#endif

        case connector_request_id_sm_ping_request:
        case connector_request_id_sm_ping_response:
        case connector_request_id_sm_cli_response:
        case connector_request_id_sm_cli_response_length:
        case connector_request_id_sm_cli_status:
        case connector_request_id_sm_more_data:
        case connector_request_id_sm_opaque_response:
        case connector_request_id_sm_config_request:
            ASSERT_MSG(0);
            break;

#if 0
        case connector_request_id_data_service_receive_data:
        {
            connector_data_service_receive_data_t * const data_ptr = data;

            connector_status = ccapi_process_device_request_data(data_ptr, ccapi_data);

            break;
        }
        case connector_request_id_data_service_receive_reply_data:
        {
            connector_data_service_receive_reply_data_t * const reply_ptr = data;

            connector_status = ccapi_process_device_request_response(reply_ptr);

            break;
        }
        case connector_request_id_data_service_receive_status:
        {
            connector_data_service_status_t const * const status_ptr = data;

            connector_status = ccapi_process_device_request_status(status_ptr, ccapi_data);

            break;
        }
        case connector_request_id_data_service_receive_reply_length:
        {
            connector_data_service_length_t * const length_ptr = data;
            
            connector_status = ccapi_process_device_request_length(length_ptr);

            break;
        }
#endif
    }

    ASSERT_MSG_GOTO(connector_status != connector_callback_unrecognized, done);

done:
    return connector_status;
}
#endif
