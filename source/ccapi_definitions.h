/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccapi_connector_config.h"
#include "custom/custom_debug.h"
#include "ccimp/ccimp_types.h"
#include "ccimp/ccimp_os.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)

#define ON_FALSE_DO_(cond, code)        do { if (!(cond)) {code;} } while (0)

#if (defined CCAPI_DEBUG)
#define ON_ASSERT_DO_(cond, code, output)   ON_FALSE_DO_((cond), {ASSERT(cond); code;})
#else
#define ON_ASSERT_DO_(cond, code, output)   ON_FALSE_DO_((cond), {code})
#endif

#if (defined UNIT_TEST)
extern char * assert_buffer;
#define ASSERT_GOTO(cond, message, label)   ON_FALSE_DO_((cond), {assert_buffer = message; goto label;})
#else
#define ASSERT_GOTO(cond, message, label)   ON_ASSERT_DO_((cond), {goto label;}, {})
#endif

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

typedef enum {
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR
} ccapi_debug_level_t;

typedef enum {
    ZONE_LAYER1     = 1,
    ZONE_START_STOP = 2,
    ZONE_TRANSPORT  = 4,
    ZONE_SEND_DATA  = 8
} ccapi_debug_zones_t;

typedef struct {
    char const * signature;
    void * connector_handle;
    ccapi_config_t config;
    struct {
        ccapi_thread_info_t * connector_run;
    } thread;
    ccapi_debug_zones_t dbg_zones;
    ccapi_debug_level_t dbg_level;
} ccapi_data_t;

extern ccapi_data_t * ccapi_data;
extern char const ccapi_signature[];
extern void ccapi_connector_run_thread(void * const argument);
extern void * ccapi_malloc(size_t size);
extern connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);

extern void ccapi_debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, char const * const format, ...);

#endif
