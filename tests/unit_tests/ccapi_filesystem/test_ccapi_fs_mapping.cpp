#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_fs_mapping_no_CCAPI)
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

TEST(test_ccapi_fs_mapping_no_CCAPI, testCcapiNotStarted)
{
    ccapi_fs_error_t error;
    char const * const dir_path = "/home/user/my_directory";

    error = ccapi_fs_add_virtual_dir("my_virtual_dir", dir_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_CCAPI_STOPPED, error);
}

TEST_GROUP(test_ccapi_fs_mapping)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_filesystem_service_t fs_service = {NULL, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.file_system = &fs_service;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(fs_service.changed_cb, ccapi_data_single_instance->service.file_system.user_callbacks.changed_cb);
        CHECK_EQUAL(fs_service.access_cb, ccapi_data_single_instance->service.file_system.user_callbacks.access_cb);
        /* Simulate that imp_context was previously set by other call (file_open) */
        ccapi_data_single_instance->service.file_system.imp_context = &my_fs_context;
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_fs_mapping, testInvalidPath)
{
    ccapi_fs_error_t error;
    char const * const local_path = "/home/user";
    char const * const virtual_path = "my_data";
    char const * const invalid_virtual_path = "/my_data";

    error = ccapi_fs_add_virtual_dir("", local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_add_virtual_dir(NULL, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_add_virtual_dir(virtual_path, "");
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_add_virtual_dir(virtual_path, NULL);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_add_virtual_dir(invalid_virtual_path, NULL);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);

    error = ccapi_fs_add_virtual_dir(invalid_virtual_path, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INVALID_PATH, error);
}

TEST(test_ccapi_fs_mapping, testMapAFile)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data;

    ccimp_dir_open_data.errnum.pointer = NULL;
    ccimp_dir_open_data.imp_context = &my_fs_context;
    ccimp_dir_open_data.handle.pointer = NULL;
    ccimp_dir_open_data.path = "/notadir.txt";

    Mock_ccimp_fs_dir_open_expectAndReturn(&ccimp_dir_open_data, CCIMP_STATUS_ERROR);

    error = ccapi_fs_add_virtual_dir("my_virtual_dir", "/notadir.txt");
    CHECK_EQUAL(CCAPI_FS_ERROR_NOT_A_DIR, error);
}

TEST(test_ccapi_fs_mapping, testMapDirNoMemory4DirEntry)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    ccimp_fs_dir_close_t ccimp_dir_close_data;
    char const * const local_path = "/home/user/my_directory";
    char const * const virtual_path = "my_virtual_dir";
    /* Test Setup */
    {
        void * malloc_for_dir_list_entry = NULL;
        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry);
    }
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data, local_path);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data);
    /* Test */
    error = ccapi_fs_add_virtual_dir(virtual_path, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INSUFFICIENT_MEMORY, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list == NULL);
}

TEST(test_ccapi_fs_mapping, testMapDirNoMemory4ActualPath)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    ccimp_fs_dir_close_t ccimp_dir_close_data;
    char const * const local_path = "/home/user/my_directory";
    char const * const virtual_path = "my_virtual_dir";
    /* Test Setup */
    {
        void * malloc_for_dir_list_entry = malloc(sizeof (ccapi_fs_virtual_dir_t));
        void * malloc_for_local_path = NULL;

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path) + 1, malloc_for_local_path);
        Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry, CCIMP_STATUS_OK);
    }
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data, local_path);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data);
    /* Test */
    error = ccapi_fs_add_virtual_dir(virtual_path, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INSUFFICIENT_MEMORY, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list == NULL);
}

TEST(test_ccapi_fs_mapping, testMapDirNoMemory4VirtualPath)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    ccimp_fs_dir_close_t ccimp_dir_close_data;
    char const * const local_path = "/home/user/my_directory";
    char const * const virtual_path = "my_virtual_dir";
    /* Test Setup */
    {
        void * malloc_for_dir_list_entry = malloc(sizeof (ccapi_fs_virtual_dir_t));
        void * malloc_for_local_path = malloc(strlen(local_path) + 1);
        void * malloc_for_virtual_path = NULL;

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path) + 1, malloc_for_local_path);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path) + 1, malloc_for_virtual_path);
        Mock_ccimp_os_free_expectAndReturn(malloc_for_dir_list_entry, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(malloc_for_local_path, CCIMP_STATUS_OK);
    }
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data, local_path);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data);
    /* Test */
    error = ccapi_fs_add_virtual_dir(virtual_path, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_INSUFFICIENT_MEMORY, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list == NULL);
}

