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

#ifndef _CCAPI_FIRMWARE_UPDATE_H_
#define _CCAPI_FIRMWARE_UPDATE_H_

typedef enum {
    CCAPI_FIRMWARE_UPDATE_ERROR_NONE
} ccapi_firmware_update_error_t;

typedef ccapi_firmware_update_error_t (*ccapi_firmware_update_request_cb_t)(unsigned int target, char const * const filename, size_t const total_size);
typedef ccapi_firmware_update_error_t (*ccapi_firmware_update_data_cb_t)(unsigned int target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk);
typedef ccapi_firmware_update_error_t (*ccapi_firmware_update_cancel_cb_t)(unsigned int target);

typedef struct {
    struct {
        uint8_t major;
        uint8_t minor;
        uint8_t revision;
        uint8_t build;
    } version;
    char const * description;
    char const * filespec;
    size_t maximum_size;
    size_t chunk_size;
} firmware_target_t;

typedef struct {
    struct {
        firmware_target_t * list;
        uint8_t count;
    } target;
    struct {
        ccapi_firmware_update_request_cb_t request_cb;
        ccapi_firmware_update_data_cb_t data_cb;
        ccapi_firmware_update_cancel_cb_t cancel_cb;
    } callback;
} ccapi_firmware_update_service_t;

#endif
