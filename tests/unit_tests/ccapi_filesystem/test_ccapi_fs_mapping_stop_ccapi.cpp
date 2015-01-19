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

TEST_GROUP(test_ccapi_fs_mapping_stop_ccapi)
{
    void * malloc_for_ccapi_data;
    void * malloc_for_device_type;
    void * malloc_for_device_cloud_url;
    void * malloc_for_dir_list_entry_1;
    void * malloc_for_local_path_1;
    void * malloc_for_virtual_path_1;
    void * malloc_for_dir_list_entry_2;
    void * malloc_for_local_path_2;
    void * malloc_for_virtual_path_2;
    void * malloc_for_thread_connector_run;

    void setup()
    {
        static ccimp_os_create_thread_info_t mem_for_thread_connector_run;
        ccapi_start_t start;
        ccapi_filesystem_service_t fs_service = {NULL, NULL};
        ccapi_start_error_t start_error;
        ccapi_fs_error_t ccapi_fs_error;
        ccimp_fs_dir_entry_status_t ccimp_fs_dir_entry_status_data_1, ccimp_fs_dir_entry_status_data_2;
        char const * const local_path_1 = "/home/user/my_directory";
        char const * const virtual_path_1 = "my_virtual_dir";
        char const * const local_path_2 = "/home/other_user/other_my_directory";
        char const * const virtual_path_2 = "other_virtual_dir";

        Mock_create_all();

        malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
        malloc_for_device_type = malloc(sizeof TH_DEVICE_TYPE_STRING);
        malloc_for_device_cloud_url = malloc(sizeof TH_DEVICE_CLOUD_URL_STRING);
        malloc_for_dir_list_entry_1 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        malloc_for_local_path_1 = malloc(strlen(local_path_1) + 1);
        malloc_for_virtual_path_1 = malloc(strlen(virtual_path_1) + 1);
        malloc_for_dir_list_entry_2 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        malloc_for_local_path_2 = malloc(strlen(local_path_2) + 1);
        malloc_for_virtual_path_2 = malloc(strlen(virtual_path_2) + 1);
        malloc_for_thread_connector_run = &mem_for_thread_connector_run;

        Mock_ccimp_os_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof(TH_DEVICE_TYPE_STRING), malloc_for_device_type);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof(TH_DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), malloc_for_thread_connector_run);

        th_fill_start_structure_with_good_parameters(&start);
        start.service.file_system = &fs_service;
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);
        /* Simulate that imp_context was previously set by other call (file_open) */
        ccapi_data_single_instance->service.file_system.imp_context = &my_fs_context;

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_1) + 1, malloc_for_local_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_1) + 1, malloc_for_virtual_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_2) + 1, malloc_for_local_path_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_2) + 1, malloc_for_virtual_path_2);

        th_filesystem_prepare_ccimp_dir_entry_status_call(&ccimp_fs_dir_entry_status_data_1, local_path_1);
        th_filesystem_prepare_ccimp_dir_entry_status_call(&ccimp_fs_dir_entry_status_data_2, local_path_2);

        ccapi_fs_error = ccapi_fs_add_virtual_dir(virtual_path_1, local_path_1);
        CHECK_EQUAL(CCAPI_FS_ERROR_NONE, ccapi_fs_error);

        ccapi_fs_error = ccapi_fs_add_virtual_dir(virtual_path_2, local_path_2);
        CHECK_EQUAL(CCAPI_FS_ERROR_NONE, ccapi_fs_error);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};


TEST(test_ccapi_fs_mapping_stop_ccapi, testMapTwoDirsAndStop)
{
    ccapi_stop_error_t stop_error;

    Mock_ccimp_os_free_expectAndReturn(malloc_for_local_path_1, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_virtual_path_1, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry_1, CCIMP_STATUS_OK);

    Mock_ccimp_os_free_expectAndReturn(malloc_for_local_path_2, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_virtual_path_2, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry_2, CCIMP_STATUS_OK);

    Mock_ccimp_os_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_device_cloud_url, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_thread_connector_run, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}
