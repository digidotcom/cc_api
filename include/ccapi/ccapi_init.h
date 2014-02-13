#ifndef _CCAPI_INIT_H_
#define _CCAPI_INIT_H_

#include "connector_api.h"

#include "internal/ccapi_definitions.h"

typedef void * ccapi_device_request_service_t; /* STUB */
typedef void * ccapi_firmware_service_t; /* STUB */
typedef void * ccapi_remote_config_service_t; /* STUB */
typedef void * ccapi_filesystem_service_t; /* STUB */
typedef void * ccapi_cli_service_t; /* STUB */
typedef void * ccapi_status_callback_t; /* STUB */

typedef enum {
    CCAPI_INIT_ERROR_NONE,
    CCAPI_INIT_ERROR_NULL_PARAMETER,
    CCAPI_INIT_ERROR_INVALID_VENDORID,
    CCAPI_INIT_ERROR_INVALID_DEVICEID,
    CCAPI_INIT_ERROR_INVALID_URL,
    CCAPI_INIT_ERROR_INVALID_DEVICETYPE,
    CCAPI_INIT_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_INIT_ERROR_COUNT
} ccapi_init_error_t;

typedef struct {
        uint32_t vendor_id;
        uint8_t device_id[DEVICE_ID_LENGTH];
        char const * device_type;
        char const * device_cloud_url;         /* Shared for TCP and UDP transports, meaningless for SMS */
        ccapi_status_callback_t status_callback;
        struct {
                ccapi_device_request_service_t * receive; /* See Device Request section, this structure will need more pointers */
                ccapi_firmware_service_t * firmware; /* EDPoTCP only */
                ccapi_remote_config_service_t * rci; /* TBD what this means */ /* EDPoTCP only */
                ccapi_filesystem_service_t * file_system;
                ccapi_cli_service_t * cli; /* SM only */
        } service;
} ccapi_start_t;


ccapi_init_error_t ccapi_start(ccapi_start_t const * const start);

#endif
