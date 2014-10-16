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

#define MAX_CALLBACK_CALLS 2

static unsigned int ccapi_firmware_data_expected_target[MAX_CALLBACK_CALLS];
static uint32_t ccapi_firmware_data_expected_offset[MAX_CALLBACK_CALLS];
static void const * ccapi_firmware_data_expected_data[MAX_CALLBACK_CALLS];
static size_t ccapi_firmware_data_expected_size[MAX_CALLBACK_CALLS];
static ccapi_bool_t ccapi_firmware_data_expected_last_chunk[MAX_CALLBACK_CALLS];
static ccapi_fw_data_error_t ccapi_firmware_data_retval[MAX_CALLBACK_CALLS];
static uint8_t ccapi_firmware_data_cb_called;

static ccapi_fw_data_error_t test_firmware_update_data_cb(unsigned int const target, uint32_t offset, void const * const data, size_t size, ccapi_bool_t last_chunk)
{
    CHECK_EQUAL(ccapi_firmware_data_expected_target[ccapi_firmware_data_cb_called], target);
    CHECK_EQUAL(ccapi_firmware_data_expected_offset[ccapi_firmware_data_cb_called], offset);

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

    CHECK_EQUAL(ccapi_firmware_data_expected_size[ccapi_firmware_data_cb_called], size);
    CHECK_EQUAL(0, memcmp(data, ccapi_firmware_data_expected_data[ccapi_firmware_data_cb_called], size));


    CHECK_EQUAL(ccapi_firmware_data_expected_last_chunk[ccapi_firmware_data_cb_called], last_chunk);

    return ccapi_firmware_data_retval[ccapi_firmware_data_cb_called++];
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

        {
            unsigned int i;
            for (i=0; i<MAX_CALLBACK_CALLS; i++)
            { 
                ccapi_firmware_data_expected_target[i] = (unsigned int)-1;
                ccapi_firmware_data_expected_offset[i] = 0;
                ccapi_firmware_data_expected_data[i] = NULL;
                ccapi_firmware_data_expected_size[i] = 0;
                ccapi_firmware_data_expected_last_chunk[i] = CCAPI_FALSE;

                ccapi_firmware_data_retval[i] = CCAPI_FW_DATA_ERROR_INVALID_DATA;
                ccapi_firmware_data_cb_called = 0;
            }
        }

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

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data[0] = NULL;
    ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_device_error);

    CHECK_EQUAL(0, ccapi_firmware_data_cb_called);
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
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size; /* second chunk_size block */
    connector_firmware_download_data.image.data = NULL;
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_invalid_offset);

    CHECK_EQUAL(0, ccapi_firmware_data_cb_called);
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

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data[0] = &data[0];
    ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
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

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data[0] = &data[0];
    ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
    ccapi_firmware_data_cb_called = 0;

    /* second data block match second chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
    ccapi_firmware_data_expected_data[0] = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
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

    CHECK_EQUAL(0, ccapi_firmware_data_cb_called);

    /* second data block is half a chunk. We should get a callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = 0;
    ccapi_firmware_data_expected_data[0] = &data[0];
    ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
    ccapi_firmware_data_cb_called = 0;

    /* Third data block is half a chunk */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(0, ccapi_firmware_data_cb_called);

    /* second data block is half a chunk. We should get a callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_data[0] = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);

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

    CHECK_EQUAL(0, ccapi_firmware_data_cb_called);

    /* second data block is a chunk. We should get a callback and catch half of the data */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = 0;
    ccapi_firmware_data_expected_data[0] = &data[0];
    ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
    ccapi_firmware_data_cb_called = 0;

    /* Third data block is half a chunk. We should get the second callback */
    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2;
    connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size + firmware_list[TEST_TARGET].chunk_size / 2];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size / 2;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_data[0] = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(1, ccapi_firmware_data_cb_called);

}

TEST(test_ccapi_firmware_update_data_callback, testOneBlockIsTwoChunks)
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

    connector_firmware_download_data.target_number = TEST_TARGET;
    connector_firmware_download_data.image.offset = 0;
    connector_firmware_download_data.image.data = &data[0];
    connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].maximum_size;

    ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[0] = 0;
    ccapi_firmware_data_expected_data[0] = &data[0];
    ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
    ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

    ccapi_firmware_data_expected_target[1] = connector_firmware_download_data.target_number;
    ccapi_firmware_data_expected_offset[1] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_data[1] = &data[firmware_list[TEST_TARGET].chunk_size];
    ccapi_firmware_data_expected_size[1] = firmware_list[TEST_TARGET].chunk_size;
    ccapi_firmware_data_expected_last_chunk[1] = CCAPI_TRUE;
    ccapi_firmware_data_retval[1] = CCAPI_FW_DATA_ERROR_NONE;

    request.firmware_request = connector_request_id_firmware_download_data;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

    CHECK_EQUAL(2, ccapi_firmware_data_cb_called);
}

