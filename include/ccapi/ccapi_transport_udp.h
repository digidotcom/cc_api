#ifndef _CCAPI_TRANSPORT_UDP_H_
#define _CCAPI_TRANSPORT_UDP_H_

#define CCAPI_SM_UDP_MAX_SESSIONS_LIMIT 256
#define CCAPI_SM_UDP_MAX_SESSIONS_DEFAULT 20



typedef enum {
    CCAPI_UDP_START_ERROR_NONE,
    CCAPI_UDP_START_ERROR_CCAPI_STOPPED,
    CCAPI_UDP_START_ERROR_NULL_POINTER,
    CCAPI_UDP_START_ERROR_INIT,
    CCAPI_UDP_START_ERROR_MAX_SESSIONS,
    CCAPI_UDP_START_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_UDP_START_ERROR_TIMEOUT
} ccapi_udp_start_error_t;

typedef enum {
    CCAPI_UDP_STOP_ERROR_NONE,
    CCAPI_UDP_STOP_ERROR_NOT_STARTED,
    CCAPI_UDP_STOP_ERROR_CCFSM
} ccapi_udp_stop_error_t;

typedef enum {
    CCAPI_UDP_CLOSE_DISCONNECTED,
    CCAPI_UDP_CLOSE_DATA_ERROR
} ccapi_udp_close_cause_t;

typedef ccapi_bool_t (* ccapi_udp_close_cb_t)(ccapi_udp_close_cause_t cause);

typedef struct {
    struct {
        ccapi_udp_close_cb_t close;
    } callback;
    uint8_t timeout;
    struct {
        size_t max_sessions;
        size_t rx_timeout;
    } limit;
} ccapi_udp_info_t;


typedef struct {
    ccapi_stop_t behavior;
} ccapi_udp_stop_t;


ccapi_udp_start_error_t ccapi_start_transport_udp(ccapi_udp_info_t const * const udp_start);
ccapi_udp_stop_error_t ccapi_stop_transport_udp(ccapi_udp_stop_t const * const udp_stop);

#endif
