/*
 * mocks.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: spastor
 */


#include "mocks.h"

char * assert_buffer;
char * assert_file;

void Mock_create_all()
{
    Mock_ccimp_logging_printf_create();
    Mock_ccimp_os_malloc_create();
    Mock_ccimp_os_free_create();
    Mock_ccimp_os_create_thread_create();
    Mock_connector_init_create();
    Mock_connector_run_create();
    Mock_connector_initiate_action_create();
    Mock_ccimp_network_tcp_open_create();
    Mock_ccimp_network_tcp_send_create();
    Mock_ccimp_network_tcp_receive_create();
    Mock_ccimp_network_tcp_close_create();
    Mock_ccimp_os_get_system_time_create();
    Mock_ccimp_os_syncr_create_create();
    Mock_ccimp_os_syncr_acquire_create();
    Mock_ccimp_fs_file_open_create();
    Mock_ccimp_fs_file_read_create();
    Mock_ccimp_fs_file_write_create();
    Mock_ccimp_fs_file_seek_create();
    Mock_ccimp_fs_file_close_create();
    Mock_ccimp_fs_file_truncate_create();
    Mock_ccimp_fs_file_remove_create();
    Mock_ccimp_fs_dir_open_create();
    Mock_ccimp_fs_dir_read_entry_create();
    Mock_ccimp_fs_dir_entry_status_create();
    Mock_ccimp_fs_dir_close_create();
    Mock_ccimp_fs_hash_status_create();
    Mock_ccimp_fs_hash_file_create();
    Mock_ccimp_fs_error_desc_create();
    Mock_ccimp_fs_session_error_create();
}

void Mock_destroy_all()
{
    if (ccapi_data_single_instance != NULL)
    {
        /* Dear developer: If you want to call free on this pointer,
         * know that it leads to race conditions making TEST(ccapi_init_test, testDeviceTypeNoMemory) fail
         * once in ~100 iterations. Don't say I didn't tell you! Good luck! =)
         */
        ccapi_data_single_instance = NULL;
    }
    Mock_ccimp_os_create_thread_destroy();
    Mock_ccimp_logging_printf_destroy();
    Mock_ccimp_os_malloc_destroy();
    Mock_ccimp_os_free_destroy();
    Mock_ccimp_os_create_thread_destroy();
    Mock_ccimp_os_syncr_create_destroy();
    Mock_ccimp_os_syncr_acquire_destroy();
    Mock_connector_init_destroy();
    Mock_connector_run_destroy();
    Mock_connector_initiate_action_destroy();
    Mock_ccimp_network_tcp_open_destroy();
    Mock_ccimp_network_tcp_send_destroy();
    Mock_ccimp_network_tcp_receive_destroy();
    Mock_ccimp_network_tcp_close_destroy();
    Mock_ccimp_os_get_system_time_destroy();
    Mock_ccimp_fs_file_open_destroy();
    Mock_ccimp_fs_file_read_destroy();
    Mock_ccimp_fs_file_write_destroy();
    Mock_ccimp_fs_file_seek_destroy();
    Mock_ccimp_fs_file_close_destroy();
    Mock_ccimp_fs_file_truncate_destroy();
    Mock_ccimp_fs_file_remove_destroy();
    Mock_ccimp_fs_dir_open_destroy();
    Mock_ccimp_fs_dir_read_entry_destroy();
    Mock_ccimp_fs_dir_entry_status_destroy();
    Mock_ccimp_fs_dir_close_destroy();
    Mock_ccimp_fs_hash_status_destroy();
    Mock_ccimp_fs_hash_file_destroy();
    Mock_ccimp_fs_error_desc_destroy();
    Mock_ccimp_fs_session_error_destroy();
    ASSERT_CLEAN();
    mock().removeAllComparators();
    mock().checkExpectations();
    mock().clear();
}
