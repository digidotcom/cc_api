#include "test_helper_functions.h"

TEST_GROUP(test_ccfsm_filesystem)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_filesystem_service_t fs_service = &fs_service; /* Not NULL */
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.file_system = &fs_service;
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccfsm_filesystem, testFileOpen)
{
    connector_request_id_t request;
    ccimp_fs_file_open_t ccimp_open_data;
    connector_file_system_open_t ccfsm_open_data;
    connector_callback_status_t status;

    ccimp_open_data.errnum.pointer = NULL;
    ccimp_open_data.imp_context = NULL;
    ccimp_open_data.handle.pointer = NULL;
    ccimp_open_data.flags = CCIMP_FILE_O_RDWR | CCIMP_FILE_O_APPEND;
    ccimp_open_data.path = "/tmp/hello.txt";

    ccfsm_open_data.errnum = NULL;
    ccfsm_open_data.handle = NULL;
    ccfsm_open_data.oflag = CONNECTOR_FILE_O_RDWR | CONNECTOR_FILE_O_APPEND;
    ccfsm_open_data.path = ccimp_open_data.path;
    ccfsm_open_data.user_context = NULL;

    Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_open;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testFileRead)
{
    connector_request_id_t request;
    ccimp_fs_file_read_t ccimp_read_data;
    connector_file_system_read_t ccfsm_read_data;
    connector_callback_status_t status;
    uint8_t buffer[] = {'A'};
    ccimp_read_data.errnum.pointer = NULL;
    ccimp_read_data.imp_context = NULL;
    ccimp_read_data.handle.pointer = NULL;
    ccimp_read_data.buffer = buffer;
    ccimp_read_data.bytes_available = 128;
    ccimp_read_data.bytes_used = 0;

    ccfsm_read_data.errnum = NULL;
    ccfsm_read_data.user_context = NULL;
    ccfsm_read_data.handle = NULL;
    ccfsm_read_data.buffer = buffer;
    ccfsm_read_data.bytes_available = 128;
    ccfsm_read_data.bytes_used = 0;

    Mock_ccimp_fs_file_read_expectAndReturn(&ccimp_read_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_read;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_read_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testFileWrite)
{
    connector_request_id_t request;
    ccimp_fs_file_write_t ccimp_write_data;
    connector_file_system_write_t ccfsm_write_data;
    connector_callback_status_t status;
    uint8_t buffer[] = {'A'};
    ccimp_write_data.errnum.pointer = NULL;
    ccimp_write_data.imp_context = NULL;
    ccimp_write_data.handle.pointer = NULL;
    ccimp_write_data.buffer = buffer;
    ccimp_write_data.bytes_available = 128;
    ccimp_write_data.bytes_used = 0;

    ccfsm_write_data.errnum = NULL;
    ccfsm_write_data.user_context = NULL;
    ccfsm_write_data.handle = NULL;
    ccfsm_write_data.buffer = buffer;
    ccfsm_write_data.bytes_available = 128;
    ccfsm_write_data.bytes_used = 0;

    Mock_ccimp_fs_file_write_expectAndReturn(&ccimp_write_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_write;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_write_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testFileSeek)
{
    connector_request_id_t request;
    ccimp_fs_file_seek_t ccimp_seek_data;
    connector_file_system_lseek_t ccfsm_seek_data;
    connector_callback_status_t status;

    ccimp_seek_data.errnum.pointer = NULL;
    ccimp_seek_data.imp_context = NULL;
    ccimp_seek_data.handle.pointer = NULL;
    ccimp_seek_data.origin = CCIMP_SEEK_END;
    ccimp_seek_data.requested_offset = -128;
    ccimp_seek_data.resulting_offset = 0;

    ccfsm_seek_data.errnum = NULL;
    ccfsm_seek_data.user_context = NULL;
    ccfsm_seek_data.handle = NULL;
    ccfsm_seek_data.origin = connector_file_system_seek_end;
    ccfsm_seek_data.requested_offset = -128;
    ccfsm_seek_data.resulting_offset = 0;

    Mock_ccimp_fs_file_seek_expectAndReturn(&ccimp_seek_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_lseek;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_seek_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testFileClose)
{
    connector_request_id_t request;
    ccimp_fs_file_close_t ccimp_close_data;
    connector_file_system_close_t ccfsm_close_data;
    connector_callback_status_t status;

    ccimp_close_data.errnum.pointer = NULL;
    ccimp_close_data.imp_context = NULL;
    ccimp_close_data.handle.pointer = NULL;

    ccfsm_close_data.errnum = NULL;
    ccfsm_close_data.user_context = NULL;
    ccfsm_close_data.handle = NULL;

    Mock_ccimp_fs_file_close_expectAndReturn(&ccimp_close_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_close;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testFileRemove)
{
    connector_request_id_t request;
    ccimp_fs_file_remove_t ccimp_remove_data;
    connector_file_system_remove_t ccfsm_remove_data;
    connector_callback_status_t status;

    ccimp_remove_data.errnum.pointer = NULL;
    ccimp_remove_data.imp_context = NULL;
    ccimp_remove_data.path = "/tmp/hello.txt";

    ccfsm_remove_data.errnum = NULL;
    ccfsm_remove_data.user_context = NULL;
    ccfsm_remove_data.path = ccimp_remove_data.path;

    Mock_ccimp_fs_file_remove_expectAndReturn(&ccimp_remove_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_remove;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_remove_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testDirOpen)
{
    connector_request_id_t request;
    ccimp_fs_dir_open_t ccimp_dir_open_data;
    connector_file_system_opendir_t ccfsm_dir_open_data;
    connector_callback_status_t status;

    ccimp_dir_open_data.errnum.pointer = NULL;
    ccimp_dir_open_data.imp_context = NULL;
    ccimp_dir_open_data.handle.pointer = NULL;
    ccimp_dir_open_data.path = "/tmp/";

    ccfsm_dir_open_data.errnum = ccimp_dir_open_data.errnum.pointer;
    ccfsm_dir_open_data.user_context = ccimp_dir_open_data.imp_context;
    ccfsm_dir_open_data.handle = ccimp_dir_open_data.handle.pointer;
    ccfsm_dir_open_data.path = ccimp_dir_open_data.path;

    Mock_ccimp_fs_dir_open_expectAndReturn(&ccimp_dir_open_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_opendir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_open_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testDirRead)
{
    connector_request_id_t request;
    ccimp_fs_dir_read_entry_t ccimp_dir_read_entry_data;
    connector_file_system_readdir_t ccfsm_dir_read_entry_data;
    connector_callback_status_t status;
    void * handle = &handle; /* Not NULL */
    char entry_name[256] = {0};

    ccimp_dir_read_entry_data.errnum.pointer = NULL;
    ccimp_dir_read_entry_data.imp_context = NULL;
    ccimp_dir_read_entry_data.handle.pointer = handle;
    ccimp_dir_read_entry_data.entry_name = entry_name;
    ccimp_dir_read_entry_data.bytes_available = sizeof entry_name;

    ccfsm_dir_read_entry_data.errnum = ccimp_dir_read_entry_data.errnum.pointer;
    ccfsm_dir_read_entry_data.user_context = ccimp_dir_read_entry_data.imp_context;
    ccfsm_dir_read_entry_data.handle = ccimp_dir_read_entry_data.handle.pointer;
    ccfsm_dir_read_entry_data.entry_name = ccimp_dir_read_entry_data.entry_name;
    ccfsm_dir_read_entry_data.bytes_available = ccimp_dir_read_entry_data.bytes_available;

    Mock_ccimp_fs_dir_read_entry_expectAndReturn(&ccimp_dir_read_entry_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_readdir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_read_entry_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testDirEntryStat)
{
    connector_request_id_t request;
    ccimp_fs_dir_entry_status_t ccimp_dir_entry_status_data;
    connector_file_system_stat_dir_entry_t ccfsm_dir_entry_status_data;
    connector_callback_status_t status;

    ccimp_dir_entry_status_data.errnum.pointer = NULL;
    ccimp_dir_entry_status_data.imp_context = NULL;
    ccimp_dir_entry_status_data.path = "/tmp/hello.txt";
    ccimp_dir_entry_status_data.status.file_size = 0;
    ccimp_dir_entry_status_data.status.last_modified = 0;
    ccimp_dir_entry_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

    ccfsm_dir_entry_status_data.errnum = ccimp_dir_entry_status_data.errnum.pointer;
    ccfsm_dir_entry_status_data.user_context = ccimp_dir_entry_status_data.imp_context;
    ccfsm_dir_entry_status_data.path = ccimp_dir_entry_status_data.path;
    ccfsm_dir_entry_status_data.statbuf.file_size = ccimp_dir_entry_status_data.status.file_size;
    ccfsm_dir_entry_status_data.statbuf.last_modified = ccimp_dir_entry_status_data.status.last_modified;
    ccfsm_dir_entry_status_data.statbuf.flags = connector_file_system_file_type_none;

    Mock_ccimp_fs_dir_entry_status_expectAndReturn(&ccimp_dir_entry_status_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_stat_dir_entry;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_entry_status_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testDirClose)
{
    connector_request_id_t request;
    ccimp_fs_dir_close_t ccimp_dir_close_data;
    connector_file_system_close_t ccfsm_dir_close_data;
    connector_callback_status_t status;
    void * handle = &handle; /* Not NULL */

    ccimp_dir_close_data.errnum.pointer = NULL;
    ccimp_dir_close_data.imp_context = NULL;
    ccimp_dir_close_data.handle.pointer = handle;

    ccfsm_dir_close_data.errnum = ccimp_dir_close_data.errnum.pointer;
    ccfsm_dir_close_data.user_context = ccimp_dir_close_data.imp_context;
    ccfsm_dir_close_data.handle = ccimp_dir_close_data.handle.pointer;

    Mock_ccimp_fs_dir_close_expectAndReturn(&ccimp_dir_close_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_closedir;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_dir_close_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testHashStatus)
{
    connector_request_id_t request;
    ccimp_fs_hash_status_t ccimp_fs_hash_status_data;
    connector_file_system_stat_t ccfsm_file_stat_data;
    connector_callback_status_t status;

    ccimp_fs_hash_status_data.errnum.pointer = NULL;
    ccimp_fs_hash_status_data.imp_context = NULL;
    ccimp_fs_hash_status_data.path = "/tmp/hello.txt";
    ccimp_fs_hash_status_data.hash_alg.actual = CCIMP_FS_HASH_NONE;
    ccimp_fs_hash_status_data.hash_alg.requested = CCIMP_FS_HASH_CRC32;
    ccimp_fs_hash_status_data.status.file_size = 0;
    ccimp_fs_hash_status_data.status.last_modified = 0;
    ccimp_fs_hash_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

    ccfsm_file_stat_data.errnum = ccimp_fs_hash_status_data.errnum.pointer;
    ccfsm_file_stat_data.user_context = ccimp_fs_hash_status_data.imp_context;
    ccfsm_file_stat_data.path = ccimp_fs_hash_status_data.path;
    ccfsm_file_stat_data.hash_algorithm.actual = connector_file_system_hash_none;
    ccfsm_file_stat_data.hash_algorithm.requested = connector_file_system_hash_crc32;
    ccfsm_file_stat_data.statbuf.file_size = 0;
    ccfsm_file_stat_data.statbuf.last_modified = 0;
    ccfsm_file_stat_data.statbuf.flags = connector_file_system_file_type_none;

    Mock_ccimp_fs_hash_status_expectAndReturn(&ccimp_fs_hash_status_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_stat;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_file_stat_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testHashFile)
{
    connector_request_id_t request;
    ccimp_fs_hash_file_t ccimp_fs_hash_file_data;
    connector_file_system_hash_t ccfsm_hash_file_data;
    connector_callback_status_t status;
    uint32_t hash_value = 0;

    ccimp_fs_hash_file_data.errnum.pointer = NULL;
    ccimp_fs_hash_file_data.imp_context = NULL;
    ccimp_fs_hash_file_data.path = "/tmp/hello.txt";
    ccimp_fs_hash_file_data.hash_algorithm = CCIMP_FS_HASH_CRC32;
    ccimp_fs_hash_file_data.hash_value = &hash_value;
    ccimp_fs_hash_file_data.bytes_requested = sizeof hash_value;

    ccfsm_hash_file_data.errnum = ccimp_fs_hash_file_data.errnum.pointer;
    ccfsm_hash_file_data.user_context = ccimp_fs_hash_file_data.imp_context;
    ccfsm_hash_file_data.path = ccimp_fs_hash_file_data.path;
    ccfsm_hash_file_data.hash_algorithm = connector_file_system_hash_crc32;
    ccfsm_hash_file_data.hash_value = ccimp_fs_hash_file_data.hash_value;
    ccfsm_hash_file_data.bytes_requested = ccimp_fs_hash_file_data.bytes_requested;

    Mock_ccimp_fs_hash_file_expectAndReturn(&ccimp_fs_hash_file_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_hash;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_hash_file_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testErrorDesc)
{
    connector_request_id_t request;
    ccimp_fs_error_desc_t ccimp_error_desc_data;
    connector_file_system_get_error_t ccfsm_error_desc_data;
    connector_callback_status_t status;
    uint8_t buffer[256] = {0};

    ccimp_error_desc_data.errnum.pointer = ccfsm_error_desc_data.errnum;
    ccimp_error_desc_data.imp_context = NULL;
    ccimp_error_desc_data.buffer = buffer;
    ccimp_error_desc_data.bytes_available = sizeof buffer;
    ccimp_error_desc_data.bytes_used = 0;
    ccimp_error_desc_data.error_status = CCIMP_FS_ERROR_UNKNOWN;

    ccfsm_error_desc_data.errnum = ccimp_error_desc_data.errnum.pointer;
    ccfsm_error_desc_data.user_context = ccimp_error_desc_data.imp_context;
    ccfsm_error_desc_data.buffer = ccimp_error_desc_data.buffer;
    ccfsm_error_desc_data.bytes_available = ccimp_error_desc_data.bytes_available;
    ccfsm_error_desc_data.bytes_used = ccimp_error_desc_data.bytes_used;
    ccfsm_error_desc_data.error_status = connector_file_system_unspec_error;

    Mock_ccimp_fs_error_desc_expectAndReturn(&ccimp_error_desc_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_get_error;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_error_desc_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}

TEST(test_ccfsm_filesystem, testSessionError)
{
    connector_request_id_t request;
    ccimp_fs_session_error_t ccimp_session_error_data;
    connector_file_system_session_error_t ccfsm_session_error_data;
    connector_callback_status_t status;

    ccimp_session_error_data.imp_context = NULL;
    ccimp_session_error_data.session_error = CCIMP_FS_SESSION_ERROR_TIMEOUT;

    ccfsm_session_error_data.user_context = ccimp_session_error_data.imp_context;
    ccfsm_session_error_data.session_error = connector_session_error_timeout;

    Mock_ccimp_fs_session_error_expectAndReturn(&ccimp_session_error_data, CCIMP_STATUS_OK);

    request.file_system_request = connector_request_id_file_system_session_error;
    status = ccapi_connector_callback(connector_class_id_file_system, request, &ccfsm_session_error_data, ccapi_data_single_instance);

    CHECK_EQUAL(connector_callback_continue, status);
}
