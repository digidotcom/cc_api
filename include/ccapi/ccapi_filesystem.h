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

#endif
