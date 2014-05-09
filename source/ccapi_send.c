#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#ifdef CCIMP_DATA_SERVICE_ENABLED

typedef struct
{
    connector_request_data_service_send_t header;
    ccapi_svc_send_data_t svc_send;
} ccapi_send_t;

static ccapi_bool_t valid_malloc(void * * ptr, size_t size, ccapi_send_error_t * const error)
{
    ccapi_bool_t success;
 
    *ptr = ccapi_malloc(size);

    success = (*ptr == NULL) ? CCAPI_FALSE : CCAPI_TRUE;
    
    if (!success)
    {
        *error = CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY;
    }

    return success;
}

static connector_transport_t ccapi_to_connector_transport(ccapi_transport_t const ccapi_transport)
{
    connector_transport_t connector_transport = connector_transport_all;

    switch(ccapi_transport)
    {
        case CCAPI_TRANSPORT_TCP:
            connector_transport = connector_transport_tcp;
            break;
        case CCAPI_TRANSPORT_UDP:
#ifdef CCIMP_UDP_TRANSPORT_ENABLED
            connector_transport = connector_transport_udp;
#endif
            break;
        case CCAPI_TRANSPORT_SMS:
#ifdef CCIMP_SMS_TRANSPORT_ENABLED
            connector_transport = connector_transport_sms;
#endif
            break;
    }

    return connector_transport;
}

static ccimp_status_t ccapi_send_lock_acquire(ccapi_send_t const * const send_info, unsigned long const timeout_ms)
{
    ccimp_os_syncr_acquire_t acquire_data;
    ccimp_status_t status = CCIMP_STATUS_ERROR;

    ASSERT_MSG_GOTO(send_info->svc_send.send_syncr != NULL, done);
    
    acquire_data.syncr_object = send_info->svc_send.send_syncr;
    acquire_data.timeout_ms= timeout_ms;

    status = ccimp_os_syncr_acquire(&acquire_data);

    if (status == CCIMP_STATUS_OK && acquire_data.acquired != CCAPI_TRUE)
        status = CCIMP_STATUS_ERROR;

done:
    return status;
}

static ccapi_send_error_t checkargs_send_common(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;
    ccapi_bool_t * p_transport_started = NULL;

    if (!CCAPI_RUNNING(ccapi_data))
    {
        ccapi_logging_line("checkargs_send_common: CCAPI not started");

        error = CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING;
        goto done;
    }

    switch (transport)
    {
        case CCAPI_TRANSPORT_TCP:
            p_transport_started = &ccapi_data->transport_tcp.connected;
            break;
        case CCAPI_TRANSPORT_UDP:
#ifdef CCIMP_UDP_TRANSPORT_ENABLED
            p_transport_started = &ccapi_data->transport_udp.started;
#endif
            break;
        case CCAPI_TRANSPORT_SMS:
#ifdef CCIMP_SMS_TRANSPORT_ENABLED
            p_transport_started = &ccapi_data->transport_udp.started;
#endif
            break;
    }

    if (p_transport_started == NULL || *p_transport_started == CCAPI_FALSE)
    {
        ccapi_logging_line("checkargs_send_common: Transport not started");

        error = CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED;
        goto done;
    }

    if (cloud_path == NULL || *cloud_path == '\0')
    {
        ccapi_logging_line("checkargs_send_common: Invalid cloud_path");

        error = CCAPI_SEND_ERROR_INVALID_CLOUD_PATH;
        goto done;
    }

    if (content_type != NULL && (*content_type == '\0' || strlen(content_type) > UCHAR_MAX))
    {
        ccapi_logging_line("checkargs_send_common: Invalid content_type");

        error = CCAPI_SEND_ERROR_INVALID_CONTENT_TYPE;
        goto done;
    }

done:
    return error;
}

static ccapi_send_error_t checkargs_send_data_common(void const * const data, size_t bytes)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    if (data == NULL || bytes == 0)
    {
        ccapi_logging_line("checkargs_send_data_common: Invalid data");

        error = CCAPI_SEND_ERROR_INVALID_DATA;
        goto done;
    }

