#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#ifdef CCIMP_DATA_SERVICE_ENABLED

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

    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status != CCAPI_THREAD_RUNNING)
    {
        ccapi_logging_line("ccxapi_send_data: CCAPI not started");

        error = CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING;
        goto done;
    }

    switch (transport)
    {
        case CCAPI_TRANSPORT_TCP:
            if (ccapi_data->transport_tcp.connected)
                goto transport_started;
#ifdef CCIMP_UDP_TRANSPORT_ENABLED
        case CCAPI_TRANSPORT_UDP:
            if (ccapi_data->transport_udp.started)
                goto transport_started;
#endif
#ifdef CCIMP_SMS_TRANSPORT_ENABLED
        case CCAPI_TRANSPORT_SMS:
            if (ccapi_data->transport_sms.started)
                goto transport_started;
#endif
    }

    ccapi_logging_line("ccxapi_send: Transport not started");
    error = CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED;
    goto done;
transport_started:


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

ccapi_send_error_t ccxapi_send_data(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    UNUSED_ARGUMENT(behavior);

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

done:
    return error;
}

ccapi_send_error_t ccapi_send_data(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    return ccxapi_send_data(ccapi_data_single_instance, transport, cloud_path, content_type, data, bytes, behavior);

}
#endif
