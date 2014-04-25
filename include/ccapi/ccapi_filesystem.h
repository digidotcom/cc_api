#ifndef _CCAPI_FILESYSTEM_H_
#define _CCAPI_FILESYSTEM_H_

typedef enum {
  CCAPI_FS_REQUEST_READ,
  CCAPI_FS_REQUEST_WRITE,
  CCAPI_FS_REQUEST_READWRITE,
  CCAPI_FS_REQUEST_REMOVE,
  CCAPI_FS_REQUEST_LIST,
  CCAPI_FS_REQUEST_UNKNOWN
} ccapi_fs_request_t;

typedef enum {
    CCAPI_FS_CHANGED_MODIFIED,
    CCAPI_FS_CHANGED_REMOVED
} ccapi_fs_changed_t;

typedef enum {
    CCAPI_FS_ACCESS_DENY,
    CCAPI_FS_ACCESS_ALLOW
} ccapi_fs_access_t;

typedef ccapi_fs_access_t (*ccapi_fs_access_cb_t)(char const * const local_path, ccapi_fs_request_t const request);
typedef void (*ccapi_fs_changed_cb_t)(char const * const local_path, ccapi_fs_changed_t const request);

typedef struct {
    ccapi_fs_access_cb_t access_cb;
    ccapi_fs_changed_cb_t changed_cb;
} ccapi_filesystem_service_t;

typedef enum {
    CCAPI_FS_ERROR_NONE,
    CCAPI_FS_ERROR_CCAPI_STOPPED,
    CCAPI_FS_ERROR_INVALID_PATH,
    CCAPI_FS_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_FS_ERROR_NOT_A_DIR,
    CCAPI_FS_ERROR_NOT_MAPPED,
    CCAPI_FS_ERROR_ALREADY_MAPPED,
    CCAPI_FS_ERROR_EXISTING_VIRTUAL_DIR
} ccapi_fs_error_t;

ccapi_fs_error_t ccapi_fs_add_virtual_dir(char const * const virtual_dir, char const * const local_dir);
ccapi_fs_error_t ccapi_fs_remove_virtual_dir(char const * const virtual_dir);

#endif
