#ifndef _CCAPI_DATAPOINTS_BINARY_H_
#define _CCAPI_DATAPOINTS_BINARY_H_

#if (defined CCIMP_DATA_SERVICE_ENABLED) && (defined CCIMP_DATA_POINTS_ENABLED)

typedef enum {
    CCAPI_DP_B_ERROR_NONE,
    CCAPI_DP_B_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_DP_B_ERROR_TRANSPORT_NOT_STARTED,
    CCAPI_DP_B_ERROR_INVALID_CLOUD_PATH,
    CCAPI_DP_B_ERROR_INVALID_DATA,
    CCAPI_DP_B_ERROR_INVALID_HINT_POINTER,
    CCAPI_DP_B_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_DP_B_ERROR_SYNCR_FAILED,
    CCAPI_DP_B_ERROR_INITIATE_ACTION_FAILED,
    CCAPI_DP_B_ERROR_STATUS_CANCEL,
    CCAPI_DP_B_ERROR_STATUS_TIMEOUT,
    CCAPI_DP_B_ERROR_STATUS_SESSION_ERROR,
    CCAPI_DP_B_ERROR_RESPONSE_BAD_REQUEST,
    CCAPI_DP_B_ERROR_RESPONSE_UNAVAILABLE,
    CCAPI_DP_B_ERROR_RESPONSE_CLOUD_ERROR
} ccapi_dp_b_error_t;


ccapi_dp_b_error_t ccapi_dp_send_binary(ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes);
ccapi_dp_b_error_t ccapi_dp_send_binary_with_reply(ccapi_transport_t const transport, char const * const stream_id, void const * const data, size_t const bytes, unsigned long const timeout, ccapi_string_info_t * const hint);

#endif

#endif
