/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#ifndef _CCIMP_TYPES_H_
#define _CCIMP_TYPES_H_

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
