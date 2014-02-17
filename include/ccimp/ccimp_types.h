/*
 * ccimp_types.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_TYPES_H_
#define CCIMP_TYPES_H_

#include "connector_types.h"

#define DEVICE_ID_LENGTH    16

typedef void (* ccimp_thread_start_t) (void * const argument);

typedef enum {
    CCAPI_FALSE,
    CCAPI_TRUE
} ccapi_bool_t;

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
    CCIMP_CONNECTOR_THREAD
} ccimp_thread_type_t;

typedef struct
{
    ccimp_thread_start_t thread_start;
    void * argument;
    ccimp_thread_type_t thread_type;
} ccimp_create_thread_info_t;

#endif /* CCIMP_TYPES_H_ */
