#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_fs_virtual_drive_map_to_ccfsm)
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

typedef struct {
    ccapi_fs_virtual_dir_t * dir_entry;
} ccapi_fs_virtual_rootdir_listing_handle_t;

TEST(test_ccapi_fs_virtual_drive_map_to_ccfsm, testRootDirList)
{
#define ROOT_PATH   "/"
    connector_request_id_t request;
    connector_file_system_stat_t ccfsm_filesystem_stat_data;
    connector_file_system_opendir_t ccfsm_dir_open_data;
    connector_file_system_readdir_t ccfsm_dir_read_entry_data;
    connector_file_system_stat_dir_entry_t ccfsm_dir_entry_status_data;
    connector_file_system_close_t ccfsm_dir_close_data;
    connector_callback_status_t status;
    ccapi_fs_virtual_rootdir_listing_handle_t * root_dir_listing_handle = NULL;
    static char buffer[250] = {0};
    static char aux_buffer[250] = {0};

    /* Stat root path */

    ccfsm_filesystem_stat_data.user_context = NULL;
    ccfsm_filesystem_stat_data.path = ROOT_PATH;
    ccfsm_filesystem_stat_data.hash_algorithm.requested = connector_file_system_hash_none;

    request.file_system_request = connector_request_id_file_system_stat;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_filesystem_stat_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(ccfsm_filesystem_stat_data.user_context != NULL);
    CHECK_EQUAL(0, ccfsm_filesystem_stat_data.statbuf.file_size);
    CHECK_EQUAL(0, ccfsm_filesystem_stat_data.statbuf.last_modified);
    CHECK_EQUAL(connector_file_system_file_type_is_dir, ccfsm_filesystem_stat_data.statbuf.flags);
    CHECK_EQUAL(connector_file_system_hash_none, ccfsm_filesystem_stat_data.hash_algorithm.actual);

    /* Open root path */
    ccfsm_dir_open_data.path = ROOT_PATH;
    ccfsm_dir_open_data.handle = NULL;
    ccfsm_dir_open_data.user_context = ccfsm_filesystem_stat_data.user_context;

    request.file_system_request = connector_request_id_file_system_opendir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_open_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_open_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(ccapi_data_single_instance->service.file_system.virtual_dir_list, root_dir_listing_handle->dir_entry);
    CHECK(NULL != ccfsm_dir_open_data.handle);

    /* Read first entry and check that it's the second mapped path */
    ccfsm_dir_read_entry_data.handle = ccfsm_dir_open_data.handle;
    ccfsm_dir_read_entry_data.user_context = ccfsm_filesystem_stat_data.user_context;
    ccfsm_dir_read_entry_data.entry_name = buffer;
    ccfsm_dir_read_entry_data.bytes_available = sizeof buffer;

    request.file_system_request = connector_request_id_file_system_readdir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_read_entry_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_read_entry_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    STRCMP_EQUAL(virtual_path_2, ccfsm_dir_read_entry_data.entry_name);
    CHECK_EQUAL(ccapi_data_single_instance->service.file_system.virtual_dir_list->next, root_dir_listing_handle->dir_entry);

    /* Stat first entry and check that it's listed as dir */
    strcpy(aux_buffer, ROOT_PATH);
    strcat(aux_buffer, ccfsm_dir_read_entry_data.entry_name);
    ccfsm_dir_entry_status_data.path = aux_buffer;
    ccfsm_dir_entry_status_data.user_context = ccfsm_filesystem_stat_data.user_context;

    request.file_system_request = connector_request_id_file_system_stat_dir_entry;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_entry_status_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_entry_status_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(connector_file_system_file_type_is_dir, ccfsm_dir_entry_status_data.statbuf.flags);
    CHECK_EQUAL(0, ccfsm_dir_entry_status_data.statbuf.file_size);
    CHECK_EQUAL(0, ccfsm_dir_entry_status_data.statbuf.last_modified);

    /* Read second entry and check that it's the first mapped path */
    request.file_system_request = connector_request_id_file_system_readdir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_read_entry_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_read_entry_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    STRCMP_EQUAL(virtual_path_1, ccfsm_dir_read_entry_data.entry_name);
    CHECK(NULL == root_dir_listing_handle->dir_entry);

    /* Stat second entry and check that it's listed as dir */
    strcpy(aux_buffer, ROOT_PATH);
    strcat(aux_buffer, ccfsm_dir_read_entry_data.entry_name);
    ccfsm_dir_entry_status_data.path = aux_buffer;
    ccfsm_dir_entry_status_data.user_context = ccfsm_filesystem_stat_data.user_context;

    request.file_system_request = connector_request_id_file_system_stat_dir_entry;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_entry_status_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_entry_status_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(connector_file_system_file_type_is_dir, ccfsm_dir_entry_status_data.statbuf.flags);
    CHECK_EQUAL(0, ccfsm_dir_entry_status_data.statbuf.file_size);
    CHECK_EQUAL(0, ccfsm_dir_entry_status_data.statbuf.last_modified);

    /* Read last entry and check that it's an empty string */
    request.file_system_request = connector_request_id_file_system_readdir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_read_entry_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_read_entry_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    STRCMP_EQUAL("", ccfsm_dir_read_entry_data.entry_name);
    CHECK(NULL == root_dir_listing_handle->dir_entry);

    /* Close directory */
    ccfsm_dir_close_data.user_context = ccfsm_filesystem_stat_data.user_context;
    request.file_system_request = connector_request_id_file_system_closedir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_close_data, ccapi_data_single_instance);
    root_dir_listing_handle = (ccapi_fs_virtual_rootdir_listing_handle_t *)ccfsm_dir_close_data.user_context;

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(NULL == ccfsm_dir_close_data.user_context);
}
