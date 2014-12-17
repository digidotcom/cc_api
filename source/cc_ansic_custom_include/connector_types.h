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

#ifndef CONNECTOR_TYPES_H_
#define CONNECTOR_TYPES_H_

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stddef.h>

#include "custom/custom_ccimp_types.h"

int connector_snprintf(char * const str, size_t const size, char const * const format, ...);

typedef ccimp_network_handle_t connector_network_handle_t;

typedef void * connector_filesystem_file_handle_t;
#define CONNECTOR_FILESYSTEM_FILE_HANDLE_NOT_INITIALIZED    NULL

typedef ccimp_fs_dir_handle_t connector_filesystem_dir_handle_t;
#define CONNECTOR_FILESYSTEM_DIR_HANDLE_NOT_INITIALIZED     CCIMP_FILESYSTEM_DIR_HANDLE_NOT_INITIALIZED

typedef void * connector_filesystem_errnum_t;
#define CONNECTOR_FILESYSTEM_ERRNUM_NONE    NULL


#endif
