/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccimp/ccimp_types.h"
#include "ccimp/ccimp_os.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)


/* TODO: Get from ccimp_debug */
#define ASSERT(cond)        assert(cond)

#define ON_FALSE_DO_(cond, code)        do { if (!(cond)) {code;} } while (0)

#if (defined UNIT_TESTING)

extern char * assert_buffer;

#define ASSERT_CHECK(cond, label)           ON_FALSE_DO_((cond), {/* printf("Asserted: %s\n", label); */ assert_buffer = label; while(1);})
#define ASSERT_WAIT(timeout_ms)             { unsigned int loops= (timeout_ms * 2); while (assert_buffer == NULL && loops--) usleep(500); }
#define ASSERT_IF_NOT_HIT_DO(label, code)   ON_FALSE_DO_(assert_buffer != NULL && (!strcmp(assert_buffer, label)), {printf("Didn't hit assert: %s\n", label); code;})
#define ASSERT_CLEAN()                      assert_buffer = NULL;

#else
#define ASSERT_CHECK(cond, label)           ON_FALSE_DO_((cond), {ASSERT(cond);})
#define ASSERT_WAIT(timeout_ms)
#define ASSERT_IF_NOT_HIT_DO(label, code)
#define ASSERT_CLEAN()
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

typedef struct {
    char const * signature;
    void * connector_handle;
    ccapi_config_t config;
    struct {
        ccapi_thread_info_t * connector_run;
    } thread;
} ccapi_data_t;

extern ccapi_data_t * ccapi_data;
extern char const ccapi_signature[];
extern void ccapi_connector_run_thread(void * const argument);
extern void * ccapi_malloc(size_t size);
extern connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);

#endif
