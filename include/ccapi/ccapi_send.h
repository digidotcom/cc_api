#ifndef _CCAPI_SEND_H_
#define _CCAPI_SEND_H_

#ifdef CCIMP_DATA_SERVICE_ENABLED
typedef enum {
    CCAPI_SEND_ERROR_NONE,
    CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_SEND_ERROR_INVALID_XX
} ccapi_send_error_t;

typedef enum {
    CCAPI_SEND_APPEND,
    CCAPI_SEND_OVERWRITE
} ccapi_send_behavior_t;

ccapi_send_error_t ccapi_send_data(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior);
#endif

#endif
