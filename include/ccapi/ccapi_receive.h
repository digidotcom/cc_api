#ifndef _CCAPI_RECEIVE_H_
#define _CCAPI_RECEIVE_H_

typedef ccapi_bool_t (*ccapi_receive_accept_cb_t)(char const * const target, ccapi_transport_t const transport);
typedef void (*ccapi_receive_request_cb_t)(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request, ccapi_buffer_info_t * const response /* TODO: error */);
typedef void (*ccapi_receive_response_cb_t)(char const * const target, ccapi_transport_t const transport, /* TODO: error */ ccapi_buffer_info_t * const response);

typedef struct {
    ccapi_receive_accept_cb_t accept_cb;
    ccapi_receive_request_cb_t request_cb;
    ccapi_receive_response_cb_t response_cb;
} ccapi_receive_service_t;

typedef enum {
    CCAPI_RECEIVE_ERROR_NONE,
    CCAPI_RECEIVE_ERROR_CCAPI_STOPPED,
    CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT,
    CCAPI_RECEIVE_ERROR_INSUFFICIENT_MEMORY
} ccapi_receive_error_t;

#endif
