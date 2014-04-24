#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_fs_unmapping_no_CCAPI)
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

TEST(test_ccapi_fs_unmapping_no_CCAPI, testCcapiNotStarted)
{
    ccapi_fs_error_t error;

    error = ccapi_fs_remove_virtual_dir("my_virtual_dir");
    CHECK_EQUAL(CCAPI_FS_ERROR_CCAPI_STOPPED, error);
}

TEST_GROUP(test_ccapi_fs_unmapping)
{
    char const * local_path_1;
    char const * virtual_path_1;
    char const * local_path_2;
    char const * virtual_path_2;
    void * malloc_for_dir_list_entry_1;
    void * malloc_for_local_path_1;
    void * malloc_for_virtual_path_1;
    void * malloc_for_dir_list_entry_2;
    void * malloc_for_local_path_2;
    void * malloc_for_virtual_path_2;

    void setup()
    {

        ccapi_start_t start = {0};
        ccapi_start_error_t start_error;
        ccapi_filesystem_service_t fs_service = {NULL, NULL};
        ccapi_fs_error_t error;
        ccimp_fs_dir_open_t ccimp_dir_open_data_1, ccimp_dir_open_data_2;
        ccimp_fs_dir_close_t ccimp_dir_close_data_1, ccimp_dir_close_data_2;

        local_path_1 = "/home/user/my_directory";
        virtual_path_1 = "my_virtual_dir";
        local_path_2 = "/home/other_user/other_my_directory";
        virtual_path_2 = "other_virtual_dir";
        malloc_for_dir_list_entry_1 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        malloc_for_local_path_1 = malloc(strlen(local_path_1) + 1);
        malloc_for_virtual_path_1 = malloc(strlen(virtual_path_1) + 1);
        malloc_for_dir_list_entry_2 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        malloc_for_local_path_2 = malloc(strlen(local_path_2) + 1);
        malloc_for_virtual_path_2 = malloc(strlen(virtual_path_2) + 1);

        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.file_system = &fs_service;

        start_error = ccapi_start(&start);
        CHECK(start_error == CCAPI_START_ERROR_NONE);
        /* Simulate that imp_context was previously set by other call (file_open) */
        ccapi_data_single_instance->service.file_system.imp_context = &my_fs_context;

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_1) + 1, malloc_for_local_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_1) + 1, malloc_for_virtual_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_2) + 1, malloc_for_local_path_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_2) + 1, malloc_for_virtual_path_2);

        th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data_1, local_path_1);
        th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data_1);
        th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data_2, local_path_2);
        th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data_2);

        error = ccapi_fs_add_virtual_dir(virtual_path_1, local_path_1);
        CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);

        error = ccapi_fs_add_virtual_dir(virtual_path_2, local_path_2);
        CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_fs_unmapping, testInvalidPath)
{
    ccapi_fs_error_t error;
    char const * const invalid_virtual_path = "/my_data";

    error = ccapi_fs_remove_virtual_dir("");
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_remove_virtual_dir(NULL);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_remove_virtual_dir(invalid_virtual_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);
}

TEST(test_ccapi_fs_unmapping, testUnMapNotMapped)
{
    ccapi_fs_error_t error;

    error = ccapi_fs_remove_virtual_dir("inexisting_virtual_dir");
    CHECK_EQUAL(CCAPI_FS_ERROR_NOT_MAPPED, error);
}

TEST(test_ccapi_fs_unmapping, testUnMap_first)
{
    ccapi_fs_error_t error;

    Mock_ccimp_os_free_expectAndReturn(malloc_for_local_path_1, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_virtual_path_1, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry_1, CCIMP_STATUS_OK);

    CHECK_EQUAL(malloc_for_dir_list_entry_1, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list);
    CHECK_EQUAL(malloc_for_dir_list_entry_2, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list->next);

    error = ccapi_fs_remove_virtual_dir(virtual_path_1);

    CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    CHECK_EQUAL(malloc_for_dir_list_entry_2, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list);
    CHECK(NULL == (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list->next);
}

TEST(test_ccapi_fs_unmapping, testUnMap_second)
{
    ccapi_fs_error_t error;

    Mock_ccimp_os_free_expectAndReturn(malloc_for_local_path_2, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_virtual_path_2, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry_2, CCIMP_STATUS_OK);

    CHECK_EQUAL(malloc_for_dir_list_entry_1, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list);
    CHECK_EQUAL(malloc_for_dir_list_entry_2, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list->next);

    error = ccapi_fs_remove_virtual_dir(virtual_path_2);

    CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    CHECK_EQUAL(malloc_for_dir_list_entry_1, (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list);
    CHECK(NULL == (void *)ccapi_data_single_instance->service.file_system.virtual_dir_list->next);
}
