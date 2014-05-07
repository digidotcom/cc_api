#ifndef _CCAPI_SEND_H_
#define _CCAPI_SEND_H_

#ifdef CCIMP_DATA_SERVICE_ENABLED

#define SEND_WAIT_FOREVER ((unsigned long) 0)

typedef enum {
    CCAPI_SEND_ERROR_NONE,
    CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED,
    CCAPI_SEND_ERROR_FILESYSTEM_NOT_RUNNING,
    CCAPI_SEND_ERROR_INVALID_CLOUD_PATH,
    CCAPI_SEND_ERROR_INVALID_CONTENT_TYPE,
    CCAPI_SEND_ERROR_INVALID_DATA,
    CCAPI_SEND_ERROR_INVALID_LOCAL_PATH,
    CCAPI_SEND_ERROR_NOT_A_FILE,
    CCAPI_SEND_ERROR_ACCESSING_FILE,
    CCAPI_SEND_ERROR_INVALID_HINT_POINTER,
    CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_SEND_ERROR_SYNCR_FAILED,
    CCAPI_SEND_ERROR_INITIATE_ACTION_FAILED,
    CCAPI_SEND_ERROR_STATUS_CANCEL,
    CCAPI_SEND_ERROR_STATUS_TIMEOUT,
    CCAPI_SEND_ERROR_STATUS_SESSION_ERROR,
    CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST,
    CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE,
    CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR
} ccapi_send_error_t;

typedef enum {
    CCAPI_SEND_BEHAVIOR_APPEND,
    CCAPI_SEND_BEHAVIOR_OVERWRITE
} ccapi_send_behavior_t;

typedef struct {
    char * string;
    size_t length;
} ccapi_string_info_t;

ccapi_send_error_t ccapi_send_data(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior);
ccapi_send_error_t ccapi_send_data_with_reply(ccapi_transport_t const transport, char const * const cloud_path, char const * const content_type, void const * const data, size_t bytes, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint);
ccapi_send_error_t ccapi_send_file(ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior);
ccapi_send_error_t ccapi_send_file_with_reply(ccapi_transport_t const transport, char const * const local_path, char const * const cloud_path, char const * const content_type, ccapi_send_behavior_t behavior, unsigned long const timeout, ccapi_string_info_t * const hint);
#endif

#endif
