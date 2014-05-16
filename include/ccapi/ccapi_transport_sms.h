#ifndef _CCAPI_TRANSPORT_SMS_H_
#define _CCAPI_TRANSPORT_SMS_H_

#define CCAPI_SM_SMS_MAX_SESSIONS_LIMIT 256
#define CCAPI_SM_SMS_MAX_SESSIONS_DEFAULT 20

typedef enum {
    CCAPI_SMS_START_ERROR_NONE,
    CCAPI_SMS_START_ERROR_CCAPI_STOPPED,
    CCAPI_SMS_START_ERROR_NULL_POINTER,
    CCAPI_SMS_START_ERROR_INIT,
    CCAPI_SMS_START_ERROR_MAX_SESSIONS,
    CCAPI_SMS_START_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_SMS_START_ERROR_TIMEOUT
} ccapi_sms_start_error_t;

typedef enum {
    CCAPI_SMS_CLOSE_DISCONNECTED,
    CCAPI_SMS_CLOSE_DATA_ERROR
} ccapi_sms_close_cause_t;

typedef ccapi_bool_t (* ccapi_sms_close_cb_t)(ccapi_sms_close_cause_t cause);

typedef struct {
    struct {
        ccapi_sms_close_cb_t close;
    } callback;
    uint8_t timeout;
    struct {
        size_t max_sessions;
        size_t rx_timeout;
    } limit;
} ccapi_sms_info_t;


typedef struct {
    uint8_t timeout;
} ccapi_sms_stop_t;


ccapi_sms_start_error_t ccapi_start_transport_sms(ccapi_sms_info_t const * const sms_start);

#endif
