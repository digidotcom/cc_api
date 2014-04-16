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

ccapi_send_error_t ccxapi_send_data(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior)
{
    ccapi_send_error_t error = CCAPI_SEND_ERROR_NONE;

    UNUSED_ARGUMENT(transport);
    UNUSED_ARGUMENT(cloud_path);
    UNUSED_ARGUMENT(content_type);
    UNUSED_ARGUMENT(data);
    UNUSED_ARGUMENT(bytes);
    UNUSED_ARGUMENT(behavior);
    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status != CCAPI_THREAD_RUNNING)
    {
        ccapi_logging_line("ccxapi_send_data: CCAPI not started");

        error = CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING;
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
