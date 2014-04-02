/*
 * ccimp_types.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_TYPES_H_
#define CCIMP_TYPES_H_

#include "custom/custom_ccimp_types.h"

#define CCAPI_DEVICE_ID_LENGTH    16
#define CCAPI_MAC_ADDR_LENGTH     6
#define CCAPI_IPV4_LENGTH         4
#define CCAPI_IPV6_LENGTH         16

typedef enum {
    CCAPI_FALSE,
    CCAPI_TRUE
} ccapi_bool_t;

typedef enum {
    CCIMP_STATUS_OK,
    CCIMP_STATUS_BUSY,
    CCIMP_STATUS_ABORT
} ccimp_status_t;

#endif /* CCIMP_TYPES_H_ */
