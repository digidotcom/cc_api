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
        {{0,0,1,0}, (char*)"Test",        (char*)".*\\.test",         32,                16         }   /* any *.test files */
    };
static uint8_t firmware_count = asizeof(firmware_list);

#define TEST_TARGET 0

#define DATA  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, \
                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, \
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f }

static unsigned int ccapi_firmware_data_expected_target;
static uint32_t ccapi_firmware_data_expected_offset;
static void const * ccapi_firmware_data_expected_data;
static size_t ccapi_firmware_data_expected_size;
static ccapi_bool_t ccapi_firmware_data_expected_last_chunk;
static ccapi_firmware_update_error_t ccapi_firmware_data_retval;
static ccapi_bool_t ccapi_firmware_data_cb_called;

static ccapi_firmware_update_error_t test_firmware_update_data_cb(unsigned int const target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk)
{
    CHECK_EQUAL(ccapi_firmware_data_expected_target, target);
    CHECK_EQUAL(ccapi_firmware_data_expected_offset, offset);

#if 0
    {
        unsigned int i;
        uint8_t * pData = (uint8_t*)data;
        for (i=0; i<size; i++)
        {
            printf("%d=0x%x\r\n", i, pData[i]);
        }
    }
#endif

    CHECK_EQUAL(ccapi_firmware_data_expected_size, size);
    CHECK_EQUAL(0, memcmp(data, ccapi_firmware_data_expected_data, size));


    CHECK_EQUAL(ccapi_firmware_data_expected_last_chunk, last_chunk);

    ccapi_firmware_data_cb_called = CCAPI_TRUE;
    return ccapi_firmware_data_retval;
}

TEST_GROUP(test_ccapi_firmware_update_data_callback)
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

        ccapi_firmware_data_expected_target = (unsigned int)-1;
        ccapi_firmware_data_expected_offset = 0;
        ccapi_firmware_data_expected_data = NULL;
        ccapi_firmware_data_expected_size = 0;
        ccapi_firmware_data_expected_last_chunk = CCAPI_FALSE;

        ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_INVALID_DATA;
        ccapi_firmware_data_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);

        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_firmware_update_data_callback, testDataStartNotCalled)
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;

    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0x10000;
    connector_firmware_download_data.image.data = NULL;
    connector_firmware_download_data.image.bytes_used = 1024;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data = NULL;
    ccapi_firmware_data_expected_size = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk = CCAPI_FALSE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_device_error);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_firmware_data_cb_called);
}

TEST(test_ccapi_firmware_update_data_callback, testDataBadInitialOffset) /* TODO: Do one with bad intermediate offset */
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size; /* second chuck_size block */
    connector_firmware_download_data.image.data = NULL;
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_invalid_offset);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_firmware_data_cb_called);
}

TEST(test_ccapi_firmware_update_data_callback, testDataTotalsizeEqualsChunksize)
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].chunk_size;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0;
    connector_firmware_download_data.image.data = &data[0];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data = &data[0];
    ccapi_firmware_data_expected_size = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk = CCAPI_TRUE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);
}

TEST(test_ccapi_firmware_update_data_callback, testDataTwoBlocksThatMachChuncks)
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    /* First data block match first chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0;
    connector_firmware_download_data.image.data = &data[0];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data = &data[0];
    ccapi_firmware_data_expected_size = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk = CCAPI_FALSE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);

    /* second data block match second chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk = CCAPI_TRUE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);
}

TEST(test_ccapi_firmware_update_data_callback, testDataFourBlocksSmallerThanChuncks)
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    /* First data block is half a chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0;
    connector_firmware_download_data.image.data = &data[0];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_firmware_data_cb_called);

    /* second data block is half a chunk. We should get a callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = 0;
    ccapi_firmware_data_expected_data = &data[0];
    ccapi_firmware_data_expected_size = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk = CCAPI_FALSE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);
    ccapi_firmware_data_cb_called = CCAPI_FALSE;

    /* Third data block is half a chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_firmware_data_cb_called);

    /* second data block is half a chunk. We should get a callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_data = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk = CCAPI_TRUE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);

}

TEST(test_ccapi_firmware_update_data_callback, testDataThreeBlocksThatDoNotMatchChunckBoundaries)
{
    connector_request_id_t request;
    connector_firmware_download_data_t connector_firmware_download_data;
    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    /* First data block is half a chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0;
    connector_firmware_download_data.image.data = &data[0];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_firmware_data_cb_called);

    /* second data block is a chunk. We should get a callback and catch half of the data */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = 0;
    ccapi_firmware_data_expected_data = &data[0];
    ccapi_firmware_data_expected_size = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk = CCAPI_FALSE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);
    ccapi_firmware_data_cb_called = CCAPI_FALSE;

    /* Third data block is half a chunk. We should get the second callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_data = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk = CCAPI_TRUE;
    ccapi_firmware_data_retval = CCAPI_FIRMWARE_UPDATE_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_firmware_data_cb_called);

}

