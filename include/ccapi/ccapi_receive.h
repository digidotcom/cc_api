#ifndef _CCAPI_RECEIVE_H_
#define _CCAPI_RECEIVE_H_

typedef enum {
    CCAPI_RECEIVE_ERROR_NONE,
    CCAPI_RECEIVE_ERROR_CCAPI_STOPPED,
    CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT,
    CCAPI_RECEIVE_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_RECEIVE_ERROR_USER_REFUSED_TARGET
} ccapi_receive_error_t;

typedef ccapi_bool_t (*ccapi_receive_accept_cb_t)(char const * const target, ccapi_transport_t const transport);
typedef void (*ccapi_receive_data_cb_t)(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request, ccapi_buffer_info_t * const response, ccapi_receive_error_t receive_error);
typedef void (*ccapi_receive_status_cb_t)(char const * const target, ccapi_transport_t const transport, /* TODO: error */ ccapi_buffer_info_t * const response);

typedef struct {
    ccapi_receive_accept_cb_t accept_cb;
    ccapi_receive_data_cb_t data_cb;
    ccapi_receive_status_cb_t status_cb;
} ccapi_receive_service_t;

#endif
