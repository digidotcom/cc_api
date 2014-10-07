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

#include "test_helper_functions.h"

firmware_target_t firmware_list[] = {
       /* version   description    filespec             maximum_size       chunk_size */
        {{1,0,0,0}, "Bootloader",  ".*\\.[bB][iI][nN]", 1 * 1024 * 1024,   128 * 1024 },  /* any *.bin files */
        {{0,0,1,0}, "Kernel",      ".*\\.a",            128 * 1024 * 1024, 128 * 1024 }   /* any *.a files */
    };

#define firmware_count asizeof(firmware_list)

TEST_GROUP(test_ccapi_firmware_update_init)
{
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_firmware_update_init, testBadtargetCount)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_firmware_update_service_t fw_service = {
                                                     {
                                                         firmware_list, 
                                                         0
                                                     }, 
                                                     {
                                                         NULL, 
                                                         NULL, 
                                                         NULL
                                                     }
                                                 };

    th_fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &fw_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_FIRMWARE_INFO);
}

TEST(test_ccapi_firmware_update_init, testBadtargetList)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_firmware_update_service_t fw_service = {
                                                     {
                                                         NULL, 
                                                         firmware_count
                                                     }, 
                                                     {
                                                         NULL, 
                                                         NULL, 
                                                         NULL
                                                     }
                                                 };

    th_fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &fw_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_FIRMWARE_INFO);
}

TEST(test_ccapi_firmware_update_init, testInitOk)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_firmware_update_service_t fw_service = {
                                                     {
                                                         firmware_list, 
                                                         firmware_count
                                                     }, 
                                                     {
                                                         NULL, 
                                                         NULL, 
                                                         NULL
                                                     }
                                                 };

    th_fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &fw_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->service.firmware_update.target.list == firmware_list);
    CHECK(ccapi_data_single_instance->service.firmware_update.target.count == firmware_count);

}