TEST(test_ccapi_firmware_update_data_callback, testDataCompleteBeforeAllDataArrives)
{
    connector_request_id_t request;

    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    const uint8_t BYTES_TO_MAX = 5;
    const uint8_t MISSING_DATA_BYTES = 3;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size - BYTES_TO_MAX;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    {
        connector_firmware_download_data_t connector_firmware_download_data;

        /* First data block match first chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = 0;
        connector_firmware_download_data.image.data = &data[0];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

        ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
        ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
        ccapi_firmware_data_expected_data[0] = &data[0];
        ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
        ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
        ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
        ccapi_firmware_data_cb_called = 0;

        /* second data block less than second chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
        connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size - BYTES_TO_MAX - MISSING_DATA_BYTES;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(0, ccapi_firmware_data_cb_called);
    }

    {
        connector_firmware_download_complete_t connector_firmware_download_complete;

        connector_firmware_download_complete.target_number = TEST_TARGET;

        request.firmware_request = connector_request_id_firmware_download_complete;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_complete, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_complete.status == connector_firmware_download_not_complete);

        CHECK_EQUAL(0, ccapi_firmware_data_cb_called);

        CHECK(ccapi_data_single_instance->service.firmware_update.service.chunk_data == NULL);
    }
}

TEST(test_ccapi_firmware_update_data_callback, testDataCompleteHasNoDataToFlush)
{
    connector_request_id_t request;

    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    const uint8_t BYTES_TO_MAX = 0;
    const uint8_t MISSING_DATA_BYTES = 0;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size - BYTES_TO_MAX;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    {
        connector_firmware_download_data_t connector_firmware_download_data;

        /* First data block match first chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = 0;
        connector_firmware_download_data.image.data = &data[0];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

        ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
        ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
        ccapi_firmware_data_expected_data[0] = &data[0];
        ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
        ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
        ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
        ccapi_firmware_data_cb_called = 0;

        /* second data block match second chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
        connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size - BYTES_TO_MAX - MISSING_DATA_BYTES;

        ccapi_firmware_data_expected_target[0] = TEST_TARGET;
        ccapi_firmware_data_expected_offset[0] = firmware_list[TEST_TARGET].chunk_size;
        ccapi_firmware_data_expected_data[0] = &data[firmware_list[TEST_TARGET].chunk_size];
        ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size - BYTES_TO_MAX - MISSING_DATA_BYTES;
        ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
        ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
        ccapi_firmware_data_cb_called = 0;
    }

    {
        connector_firmware_download_complete_t connector_firmware_download_complete;

        connector_firmware_download_complete.target_number = TEST_TARGET;

        request.firmware_request = connector_request_id_firmware_download_complete;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_complete, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_complete.status == connector_firmware_download_success);

        CHECK_EQUAL(0, ccapi_firmware_data_cb_called);

        CHECK(ccapi_data_single_instance->service.firmware_update.service.chunk_data == NULL);
    }
}

TEST(test_ccapi_firmware_update_data_callback, testDataCompleteFlushLastData)
{
    connector_request_id_t request;

    connector_callback_status_t status;
    uint8_t const data[] = DATA;

    const uint8_t BYTES_TO_MAX = 5;
    const uint8_t MISSING_DATA_BYTES = 0;

    {
        connector_firmware_download_start_t connector_firmware_download_start;

        connector_firmware_download_start.target_number = TEST_TARGET;
        connector_firmware_download_start.code_size = firmware_list[TEST_TARGET].maximum_size - BYTES_TO_MAX;

        request.firmware_request = connector_request_id_firmware_download_start;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_start, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_start.status == connector_firmware_status_success);
    }

    {
        connector_firmware_download_data_t connector_firmware_download_data;

        /* First data block match first chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = 0;
        connector_firmware_download_data.image.data = &data[0];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size;

        ccapi_firmware_data_expected_target[0] = connector_firmware_download_data.target_number;
        ccapi_firmware_data_expected_offset[0] = connector_firmware_download_data.image.offset;
        ccapi_firmware_data_expected_data[0] = &data[0];
        ccapi_firmware_data_expected_size[0] = connector_firmware_download_data.image.bytes_used;
        ccapi_firmware_data_expected_last_chunk[0] = CCAPI_FALSE;
        ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(1, ccapi_firmware_data_cb_called);
        ccapi_firmware_data_cb_called = 0;

        /* second data block less than second chunk */
        connector_firmware_download_data.target_number = TEST_TARGET;
        connector_firmware_download_data.image.offset = firmware_list[TEST_TARGET].chunk_size;
        connector_firmware_download_data.image.data = &data[firmware_list[TEST_TARGET].chunk_size];
        connector_firmware_download_data.image.bytes_used = firmware_list[TEST_TARGET].chunk_size - BYTES_TO_MAX - MISSING_DATA_BYTES;

        request.firmware_request = connector_request_id_firmware_download_data;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_data, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);

        CHECK(connector_firmware_download_data.status == connector_firmware_status_success);

        CHECK_EQUAL(0, ccapi_firmware_data_cb_called);
    }

    {
        connector_firmware_download_complete_t connector_firmware_download_complete;

        connector_firmware_download_complete.target_number = TEST_TARGET;

        ccapi_firmware_data_expected_target[0] = TEST_TARGET;
        ccapi_firmware_data_expected_offset[0] = firmware_list[TEST_TARGET].chunk_size;
        ccapi_firmware_data_expected_data[0] = &data[firmware_list[TEST_TARGET].chunk_size];
        ccapi_firmware_data_expected_size[0] = firmware_list[TEST_TARGET].chunk_size - BYTES_TO_MAX - MISSING_DATA_BYTES;
        ccapi_firmware_data_expected_last_chunk[0] = CCAPI_TRUE;
        ccapi_firmware_data_retval[0] = CCAPI_FW_DATA_ERROR_NONE;

        request.firmware_request = connector_request_id_firmware_download_complete;
        status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_download_complete, ccapi_data_single_instance);
        CHECK_EQUAL(connector_callback_continue, status);
        CHECK(connector_firmware_download_complete.status == connector_firmware_download_success);

        CHECK_EQUAL(1, ccapi_firmware_data_cb_called);

        CHECK(ccapi_data_single_instance->service.firmware_update.service.chunk_data == NULL);
    }
}

