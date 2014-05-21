/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#if !(defined _CCAPI_DEFINITIONS_H_)
#define _CCAPI_DEFINITIONS_H_

#include <ctype.h>

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

#define CCAPI_BOOL_TO_CONNECTOR_BOOL(a)     ((a) == CCAPI_TRUE ? connector_true : connector_false)
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
    char * local_dir;
    size_t virtual_dir_length;
    size_t local_dir_length;
    struct ccapi_fs_virtual_dir * next;
} ccapi_fs_virtual_dir_t;

#if (defined CCIMP_DATA_SERVICE_ENABLED)
typedef struct ccapi_receive_target
{
    char * target;
    struct {
        ccapi_receive_data_cb_t data_cb;
        ccapi_receive_status_cb_t status_cb;
    } user_callbacks;
    size_t max_request_size;
    struct ccapi_receive_target * next;
} ccapi_receive_target_t;
#endif

typedef struct {
    void * connector_handle;
    ccapi_config_t config;
    struct {
        ccapi_thread_info_t * connector_run;
    } thread;
    void * initiate_action_syncr;
    void * file_system_syncr;
    struct {
        struct {
            ccapi_filesystem_service_t user_callbacks;
            ccapi_fs_virtual_dir_t * virtual_dir_list;
            void * imp_context;
        } file_system;
#if (defined CCIMP_DATA_SERVICE_ENABLED)
        struct {
            ccapi_receive_service_t user_callbacks;
            void * receive_syncr;
            ccapi_receive_target_t * target_list;
        } receive;
#endif
    } service;
    struct {
        ccapi_tcp_info_t * info;
        ccapi_bool_t connected;
    } transport_tcp;
#if (defined CCIMP_UDP_TRANSPORT_ENABLED)
    struct {
        ccapi_udp_info_t * info;
        ccapi_bool_t started;
    } transport_udp;
#endif
#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
    struct {
        ccapi_sms_info_t * info;
        ccapi_bool_t started;
    } transport_sms;
#endif
} ccapi_data_t;

#if (defined CCIMP_DATA_SERVICE_ENABLED)
typedef struct
{
    ccapi_data_t * ccapi_data;
    ccapi_bool_t sending_file;
    void * next_data;
    ccimp_fs_handle_t file_handler;
    size_t bytes_remaining;
    void * send_syncr;
    ccapi_send_error_t request_error;
    ccapi_send_error_t response_error;
    ccapi_send_error_t status_error;
    ccapi_string_info_t * hint;
} ccapi_svc_send_t;

typedef struct
{
    char * target;
    ccapi_bool_t response_required;
    struct {
        ccapi_receive_data_cb_t data_cb;
        ccapi_receive_status_cb_t status_cb;
    } user_callbacks;
    size_t max_request_size;
    ccapi_buffer_info_t request_buffer_info;
    ccapi_buffer_info_t response_buffer_info;
    ccapi_buffer_info_t response_processing;
    ccapi_receive_error_t receive_error;
} ccapi_svc_receive_t;

ccapi_receive_target_t * * get_pointer_to_target_entry(ccapi_data_t * const ccapi_data, char const * const target);
#endif

#if (defined CCIMP_DATA_POINTS_ENABLED)

typedef enum {
    CCAPI_DP_ARG_DATA_INT32,
    CCAPI_DP_ARG_DATA_INT64,
    CCAPI_DP_ARG_DATA_FLOAT,
    CCAPI_DP_ARG_DATA_DOUBLE,
    CCAPI_DP_ARG_DATA_STRING,
    CCAPI_DP_ARG_TIME_EPOCH,
    CCAPI_DP_ARG_TIME_EPOCH_MSEC,
    CCAPI_DP_ARG_TIME_ISO8601,
    CCAPI_DP_ARG_LOC,
    CCAPI_DP_ARG_QUAL,
    CCAPI_DP_ARG_INVALID
} ccapi_dp_argument_t;

typedef struct ccapi_dp_data_stream {
    connector_data_stream_t * ccfsm_data_stream;
    struct {
        ccapi_dp_argument_t * list;
        unsigned int count;
    } arguments;
    struct ccapi_dp_data_stream * next;
} ccapi_dp_data_stream_t;

typedef struct stream_seen {
    char * stream_id;
    struct stream_seen * next;
} stream_seen_t;

typedef struct ccapi_dp_collection {
    ccapi_dp_data_stream_t * ccapi_data_stream_list;
    void * syncr;
} ccapi_dp_collection_t;

typedef struct {
    void * syncr;
    ccapi_string_info_t * hint;
    ccapi_dp_error_t response_error;
    ccapi_dp_error_t status;
} ccapi_dp_transaction_info_t;

#endif

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
typedef enum {
    CCAPI_FS_INTERNAL_ERROR_ACCESS_DENIED,
    CCAPI_FS_INTERNAL_ERROR_INVALID_PATH
} ccapi_fs_internal_error_t;

typedef struct {
    ccapi_bool_t error_is_internal;
    union {
        void * ccimp_error;
        ccapi_fs_internal_error_t ccapi_error;
    } error;
} ccapi_fs_error_handle_t;
#endif

extern ccapi_data_t * ccapi_data_single_instance;
extern void * logging_syncr;

void ccapi_connector_run_thread(void * const argument);
void * ccapi_malloc(size_t size);
ccimp_status_t ccapi_free(void * ptr);
char * ccapi_strdup(char const * const string);

void * ccapi_syncr_create(void);
void * ccapi_syncr_create_and_release(void);
ccimp_status_t ccapi_syncr_acquire(void * syncr_object);
ccimp_status_t ccapi_syncr_release(void * syncr_object);
ccimp_status_t ccapi_syncr_destroy(void * syncr_object);
connector_status_t ccapi_initiate_transport_stop(ccapi_data_t * const ccapi_data, ccapi_transport_t transport, ccapi_stop_t behavior);
ccimp_status_t ccapi_open_file(ccapi_data_t * const ccapi_data, char const * const local_path, int const flags, ccimp_fs_handle_t * file_handler);
ccimp_status_t ccapi_read_file(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler, void * const data, size_t const bytes_available, size_t * const bytes_used);
ccimp_status_t ccapi_close_file(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler);
connector_transport_t ccapi_to_connector_transport(ccapi_transport_t const ccapi_transport);
ccimp_status_t ccapi_get_dir_entry_status(ccapi_data_t * const ccapi_data, char const * const local_path, ccimp_fs_stat_t * const fs_status);
connector_status_t connector_initiate_action_secure(ccapi_data_t * const ccapi_data, connector_initiate_request_t const request, void const * const request_data);

connector_callback_status_t connector_callback_status_from_ccimp_status(ccimp_status_t const ccimp_status);

connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context);
ccapi_fs_virtual_dir_t * * get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data_t * const ccapi_data, char const * const virtual_dir, unsigned int virtual_dir_length);

#if (defined CCIMP_FILE_SYSTEM_SERVICE_ENABLED)
connector_callback_status_t ccapi_filesystem_handler(connector_request_id_file_system_t filesystem_request, void * const data, ccapi_data_t * const ccapi_data);
#endif

void ccapi_logging_line(char const * const format, ...);
void ccapi_logging_print_buffer(char const * const label, void const * const buffer, size_t const length);

#endif