done:
    return error;
}

static ccapi_send_error_t checkargs_send_file_common(char const * const local_path)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    if (local_path == NULL || *local_path == '\0')
    {
        ccapi_logging_line("checkargs_send_file_common: Invalid local_path");

        error = CCAPI_SEND_ERROR_INVALID_LOCAL_PATH;
        goto done;
    }

done:
    return error;
}

static ccapi_send_error_t checkargs_send_with_reply_common(unsigned long const timeout, ccapi_string_info_t * const hint)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    /* No check needed for timeout */
    UNUSED_ARGUMENT(timeout);

    if (hint != NULL && (hint->string == NULL || hint->length == 0))
    {
        ccapi_logging_line("checkargs_send_with_reply_common: Invalid hint pointer");

        error = CCAPI_SEND_ERROR_INVALID_HINT_POINTER;
        goto done;
    }

done:
    return error;
}

static ccapi_send_error_t setup_send_common(ccapi_data_t * const ccapi_data, ccapi_send_t * send_info, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    {
        ccimp_os_syncr_create_t create_data;
    
        if (ccimp_os_syncr_create(&create_data) != CCIMP_STATUS_OK)
        {
            error = CCAPI_SEND_ERROR_SYNCR_FAILED;
            goto done;
        }

        send_info->svc_send.send_syncr = create_data.syncr_object;
    }

    send_info->header.transport = ccapi_to_connector_transport(transport);

    switch (behavior)
    {
        case CCAPI_SEND_BEHAVIOR_APPEND:
            send_info->header.option = connector_data_service_send_option_append;
            break;
        case CCAPI_SEND_BEHAVIOR_OVERWRITE:
            send_info->header.option = connector_data_service_send_option_overwrite;
            break;
    }

    send_info->svc_send.ccapi_data = ccapi_data;
    send_info->svc_send.request_error = CCAPI_SEND_ERROR_NONE;
    send_info->svc_send.response_error = CCAPI_SEND_ERROR_NONE;
    send_info->svc_send.status_error = CCAPI_SEND_ERROR_NONE;
    send_info->header.path = cloud_path;
    send_info->header.content_type = content_type;
    send_info->header.user_context = &send_info->svc_send;

done:
    return error;
}

#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
static ccapi_send_error_t setup_send_file_common(ccapi_data_t * const ccapi_data, ccapi_send_t * send_info, char const * const local_path)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    {
        ccimp_fs_stat_t fs_status;

        switch (ccapi_get_dir_entry_status(ccapi_data, local_path, &fs_status)) 
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                error = CCAPI_SEND_ERROR_NOT_A_FILE;
                goto done;
        }

        switch (fs_status.type)
        {
            case CCIMP_FS_DIR_ENTRY_FILE:
                send_info->svc_send.bytes_remaining = fs_status.file_size;
                break;
            case CCIMP_FS_DIR_ENTRY_DIR:
            case CCIMP_FS_DIR_ENTRY_UNKNOWN:
                error = CCAPI_SEND_ERROR_NOT_A_FILE;
                goto done;
                break;
        }
    }

    {
        ccimp_fs_handle_t file_handler;

        ccimp_status_t ccimp_status = ccapi_open_file(ccapi_data, local_path, CCIMP_FILE_O_RDONLY, &file_handler);
        if (ccimp_status != CCIMP_STATUS_OK)
        {
            error = CCAPI_SEND_ERROR_ACCESSING_FILE;
            goto done;
        }

        send_info->svc_send.file_handler = file_handler;
    }

    ccapi_logging_line("file_size=%d, file_handler=%p\n", send_info->svc_send.bytes_remaining, send_info->svc_send.file_handler.pointer);


    send_info->svc_send.sending_file = CCAPI_TRUE;
    send_info->svc_send.next_data = NULL;

done:
    return error;
}
#endif

