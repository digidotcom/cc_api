#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#ifdef CCIMP_DATA_SERVICE_ENABLED

typedef struct
{
    connector_request_data_service_send_t header;
    ccapi_svc_send_data_t svc_send;
} ccapi_send_t;

static ccapi_bool_t valid_malloc(void * ptr, ccapi_send_error_t * const error)
{
    if (ptr == NULL)
    {
        *error = CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY;
        return CCAPI_FALSE;
    }
    else
    {
        return CCAPI_TRUE;
    }
}

static ccapi_send_error_t check_send_common_args(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;
    ccapi_bool_t * p_transport_started = NULL;

    if (!CCAPI_RUNNING(ccapi_data))
    {
        ccapi_logging_line("ccxapi_send_data: CCAPI not started");

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
        ccapi_logging_line("ccxapi_send: Transport not started");

        error = CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED;
        goto done;
    }

    if (cloud_path == NULL)
    {
        ccapi_logging_line("ccxapi_send: Invalid cloud_path");

        error = CCAPI_SEND_ERROR_INVALID_CLOUD_PATH;
        goto done;
    }

    if (content_type != NULL)
        ASSERT_MSG_GOTO(strlen(content_type) <= UCHAR_MAX, done);

done:
    return error;
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

ccapi_send_error_t ccxapi_send_data(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;
    ccapi_send_t * send_info;

    error = check_send_common_args(ccapi_data, transport, cloud_path, content_type);
    if (error != CCAPI_SEND_ERROR_NONE)
    {
        goto done;
    }

    if (data == NULL || bytes == 0)
    {
        ccapi_logging_line("ccxapi_send_data: Invalid data");

        error = CCAPI_SEND_ERROR_INVALID_DATA;
        goto done;
    }

    send_info = ccapi_malloc(sizeof(ccapi_send_t));

    if (!valid_malloc(send_info, &error))
    {
        goto done;
    }

    {
        ccimp_os_syncr_create_t create_data;
    
        if (ccimp_os_syncr_create(&create_data) == CCIMP_STATUS_OK)
            send_info->svc_send.send_syncr = create_data.syncr_object;
        else goto done;
    }

    /* we are storing some stack variables here, need to block until we get a response */
    send_info->svc_send.error = CCAPI_SEND_ERROR_NONE;
    send_info->svc_send.next_data = (void *)data;
    send_info->svc_send.bytes_remaining = bytes;
    send_info->header.path = cloud_path;
    send_info->header.content_type = content_type;
    send_info->header.user_context = &send_info->svc_send;
    send_info->header.response_required = connector_false;
    send_info->header.timeout_in_seconds = SEND_WAIT_FOREVER;

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

    {
        connector_status_t const status = connector_initiate_action(ccapi_data->connector_handle, connector_initiate_send_data, &send_info->header);

        if (status == connector_success)
        {
            ccimp_status_t result = ccapi_send_lock_acquire(send_info, OS_SYNCR_ACQUIRE_INFINITE);
            if (result != CCIMP_STATUS_OK)
            {
                ccapi_logging_line("ccxapi_send_data: lock_acquire failed");
                error = CCAPI_SEND_ERROR_SYNCR_ERROR;
            }
            else
            {
                error = send_info->svc_send.error;
            }
        }
        else
        {
            ccapi_logging_line("ccxapi_send_data: ccfsm error %d", status);
            error = CCAPI_SEND_ERROR_CCFSM_ERROR;
        }
    }

    /* Free resources */
    {
        ccimp_os_syncr_destroy_t destroy_data;
        destroy_data.syncr_object = send_info->svc_send.send_syncr;
    
        ccimp_os_syncr_destroy(&destroy_data);
    }
    ccapi_free(send_info);

done:
    return error;
}

ccapi_send_error_t ccapi_send_data(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    return ccxapi_send_data(ccapi_data_single_instance, transport, cloud_path, content_type, data, bytes, behavior);

}
#endif
