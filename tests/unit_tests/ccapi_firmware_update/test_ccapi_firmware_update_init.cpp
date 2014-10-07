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
                                                             NULL, 
                                                             NULL
                                                         }
                                                     };

        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.firmware = &fw_service;

        error = ccapi_start(&start);

        CHECK(error == CCAPI_START_ERROR_NONE);

        CHECK(ccapi_data_single_instance->service.firmware_update.target.list == firmware_list);
        CHECK(ccapi_data_single_instance->service.firmware_update.target.count == firmware_count);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_firmware_update_init_callback, testInitOkCallbackCount)
{
    connector_request_id_t request;
    connector_firmware_count_t connector_firmware_count;
    connector_callback_status_t status;

    request.firmware_request = connector_request_id_firmware_target_count;
    status = ccapi_connector_callback(connector_class_id_firmware, request, &connector_firmware_count, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(connector_firmware_count.count == firmware_count);
}
 
/*
TEST(test_ccapi_receive_add_target, testTargetAdded)
{
    ccapi_receive_error_t error;

    connector_request_id_t request;
    connector_data_service_receive_target_t ccfsm_receive_target_data;
    connector_callback_status_t status;

    error = ccapi_receive_add_target(TEST_TARGET, specific_receive_data_cb, specific_receive_status_cb, CCAPI_RECEIVE_NO_LIMIT);
    CHECK_EQUAL(CCAPI_RECEIVE_ERROR_NONE, error);

    ccfsm_receive_target_data.transport = connector_transport_tcp;
    ccfsm_receive_target_data.user_context = NULL;
    ccfsm_receive_target_data.target = TEST_TARGET;
    ccfsm_receive_target_data.response_required = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_target;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_target_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_target_data.user_context != NULL);

    {
        ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)ccfsm_receive_target_data.user_context;
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive->user_callbacks.data_cb == specific_receive_data_cb);
        CHECK(svc_receive->user_callbacks.status_cb == specific_receive_status_cb);
    }

    CHECK_EQUAL(CCAPI_FALSE, generic_receive_accept_cb_called);
}

TEST(test_ccapi_receive_add_target, testTargetNotAdded)
{
    ccapi_receive_error_t error;

    connector_request_id_t request;
    connector_data_service_receive_target_t ccfsm_receive_target_data;
    connector_callback_status_t status;

    error = ccapi_receive_add_target("other_target", specific_receive_data_cb, specific_receive_status_cb, CCAPI_RECEIVE_NO_LIMIT);
    CHECK_EQUAL(CCAPI_RECEIVE_ERROR_NONE, error);

    ccfsm_receive_target_data.transport = connector_transport_tcp;
    ccfsm_receive_target_data.user_context = NULL;
    ccfsm_receive_target_data.target = TEST_TARGET;
    ccfsm_receive_target_data.response_required = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_target;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_target_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_target_data.user_context != NULL);

    {
        ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)ccfsm_receive_target_data.user_context;
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive->user_callbacks.data_cb == generic_receive_data_cb);
        CHECK(svc_receive->user_callbacks.status_cb == generic_receive_status_cb);
    }

    CHECK_EQUAL(CCAPI_TRUE, generic_receive_accept_cb_called);
}
*/