static void setup_send_data_common(ccapi_send_t * send_info, void const * const data, size_t bytes)
{
    send_info->svc_send.next_data = (void *)data;
    send_info->svc_send.bytes_remaining = bytes;
    send_info->svc_send.sending_file = CCAPI_FALSE;
}

static void setup_send_no_reply_common(ccapi_send_t * send_info)
{
    send_info->svc_send.hint = NULL;
    send_info->header.response_required = connector_false;
    send_info->header.timeout_in_seconds = SEND_WAIT_FOREVER;
}

static void setup_send_with_reply_common(ccapi_send_t * send_info, ccapi_string_info_t * const hint, unsigned long const timeout)
{
    send_info->svc_send.hint = hint;
    send_info->header.response_required = connector_true;
    send_info->header.timeout_in_seconds = timeout;
}

static ccapi_send_error_t call_send_common(ccapi_data_t * const ccapi_data, ccapi_send_t * send_info)
{
    connector_status_t status;
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;
    do
    {
        status = connector_initiate_action_secure(ccapi_data, connector_initiate_send_data, &send_info->header);

        if (status == connector_service_busy)
        {
            ccimp_os_yield();
        }
    } while (status == connector_service_busy);

    if (status == connector_success)
    {
        ccimp_status_t result = ccapi_send_lock_acquire(send_info, OS_SYNCR_ACQUIRE_INFINITE);
        if (result != CCIMP_STATUS_OK)
        {
            ccapi_logging_line("call_send_common: lock_acquire failed");
            error = CCAPI_SEND_ERROR_SYNCR_FAILED;
        }
    }
    else
    {
        ccapi_logging_line("call_send_common: ccfsm error %d", status);
        error = CCAPI_SEND_ERROR_INITIATE_ACTION_FAILED;
    }

    return error;
}

static void finish_send_common(ccapi_data_t * const ccapi_data, ccapi_send_t * send_info)
{
    /* Free resources */
    if (send_info != NULL)
    {
        if (send_info->svc_send.send_syncr != NULL)
        {
            ASSERT_MSG(ccapi_syncr_destroy(send_info->svc_send.send_syncr) == CCIMP_STATUS_OK);
        }

#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
        if (send_info->svc_send.sending_file == CCAPI_TRUE)
        {
            ASSERT_MSG(ccapi_close_file(ccapi_data, send_info->svc_send.file_handler) == CCIMP_STATUS_OK);
        }
#else
        UNUSED_ARGUMENT(ccapi_data);
#endif

        ccapi_free(send_info);
    }
}

ccapi_send_error_t ccxapi_send_data(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error;
    ccapi_send_t * send_info = NULL;

    error = checkargs_send_common(ccapi_data, transport, cloud_path, content_type);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_data_common(data, bytes);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (!valid_malloc((void**)&send_info, sizeof(ccapi_send_t), &error))
    {
        goto done;
    }

    error = setup_send_common(ccapi_data, send_info, transport, cloud_path, content_type, behavior);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    setup_send_data_common(send_info, data, bytes);

    setup_send_no_reply_common(send_info);

    error = call_send_common(ccapi_data, send_info);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    /* tcp transport will update response_error even if not requested but we ignore it here */
    if (send_info->svc_send.request_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.request_error;
    }
    else
    {
        error = send_info->svc_send.status_error;
    }

done:
    finish_send_common(ccapi_data, send_info);

    return error;
}

ccapi_send_error_t ccxapi_send_data_with_reply(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    ccapi_send_error_t error;
    ccapi_send_t * send_info = NULL;

    error = checkargs_send_common(ccapi_data, transport, cloud_path, content_type);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_data_common(data, bytes);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_with_reply_common(timeout, hint);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (!valid_malloc((void**)&send_info, sizeof(ccapi_send_t), &error))
    {
        goto done;
    }

    error = setup_send_common(ccapi_data, send_info, transport, cloud_path, content_type, behavior);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    setup_send_data_common(send_info, data, bytes);

    setup_send_with_reply_common(send_info, hint, timeout);

    error = call_send_common(ccapi_data, send_info);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (send_info->svc_send.request_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.request_error;
    }
    else if (send_info->svc_send.response_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.response_error;
    }
    else
    {
        error = send_info->svc_send.status_error;
    }

done:
    finish_send_common(ccapi_data, send_info);

    return error;
}

