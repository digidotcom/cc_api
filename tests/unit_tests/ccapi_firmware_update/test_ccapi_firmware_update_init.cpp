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

static firmware_target_t firmware_list[] = {
       /* version   description           filespec                    maximum_size       chunk_size */
        {{1,0,0,0}, (char*)"Bootloader",  (char*)".*\\.[bB][iI][nN]", 1 * 1024 * 1024,   128 * 1024 },  /* any *.bin files */
        {{0,0,1,0}, (char*)"Kernel",      (char*)".*\\.a",            128 * 1024 * 1024, 128 * 1024 }   /* any *.a files */
    };
static uint8_t firmware_count = asizeof(firmware_list);

static ccapi_firmware_update_error_t test_firmware_update_data_cb(unsigned int const target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk)
{
    (void)target;
    (void)offset;
    (void)data;
    (void)size;
    (void)last_chunk;

    return CCAPI_FIRMWARE_UPDATE_ERROR_NONE;
}

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

TEST(test_ccapi_firmware_update_init, testBadDataCallback)
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

    CHECK(error == CCAPI_START_ERROR_INVALID_FIRMWARE_DATA_CALLBACK);
}

TEST(test_ccapi_firmware_update_init, testBadTargetCount)
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
                                                         test_firmware_update_data_cb, 
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
                                                         test_firmware_update_data_cb, 
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
                                                         test_firmware_update_data_cb, 
                                                         NULL
                                                     }
                                                 };

    th_fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &fw_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);
}

TEST_GROUP(test_ccapi_firmware_update_init_callback)
{
    void setup()
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
                                                             test_firmware_update_data_cb, 
                                                             NULL
                                                         }
                                                     };

        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.firmware = &fw_service;

        error = ccapi_start(&start);

        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_firmware_update_init_callback, testInitOkCallbackCount)
{
    connector_request_id_t request;
    connector_firmware_count_t ccfsm_firmware_count;
    connector_callback_status_t status;

    request.firmware_request = connector_request_id_firmware_target_count;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &ccfsm_firmware_count, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_firmware_count.count == firmware_count);
}

TEST(test_ccapi_firmware_update_init_callback, testInitOkCallbackList)
{
    connector_request_id_t request;
    connector_firmware_info_t ccfsm_request_id_firmware_info;
    connector_callback_status_t status;
    unsigned char target;

    /* Check valid targets */
    for (target=0 ; target < firmware_count ; target++)
    {
        ccfsm_request_id_firmware_info.target_number = target;

        request.firmware_request = connector_request_id_firmware_info;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &ccfsm_request_id_firmware_info, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(ccfsm_request_id_firmware_info.version.major == firmware_list[target].version.major);
        CHECK(ccfsm_request_id_firmware_info.version.minor == firmware_list[target].version.minor);
        CHECK(ccfsm_request_id_firmware_info.version.revision == firmware_list[target].version.revision);
        CHECK(ccfsm_request_id_firmware_info.version.build == firmware_list[target].version.build);

        STRCMP_EQUAL(ccfsm_request_id_firmware_info.description, firmware_list[target].description);
        STRCMP_EQUAL(ccfsm_request_id_firmware_info.filespec, firmware_list[target].filespec);
    }

    /* Check invalid targets */
    ccfsm_request_id_firmware_info.target_number = firmware_count;

    request.firmware_request = connector_request_id_firmware_info;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &ccfsm_request_id_firmware_info, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);
}

 
