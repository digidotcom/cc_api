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

static unsigned int ccapi_firmware_start_expected_target;
static char const * ccapi_firmware_start_expected_filename;
static size_t ccapi_firmware_start_expected_total_size;
static ccapi_firmware_update_error_t ccapi_firmware_start_retval;
static ccapi_bool_t ccapi_firmware_start_cb_called;

static ccapi_firmware_update_error_t test_firmware_update_request_cb(unsigned int const target, char const * const filename, size_t const total_size)
{
    CHECK_EQUAL(ccapi_firmware_start_expected_target, target);
    STRCMP_EQUAL(ccapi_firmware_start_expected_filename, filename);
    CHECK_EQUAL(ccapi_firmware_start_expected_total_size, total_size);
    ccapi_firmware_start_cb_called = CCAPI_TRUE;
    return ccapi_firmware_start_retval;
}

TEST_GROUP(test_ccapi_firmware_update_start_no_callback)
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

TEST(test_ccapi_firmware_update_start_no_callback, testStartBadTarget)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = firmware_count;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);
}

TEST(test_ccapi_firmware_update_start_no_callback, testStartAlreadyStarted)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = 0;
    connector_firmware_download_start.code_size = firmware_list[0].maximum_size;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_success);

    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_device_error);
}

TEST(test_ccapi_firmware_update_start_no_callback, testStartBadSize)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = 0;
    connector_firmware_download_start.code_size = firmware_list[0].maximum_size + 1;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_download_invalid_size);
}

TEST(test_ccapi_firmware_update_start_no_callback, testStartOk_nocallback)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = 0;
    connector_firmware_download_start.code_size = firmware_list[0].maximum_size;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
}

TEST_GROUP(test_ccapi_firmware_update_start_callback)
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
                                                             test_firmware_update_request_cb, 
                                                             test_firmware_update_data_cb, 
                                                             NULL
                                                         }
                                                     };

        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.firmware = &fw_service;

        ccapi_firmware_start_expected_target = (unsigned int)-1;
        ccapi_firmware_start_expected_filename = "";
        ccapi_firmware_start_expected_total_size = 0;
        ccapi_firmware_start_retval = CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD;
        ccapi_firmware_start_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);

        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

#define TEST_FILENAME ((char*)"test.bin")

TEST(test_ccapi_firmware_update_start_callback, testStartRefuseDownload)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = 0;
    connector_firmware_download_start.filename = TEST_FILENAME;
    connector_firmware_download_start.code_size = firmware_list[0].maximum_size;

    ccapi_firmware_start_expected_target = connector_firmware_download_start.target_number;
    ccapi_firmware_start_expected_filename = connector_firmware_download_start.filename;
    ccapi_firmware_start_expected_total_size = connector_firmware_download_start.code_size;
    ccapi_firmware_start_retval = CCAPI_FIRMWARE_UPDATE_ERROR_REFUSE_DOWNLOAD;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_start_cb_called);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_download_denied);
}

TEST(test_ccapi_firmware_update_start_callback, testStartOk)
{
    connector_request_id_t request;
    connector_firmware_download_start_t connector_firmware_download_start;
    connector_callback_status_t status;

    connector_firmware_download_start.target_number = 0;
    connector_firmware_download_start.filename = TEST_FILENAME;
    connector_firmware_download_start.code_size = firmware_list[0].maximum_size;

    ccapi_firmware_start_expected_target = connector_firmware_download_start.target_number;
    ccapi_firmware_start_expected_filename = connector_firmware_download_start.filename;
    ccapi_firmware_start_expected_total_size = connector_firmware_download_start.code_size;
    ccapi_firmware_start_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_start;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_start_cb_called);

    CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
}