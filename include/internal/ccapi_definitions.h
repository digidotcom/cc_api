/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccimp/ccimp_types.h"

#define DEVICE_ID_LENGTH    16

#define UNUSED_ARGUMENT(a)  (void)(a)

typedef enum {
    CCAPI_FALSE,
    CCAPI_TRUE
} ccapi_bool_t;

typedef struct {
    uint32_t vendor_id;
    uint8_t device_id[DEVICE_ID_LENGTH];
    char * device_type;
    ccapi_bool_t cli_supported;
    ccapi_bool_t receive_supported;
    ccapi_bool_t firmware_supported;
    ccapi_bool_t rci_supported;
    ccapi_bool_t filesystem_supported;
} ccapi_config_t;

extern ccapi_config_t * ccapi_config;
#endif
