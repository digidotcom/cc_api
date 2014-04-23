#include "test_helper_functions.h"

static char const * ccapi_fs_access_expected_path = NULL;
static ccapi_fs_request_t ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_UNKNOWN;
static ccapi_fs_access_t ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
static ccapi_bool_t ccapi_fs_access_cb_called = CCAPI_FALSE;

static ccapi_fs_access_t ccapi_fs_access_cb(char const * const local_path, ccapi_fs_request_t const request)
{
    STRCMP_EQUAL(ccapi_fs_access_expected_path, local_path);
    CHECK_EQUAL(ccapi_fs_access_expected_request, request);
    ccapi_fs_access_cb_called = CCAPI_TRUE;
    return ccapi_fs_access_retval;
}

TEST_GROUP(test_ccapi_fs_access)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_filesystem_service_t fs_service = {ccapi_fs_access_cb, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.file_system = &fs_service;

        ccapi_fs_access_expected_path = NULL;
        ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_READ;
        ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
        ccapi_fs_access_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(fs_service.access_cb, ccapi_data_single_instance->service.file_system.user_callbacks.access_cb);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_fs_access, testAccessRead)
{
    connector_request_id_t request;
    ccimp_fs_file_open_t ccimp_open_data;
    connector_file_system_open_t ccfsm_open_data;
    connector_callback_status_t status;

    ccapi_fs_access_expected_path = "/tmp/hello.txt";
    ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_READ;
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_ALLOW;

    ccimp_open_data.errnum.pointer = NULL;
    ccimp_open_data.imp_context = NULL;
    ccimp_open_data.handle.pointer = NULL;
    ccimp_open_data.flags = CCIMP_FILE_O_RDONLY | CCIMP_FILE_O_APPEND;
    ccimp_open_data.path = ccapi_fs_access_expected_path;

    ccfsm_open_data.errnum = NULL;
    ccfsm_open_data.handle = NULL;
    ccfsm_open_data.oflag = CONNECTOR_FILE_O_RDONLY | CONNECTOR_FILE_O_APPEND;
    ccfsm_open_data.path = ccimp_open_data.path;
    ccfsm_open_data.user_context = NULL;

    Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_open;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(ccfsm_open_data.handle != NULL);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);

    /* Check access denied, no call to mocks */
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_error, status);
    CHECK(ccfsm_open_data.handle == NULL);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);
}

TEST(test_ccapi_fs_access, testAccessWrite)
{
    connector_request_id_t request;
    ccimp_fs_file_open_t ccimp_open_data;
    connector_file_system_open_t ccfsm_open_data;
    connector_callback_status_t status;

    ccapi_fs_access_expected_path = "/tmp/hello.txt";
    ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_WRITE;
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_ALLOW;

    ccimp_open_data.errnum.pointer = NULL;
    ccimp_open_data.imp_context = NULL;
    ccimp_open_data.handle.pointer = NULL;
    ccimp_open_data.flags = CCIMP_FILE_O_WRONLY | CCIMP_FILE_O_APPEND;
    ccimp_open_data.path = ccapi_fs_access_expected_path;

    ccfsm_open_data.errnum = NULL;
    ccfsm_open_data.handle = NULL;
    ccfsm_open_data.oflag = CONNECTOR_FILE_O_WRONLY | CONNECTOR_FILE_O_APPEND;
    ccfsm_open_data.path = ccimp_open_data.path;
    ccfsm_open_data.user_context = NULL;

    Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_open;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(ccfsm_open_data.handle != NULL);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);

    /* Check access denied, no call to mocks */
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_error, status);
    CHECK(ccfsm_open_data.handle == NULL);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);
}

TEST(test_ccapi_fs_access, testAccessReadWrite)
{
    connector_request_id_t request;
    ccimp_fs_file_open_t ccimp_open_data;
    connector_file_system_open_t ccfsm_open_data;
    connector_callback_status_t status;

    ccapi_fs_access_expected_path = "/tmp/hello.txt";
    ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_READWRITE;
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_ALLOW;

    ccimp_open_data.errnum.pointer = NULL;
    ccimp_open_data.imp_context = NULL;
    ccimp_open_data.handle.pointer = NULL;
    ccimp_open_data.flags = CCIMP_FILE_O_RDWR | CCIMP_FILE_O_APPEND;
    ccimp_open_data.path = ccapi_fs_access_expected_path;

    ccfsm_open_data.errnum = NULL;
    ccfsm_open_data.handle = NULL;
    ccfsm_open_data.oflag = CONNECTOR_FILE_O_RDWR | CONNECTOR_FILE_O_APPEND;
    ccfsm_open_data.path = ccimp_open_data.path;
    ccfsm_open_data.user_context = NULL;

    Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_open;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(ccfsm_open_data.handle != NULL);

    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);

    /* Check access denied, no call to mocks */
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_error, status);
    CHECK(ccfsm_open_data.handle == NULL);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);
}

TEST(test_ccapi_fs_access, testAccessRemove)
{
    connector_request_id_t request;
    ccimp_fs_file_remove_t ccimp_remove_data;
    connector_file_system_remove_t ccfsm_remove_data;
    connector_callback_status_t status;
    int fs_context;

    /* Simulate that imp_context was previously set by other call (file_open) */
    ccapi_data_single_instance->service.file_system.imp_context = &fs_context;

    ccapi_fs_access_expected_path = "/tmp/hello.txt";
    ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_REMOVE;
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_ALLOW;

    ccimp_remove_data.errnum.pointer = NULL;
    ccimp_remove_data.imp_context = &fs_context;
    ccimp_remove_data.path = ccapi_fs_access_expected_path;

    ccfsm_remove_data.errnum = ccimp_remove_data.errnum.pointer;
    ccfsm_remove_data.user_context = ccimp_remove_data.imp_context;
    ccfsm_remove_data.path = ccimp_remove_data.path;

    Mock_ccimp_fs_file_remove_expectAndReturn(&ccimp_remove_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_remove;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_remove_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);

    /* Check access denied, no call to mocks */
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_remove_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_error, status);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);
}

TEST(test_ccapi_fs_access, testAccessList)
{
    connector_request_id_t request;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    connector_file_system_opendir_t ccfsm_dir_open_data;
    connector_callback_status_t status;
    int fs_context;

    /* Simulate that imp_context was previously set by other call (file_open) */
    ccapi_data_single_instance->service.file_system.imp_context = &fs_context;

    ccapi_fs_access_expected_path = "/tmp/";
    ccapi_fs_access_expected_request = CCAPI_FS_REQUEST_LIST;
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_ALLOW;

    ccimp_dir_open_data.errnum.pointer = NULL;
    ccimp_dir_open_data.imp_context = &fs_context;
    ccimp_dir_open_data.handle.pointer = NULL;
    ccimp_dir_open_data.path = ccapi_fs_access_expected_path;

    ccfsm_dir_open_data.errnum = ccimp_dir_open_data.errnum.pointer;
    ccfsm_dir_open_data.user_context = ccimp_dir_open_data.imp_context;
    ccfsm_dir_open_data.handle = ccimp_dir_open_data.handle.pointer;
    ccfsm_dir_open_data.path = ccimp_dir_open_data.path;

    Mock_ccimp_fs_dir_open_expectAndReturn(&ccimp_dir_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_opendir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
    CHECK(NULL != ccfsm_dir_open_data.handle);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);

    /* Check access denied, no call to mocks */
    ccapi_fs_access_retval = CCAPI_FS_ACCESS_DENY;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_error, status);
    CHECK(ccfsm_dir_open_data.handle == NULL);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_fs_access_cb_called);
}
