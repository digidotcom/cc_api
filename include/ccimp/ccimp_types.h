#ifndef CCIMP_TYPES_H_
#define CCIMP_TYPES_H_

#include "custom/custom_ccimp_types.h"

typedef enum {
    CCAPI_FALSE,
    CCAPI_TRUE
} ccapi_bool_t;

typedef enum {
    CCIMP_STATUS_OK,
    CCIMP_STATUS_BUSY,
    CCIMP_STATUS_ERROR
} ccimp_status_t;

typedef struct {
    char * string;
    size_t length;
} ccapi_string_info_t;

typedef struct {
    void * buffer;
    size_t length;
} ccapi_buffer_info_t;

#endif /* CCIMP_TYPES_H_ */
