#ifndef _CCAPI_INIT_H_
#define _CCAPI_INIT_H_

typedef void * ccapi_firmware_service_t; /* STUB */
typedef void * ccapi_remote_config_service_t; /* STUB */
typedef void * ccapi_cli_service_t; /* STUB */
typedef void * ccapi_status_callback_t; /* STUB */

typedef enum {
    CCAPI_START_ERROR_NONE,
    CCAPI_START_ERROR_NULL_PARAMETER,
    CCAPI_START_ERROR_INVALID_VENDORID,
    CCAPI_START_ERROR_INVALID_DEVICEID,
    CCAPI_START_ERROR_INVALID_URL,
    CCAPI_START_ERROR_INVALID_DEVICETYPE,
    CCAPI_START_ERROR_INVALID_RECEIVE_DATA_CB,
    CCAPI_START_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_START_ERROR_THREAD_FAILED,
    CCAPI_START_ERROR_SYNCR_FAILED,
    CCAPI_START_ERROR_ALREADY_STARTED
} ccapi_start_error_t;

typedef enum {
    CCAPI_STOP_ERROR_NONE,
    CCAPI_STOP_ERROR_NOT_STARTED
} ccapi_stop_error_t;

typedef struct {
        uint32_t vendor_id;
        uint8_t device_id[16];
        char const * device_type;
        char const * device_cloud_url;         /* Shared for TCP and UDP transports, meaningless for SMS */
        ccapi_status_callback_t status_callback;
        struct {
                ccapi_receive_service_t * receive;
                ccapi_firmware_service_t * firmware; /* EDPoTCP only */
                ccapi_remote_config_service_t * rci; /* TBD what this means */ /* EDPoTCP only */
                ccapi_filesystem_service_t * file_system;
                ccapi_cli_service_t * cli; /* SM only */
        } service;
} ccapi_start_t;

ccapi_start_error_t ccapi_start(ccapi_start_t const * const start);
ccapi_stop_error_t ccapi_stop(ccapi_stop_t const behavior);

#endif
