/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccapi_connector_config.h"
#include "ccimp/ccimp_debug.h"
#include "ccimp/ccimp_hal.h"
#include "ccimp/ccimp_types.h"
#include "ccimp/ccimp_os.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)

#define ON_FALSE_DO_(cond, code)        do { if (!(cond)) {code;} } while (0)

#if (defined CCIMP_DEBUG_ENABLED)
#define ASSERT_GOTO(cond, message, label)   ON_FALSE_DO_((cond), {ccimp_hal_assertion_hit(message); goto label;})
#else
#define ASSERT_GOTO(cond, message, label)   ON_FALSE_DO_((cond), {goto label;})
#endif

#define reset_heap_ptr(pp) do { if (*(pp) != NULL) { ccapi_free(*(pp)); *(pp) = NULL; } } while (0)

typedef struct {
    uint32_t vendor_id;
    uint8_t device_id[DEVICE_ID_LENGTH];
    char * device_type;
    char * device_cloud_url;
    ccapi_bool_t cli_supported;
    ccapi_bool_t receive_supported;
    ccapi_bool_t firmware_supported;
    ccapi_bool_t rci_supported;
    ccapi_bool_t filesystem_supported;
} ccapi_config_t;

typedef enum {
    CCAPI_THREAD_NOT_STARTED,
    CCAPI_THREAD_REQUEST_START,
    CCAPI_THREAD_RUNNING,
    CCAPI_THREAD_REQUEST_STOP
} ccapi_thread_status_t;

typedef struct {
    ccimp_create_thread_info_t ccimp_info;
    ccapi_thread_status_t status;
} ccapi_thread_info_t;

typedef struct {
    char const * signature;
    void * connector_handle;
    ccapi_config_t config;
    struct {
        ccapi_thread_info_t * connector_run;
    } thread;
} ccapi_data_t;

typedef struct ccapi_handle * ccapi_handle_t;

extern ccapi_handle_t ccapi_data_single_instance;
extern char const ccapi_signature[];

void ccapi_connector_run_thread(void * const argument);
void * ccapi_malloc(size_t size);
ccimp_status_t ccapi_free(void * ptr);
connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context);

void ccapi_debug_line(char const * const format, ...);
void ccapi_debug_print_buffer(char const * const label, void const * const buffer, size_t const length);
#endif