ccapi_send_error_t ccxapi_send_file(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error;
    ccapi_send_t * send_info = NULL;

#ifndef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
    UNUSED_ARGUMENT(transport);
    UNUSED_ARGUMENT(local_path);
    UNUSED_ARGUMENT(cloud_path);
    UNUSED_ARGUMENT(content_type);
    UNUSED_ARGUMENT(behavior);

    error = CCAPI_SEND_ERROR_FILESYSTEM_NOT_SUPPORTED;
    goto done;
#else
    error = checkargs_send_common(ccapi_data, transport, cloud_path, content_type);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_file_common(local_path);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (!valid_malloc((void**)&send_info, sizeof(ccapi_send_t), &error))
    {
        goto done;
    }

    error = setup_send_common(ccapi_data, send_info, transport, cloud_path, content_type, behavior);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = setup_send_file_common(ccapi_data, send_info, local_path);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    setup_send_no_reply_common(send_info);

    error = call_send_common(ccapi_data, send_info);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    /* tcp transport will update response_error even if not requested but we ignore it here */
    if (send_info->svc_send.request_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.request_error;
    }
    else
    {
        error = send_info->svc_send.status_error;
    }
#endif

done:
    finish_send_common(ccapi_data, send_info);

    return error;
}

ccapi_send_error_t ccxapi_send_file_with_reply(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    ccapi_send_error_t error;
    ccapi_send_t * send_info = NULL;

#ifndef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
    UNUSED_ARGUMENT(transport);
    UNUSED_ARGUMENT(local_path);
    UNUSED_ARGUMENT(cloud_path);
    UNUSED_ARGUMENT(content_type);
    UNUSED_ARGUMENT(behavior);
    UNUSED_ARGUMENT(timeout);
    UNUSED_ARGUMENT(hint);

    error = CCAPI_SEND_ERROR_FILESYSTEM_NOT_SUPPORTED;
    goto done;
#else
    error = checkargs_send_common(ccapi_data, transport, cloud_path, content_type);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_file_common(local_path);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = checkargs_send_with_reply_common(timeout, hint);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (!valid_malloc((void**)&send_info, sizeof(ccapi_send_t), &error))
    {
        goto done;
    }

    error = setup_send_common(ccapi_data, send_info, transport, cloud_path, content_type, behavior);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    error = setup_send_file_common(ccapi_data, send_info, local_path);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    setup_send_with_reply_common(send_info, hint, timeout);

    error = call_send_common(ccapi_data, send_info);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (send_info->svc_send.request_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.request_error;
    }
    else if (send_info->svc_send.response_error != CCAPI_SEND_ERROR_NONE)
    {
        error = send_info->svc_send.response_error;
    }
    else
    {
        error = send_info->svc_send.status_error;
    }
#endif

done:
    finish_send_common(ccapi_data, send_info);

    return error;
}

ccapi_send_error_t ccapi_send_data(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    return ccxapi_send_data(ccapi_data_single_instance, transport, cloud_path, content_type, data, bytes, behavior);
}

ccapi_send_error_t ccapi_send_data_with_reply(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    return ccxapi_send_data_with_reply(ccapi_data_single_instance, transport, cloud_path, content_type, data, bytes, behavior, timeout, hint);
}

ccapi_send_error_t ccapi_send_file(ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior)
{
    return ccxapi_send_file(ccapi_data_single_instance, transport, local_path, cloud_path, content_type, behavior);
}

ccapi_send_error_t ccapi_send_file_with_reply(ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    return ccxapi_send_file_with_reply(ccapi_data_single_instance, transport, local_path, cloud_path, content_type, behavior, timeout, hint);
}
#endif
