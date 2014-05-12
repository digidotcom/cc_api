#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_DATA_SERVICE_ENABLED) && (defined CCIMP_DATA_POINTS_ENABLED)

#include "ccapi/ccxapi.h"

static ccapi_dp_b_error_t add_dp_prefix_sufix(char const * const stream_id, char * * dp_path)
{
    ccapi_dp_b_error_t dp_b_error = CCAPI_DP_B_ERROR_NONE;

    char const * const dp_prefix = "DataPoint/";
    char const * const dp_sufix = ".bin";
    char * tmp_dp_path;

    if (stream_id == NULL || *stream_id == '\0')
    {
        ccapi_logging_line("add_dp_prefix_sufix: Invalid stream_id");

        dp_b_error = CCAPI_DP_B_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    tmp_dp_path = ccapi_malloc(strlen(dp_prefix) + strlen(stream_id) + strlen(dp_sufix) + 1);
    if (tmp_dp_path == NULL)
    {
        dp_b_error = CCAPI_DP_B_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    strcpy(tmp_dp_path, dp_prefix);
    strcat(tmp_dp_path, stream_id);
    strcat(tmp_dp_path, dp_sufix);

    *dp_path = tmp_dp_path;
done:
    return dp_b_error;
}

static ccapi_dp_b_error_t send_data_error_to_dp_binary_error(ccapi_send_error_t send_data_error)
{
    ccapi_dp_b_error_t dp_b_error;

    switch(send_data_error)
    {
        case CCAPI_SEND_ERROR_NONE:
            dp_b_error = CCAPI_DP_B_ERROR_NONE;
            break;
        case CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING:
            dp_b_error = CCAPI_DP_B_ERROR_CCAPI_NOT_RUNNING;
            break;
        case CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED:
            dp_b_error = CCAPI_DP_B_ERROR_TRANSPORT_NOT_STARTED;
            break;
        case CCAPI_SEND_ERROR_INVALID_DATA:
            dp_b_error = CCAPI_DP_B_ERROR_INVALID_DATA;
            break;
        case CCAPI_SEND_ERROR_INVALID_HINT_POINTER:
            dp_b_error = CCAPI_DP_B_ERROR_INVALID_HINT_POINTER;
            break;
        case CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY:
            dp_b_error = CCAPI_DP_B_ERROR_INSUFFICIENT_MEMORY;
            break;
        case CCAPI_SEND_ERROR_SYNCR_FAILED:
            dp_b_error = CCAPI_DP_B_ERROR_SYNCR_FAILED;
            break;
        case CCAPI_SEND_ERROR_INITIATE_ACTION_FAILED:
            dp_b_error = CCAPI_DP_B_ERROR_INITIATE_ACTION_FAILED;
            break;
        case CCAPI_SEND_ERROR_STATUS_CANCEL:
            dp_b_error = CCAPI_DP_B_ERROR_STATUS_CANCEL;
            break;
        case CCAPI_SEND_ERROR_STATUS_TIMEOUT:
            dp_b_error = CCAPI_DP_B_ERROR_STATUS_TIMEOUT;
            break;
        case CCAPI_SEND_ERROR_STATUS_SESSION_ERROR:
            dp_b_error = CCAPI_DP_B_ERROR_STATUS_SESSION_ERROR;
            break;
        case CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST:
            dp_b_error = CCAPI_DP_B_ERROR_RESPONSE_BAD_REQUEST;
            break;
        case CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE:
            dp_b_error = CCAPI_DP_B_ERROR_RESPONSE_UNAVAILABLE;
            break;
        case CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR:
            dp_b_error = CCAPI_DP_B_ERROR_RESPONSE_CLOUD_ERROR;
            break;
        /* These errors should not happen */
        case CCAPI_SEND_ERROR_FILESYSTEM_NOT_RUNNING:
        case CCAPI_SEND_ERROR_INVALID_CLOUD_PATH:
        case CCAPI_SEND_ERROR_INVALID_CONTENT_TYPE:
        case CCAPI_SEND_ERROR_INVALID_LOCAL_PATH:
        case CCAPI_SEND_ERROR_NOT_A_FILE:
            dp_b_error = CCAPI_DP_B_ERROR_RESPONSE_CLOUD_ERROR;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return dp_b_error;
}

ccapi_dp_b_error_t ccxapi_dp_send_binary(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes)
{
    ccapi_send_error_t send_data_error;
    ccapi_dp_b_error_t dp_b_error;
    char * dp_path;

    dp_b_error = add_dp_prefix_sufix(stream_id, &dp_path);
    if (dp_b_error != CCAPI_DP_B_ERROR_NONE)
    {
        goto done;
    }

    send_data_error = ccxapi_send_data(ccapi_handle, transport, dp_path, NULL, data, bytes, CCAPI_SEND_BEHAVIOR_OVERWRITE);

    dp_b_error = send_data_error_to_dp_binary_error(send_data_error);

    ccapi_free(dp_path);

done:
    return dp_b_error;
}

ccapi_dp_b_error_t ccxapi_dp_send_binary_with_reply(ccapi_handle_t const ccapi_handle, ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    ccapi_send_error_t send_data_error;
    ccapi_dp_b_error_t dp_b_error;
    char * dp_path;

    dp_b_error = add_dp_prefix_sufix(stream_id, &dp_path);
    if (dp_b_error != CCAPI_DP_B_ERROR_NONE)
    {
        goto done;
    }

    send_data_error = ccxapi_send_data_with_reply(ccapi_handle, transport, dp_path, NULL, data, bytes, CCAPI_SEND_BEHAVIOR_OVERWRITE, timeout, hint);

    dp_b_error = send_data_error_to_dp_binary_error(send_data_error);

    ccapi_free(dp_path);

done:
    return dp_b_error;
}

ccapi_dp_b_error_t ccapi_dp_send_binary(ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes)
{
    return ccxapi_dp_send_binary((ccapi_handle_t)ccapi_data_single_instance, transport, stream_id, data, bytes);
}

ccapi_dp_b_error_t ccapi_dp_send_binary_with_reply(ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    return ccxapi_dp_send_binary_with_reply((ccapi_handle_t)ccapi_data_single_instance, transport, stream_id, data, bytes, timeout, hint);
}

#endif