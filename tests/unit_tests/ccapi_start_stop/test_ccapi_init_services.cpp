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

static void test_receive_data_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info)
{
    (void)target;
    (void)transport;
    (void)request_buffer_info;
    (void)response_buffer_info;
}

TEST_GROUP(test_ccapi_init_services)
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

TEST(test_ccapi_init_services, testServicesNotSupported)
{
    th_start_ccapi();
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE);
}

TEST(test_ccapi_init_services, testServicesSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_filesystem_service_t fs_service = {NULL, NULL};
    ccapi_receive_service_t receive_service = {NULL, test_receive_data_cb, NULL};
    void * pointer = pointer; /* Not-NULL */

    th_fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    start.service.receive = &receive_service;
    start.service.firmware = &pointer;
    start.service.rci = &pointer;
    start.service.file_system = &fs_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_TRUE);
    CHECK_EQUAL(receive_service.accept_cb, ccapi_data_single_instance->service.receive.user_callbacks.accept_cb);
    CHECK_EQUAL(receive_service.data_cb, ccapi_data_single_instance->service.receive.user_callbacks.data_cb);
    CHECK_EQUAL(receive_service.status_cb, ccapi_data_single_instance->service.receive.user_callbacks.status_cb);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_TRUE);
    CHECK_EQUAL(fs_service.access_cb, ccapi_data_single_instance->service.file_system.user_callbacks.access_cb);
    CHECK_EQUAL(fs_service.changed_cb, ccapi_data_single_instance->service.file_system.user_callbacks.changed_cb);
    CHECK(NULL == ccapi_data_single_instance->service.file_system.imp_context);
    CHECK(NULL == ccapi_data_single_instance->service.file_system.virtual_dir_list);
    CHECK(NULL != ccapi_data_single_instance->file_system_syncr);
}
