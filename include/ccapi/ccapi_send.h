#ifndef _CCAPI_SEND_H_
#define _CCAPI_SEND_H_

#ifdef CCIMP_DATA_SERVICE_ENABLED

#define SEND_WAIT_FOREVER UINT32_C(0x0)

typedef enum {
    CCAPI_SEND_ERROR_NONE,
    CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED,
    CCAPI_SEND_ERROR_INVALID_CLOUD_PATH,
    CCAPI_SEND_ERROR_INVALID_CONTENT_TYPE,
    CCAPI_SEND_ERROR_INVALID_DATA,
    CCAPI_SEND_ERROR_INVALID_HINT_POINTER,
    CCAPI_SEND_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_SEND_ERROR_SYNCR_FAILED,
    CCAPI_SEND_ERROR_INITIATE_ACTION_FAILED,    /* Error reported by ccfsm connector_initiate_action */ /* todo: change to _INITIATE_ACTION_FAILED */
    CCAPI_SEND_ERROR_STATUS_CANCEL,             /* session is cancelled by the user */
    CCAPI_SEND_ERROR_STATUS_TIMEOUT,            /* session timed out */
    CCAPI_SEND_ERROR_STATUS_SESSION_ERROR,      /* error from lower communication layer  */
    CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST,      /* at least some portion of the request is not valid */
    CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE,      /* service not available, may retry later */
    CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR       /* Device Cloud encountered error while handling the request */
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
#endif

#endif
