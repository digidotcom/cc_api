/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "connector_config.h"
#include "ccapi/ccapi.h"
#include "ccimp/ccimp_hal.h"
#include "ccimp/ccimp_logging.h"
#include "ccimp/ccimp_os.h"
#include "ccimp/ccimp_network.h"
#include "ccimp/ccimp_filesystem.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)

#define ON_FALSE_DO_(cond, code)        do { if (!(cond)) {code;} } while (0)

#if (defined CCIMP_DEBUG_ENABLED)
#define ASSERT_MSG_GOTO(cond, label)   ON_FALSE_DO_((cond), \
                                           { \
                                               ccapi_logging_line(TMP_FATAL_PREFIX "Following condition '%s' failed in file '%s' line %d", #cond, __FILE__, __LINE__); \
                                               ccimp_hal_halt(); \
                                               goto label; \
                                           })
#define ASSERT_MSG(cond)               ON_FALSE_DO_((cond), \
                                           { \
                                               ccapi_logging_line(TMP_FATAL_PREFIX "Following condition '%s' failed in file '%s' line %d", #cond, __FILE__, __LINE__); \
                                               ccimp_hal_halt(); \
                                           })
#else
#define ASSERT_MSG_GOTO(cond, label)   ON_FALSE_DO_((cond), {goto label;})
#define ASSERT_MSG(cond)
#endif

#define reset_heap_ptr(pp) do { if (*(pp) != NULL) { ccapi_free(*(pp)); *(pp) = NULL; } } while (0)
#define CCAPI_BOOL(v)   (!!(v) ? CCAPI_TRUE : CCAPI_FALSE)
#define CCAPI_RUNNING(c) ((c) != NULL && (c)->thread.connector_run->status == CCAPI_THREAD_RUNNING)

#define CCAPI_MAX_OF(a, b)          ((a) > (b) ? (a) : (b))
#define CCAPI_MIN_OF(a, b)          ((a) < (b) ? (a) : (b))
#define CCAPI_FS_DIR_SEPARATOR      '/'
#define CCAPI_FS_ROOT_PATH          "/"

typedef struct {
    uint32_t vendor_id;
    uint8_t device_id[16];
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

typedef struct ccapi_fs_virtual_dir {
    char * virtual_dir;
    char * local_path;
    size_t virtual_dir_length;
    size_t local_path_length;
    struct ccapi_fs_virtual_dir * next;
} ccapi_fs_virtual_dir_t;

typedef struct {
    void * connector_handle;
    ccapi_config_t config;
    struct {
        ccapi_thread_info_t * connector_run;
    } thread;
    void * initiate_action_syncr;
    struct {
        struct {
            ccapi_filesystem_service_t user_callbacks;
            ccapi_fs_virtual_dir_t * virtual_dir_list;
            void * imp_context;
        } file_system;
    } service;
    struct {
        ccapi_tcp_info_t * info;
        ccapi_bool_t connected;
    } transport_tcp;
#ifdef CCIMP_UDP_TRANSPORT_ENABLED
    struct {
        ccapi_bool_t started;
    } transport_udp;
#endif
#ifdef CCIMP_SMS_TRANSPORT_ENABLED
    struct {
        ccapi_bool_t started;
    } transport_sms;
#endif
} ccapi_data_t;

#ifdef CCIMP_DATA_SERVICE_ENABLED
typedef struct
{
    void * next_data;
    size_t bytes_remaining;
    void * send_syncr;
    ccapi_send_error_t error;
} ccapi_svc_send_data_t;
#endif

extern ccapi_data_t * ccapi_data_single_instance;
extern void * logging_syncr;

void ccapi_connector_run_thread(void * const argument);
void * ccapi_malloc(size_t size);
ccimp_status_t ccapi_free(void * ptr);
char * ccapi_strdup(char const * const string);
ccimp_status_t ccapi_syncr_release(void * syncr_object);
ccimp_status_t ccapi_syncr_destroy(void * syncr_object);
connector_status_t connector_initiate_action_secure(ccapi_data_t * const ccapi_data, connector_initiate_request_t const request, void const * const request_data);

connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context);
ccapi_fs_virtual_dir_t * * get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data_t * const ccapi_data, char const * const virtual_dir, unsigned int virtual_dir_length);

void ccapi_logging_line(char const * const format, ...);
void ccapi_logging_print_buffer(char const * const label, void const * const buffer, size_t const length);

#endif
