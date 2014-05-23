#ifndef _CCAPI_RECEIVE_H_
#define _CCAPI_RECEIVE_H_

#define CCAPI_RECEIVE_NO_LIMIT ((size_t) -1)

typedef enum {
    CCAPI_RECEIVE_ERROR_NONE,
    CCAPI_RECEIVE_ERROR_CCAPI_NOT_RUNNING,
    CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT,
    CCAPI_RECEIVE_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_RECEIVE_ERROR_INVALID_TARGET,
    CCAPI_RECEIVE_ERROR_TARGET_NOT_ADDED,
    CCAPI_RECEIVE_ERROR_TARGET_ALREADY_ADDED,
    CCAPI_RECEIVE_ERROR_INVALID_DATA_CB,
    CCAPI_RECEIVE_ERROR_SYNCR_FAILED,
    CCAPI_RECEIVE_ERROR_USER_REFUSED_TARGET,
    CCAPI_RECEIVE_ERROR_REQUEST_TOO_BIG,
    CCAPI_RECEIVE_ERROR_STATUS_CANCEL,
    CCAPI_RECEIVE_ERROR_STATUS_TIMEOUT,
    CCAPI_RECEIVE_ERROR_STATUS_SESSION_ERROR
} ccapi_receive_error_t;

typedef ccapi_bool_t (*ccapi_receive_accept_cb_t)(char const * const target, ccapi_transport_t const transport);
typedef void (*ccapi_receive_data_cb_t)(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info);
typedef void (*ccapi_receive_status_cb_t)(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error);

typedef struct {
    ccapi_receive_accept_cb_t accept_cb;
    ccapi_receive_data_cb_t data_cb;
    ccapi_receive_status_cb_t status_cb;
} ccapi_receive_service_t;

ccapi_receive_error_t ccapi_receive_add_target(char const * const target, ccapi_receive_data_cb_t data_cb, ccapi_receive_status_cb_t status_cb, size_t maximum_request_size);
ccapi_receive_error_t ccapi_receive_remove_target(char const * const target);

#endif