TEST(test_ccapi_fs_mapping, testMapDirOK)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    ccimp_fs_dir_close_t ccimp_dir_close_data;
    char const * const local_path = "/home/user/my_directory";
    char const * const virtual_path = "my_virtual_dir";
    /* Test Setup */
    {
        void * malloc_for_dir_list_entry = malloc(sizeof (ccapi_fs_virtual_dir_t));
        void * malloc_for_local_path = malloc(strlen(local_path) + 1);
        void * malloc_for_virtual_path = malloc(strlen(virtual_path) + 1);

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path) + 1, malloc_for_local_path);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path) + 1, malloc_for_virtual_path);
    }
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data, local_path);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data);
    /* Test */
    error = ccapi_fs_add_virtual_dir(virtual_path, local_path);
    CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list != NULL);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list->next == NULL);
    STRCMP_EQUAL(virtual_path, ccapi_data_single_instance->service.file_system.virtual_dir_list->virtual_dir);
    STRCMP_EQUAL(local_path, ccapi_data_single_instance->service.file_system.virtual_dir_list->local_path);
}

TEST(test_ccapi_fs_mapping, testMapTwoDirs)
{
    ccapi_fs_error_t error;
    ccimp_fs_dir_open_t ccimp_dir_open_data_1, ccimp_dir_open_data_2;
    ccimp_fs_dir_close_t ccimp_dir_close_data_1, ccimp_dir_close_data_2;
    char const * const local_path_1 = "/home/user/my_directory";
    char const * const virtual_path_1 = "my_virtual_dir";
    char const * const local_path_2 = "/home/other_user/other_my_directory";
    char const * const virtual_path_2 = "other_virtual_dir";
    /* Test Setup */
    {
        void * malloc_for_dir_list_entry_1 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        void * malloc_for_local_path_1 = malloc(strlen(local_path_1) + 1);
        void * malloc_for_virtual_path_1 = malloc(strlen(virtual_path_1) + 1);
        void * malloc_for_dir_list_entry_2 = malloc(sizeof (ccapi_fs_virtual_dir_t));
        void * malloc_for_local_path_2 = malloc(strlen(local_path_2) + 1);
        void * malloc_for_virtual_path_2 = malloc(strlen(virtual_path_2) + 1);

        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_1) + 1, malloc_for_local_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_1) + 1, malloc_for_virtual_path_1);
        Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_fs_virtual_dir_t), malloc_for_dir_list_entry_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(local_path_2) + 1, malloc_for_local_path_2);
        Mock_ccimp_os_malloc_expectAndReturn(strlen(virtual_path_2) + 1, malloc_for_virtual_path_2);
    }
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data_1, local_path_1);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data_1);
    th_filesystem_prepare_ccimp_dir_open_data_call(&ccimp_dir_open_data_2, local_path_2);
    th_filesystem_prepare_ccimp_dir_close_call(&ccimp_dir_close_data_2);
    /* Test */
    error = ccapi_fs_add_virtual_dir(virtual_path_1, local_path_1);
    CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list != NULL);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list->next == NULL);
    STRCMP_EQUAL(virtual_path_1, ccapi_data_single_instance->service.file_system.virtual_dir_list->virtual_dir);
    STRCMP_EQUAL(local_path_1, ccapi_data_single_instance->service.file_system.virtual_dir_list->local_path);

    error = ccapi_fs_add_virtual_dir(virtual_path_2, local_path_2);
    CHECK_EQUAL(CCAPI_FS_ERROR_NONE, error);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list != NULL);
    CHECK(ccapi_data_single_instance->service.file_system.virtual_dir_list->next != NULL);
    STRCMP_EQUAL(virtual_path_2, ccapi_data_single_instance->service.file_system.virtual_dir_list->virtual_dir);
    STRCMP_EQUAL(local_path_2, ccapi_data_single_instance->service.file_system.virtual_dir_list->local_path);
    STRCMP_EQUAL(virtual_path_1, ccapi_data_single_instance->service.file_system.virtual_dir_list->next->virtual_dir);
    STRCMP_EQUAL(local_path_1, ccapi_data_single_instance->service.file_system.virtual_dir_list->next->local_path);
}
