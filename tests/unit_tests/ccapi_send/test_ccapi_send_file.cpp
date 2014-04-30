#include "test_helper_functions.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#define CLOUD_PATH   "test/test.txt"
#define CONTENT_TYPE "text/plain"
#define DATA         "CCAPI send data sample\n"
#define LOCAL_PATH   "./send_file.txt"

void create_test_file(char const * const path, void const * const data, size_t bytes)
{
    int fd, result;

    fd = open(path, O_CREAT | O_RDWR, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); /* 0664 = Owner RW + Group RW + Others R */
    CHECK (fd >= 0);

    result = write(fd, data, bytes);
    CHECK (result >= 0);

    result = close(fd);
    CHECK (result >= 0);
}

void destroy_test_file(char const * const path)
{
    int result = unlink(path);
    CHECK (result >= 0);
}

TEST_GROUP(test_ccapi_send_file_no_reply)
{
    static ccapi_send_error_t error;

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

        th_start_tcp_lan_ipv4();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_send_file_no_reply, testSEND_ERROR_NONE)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, data, strlen(data));

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = SEND_WAIT_FOREVER;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, strlen(data)));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testChunkSizeEqual)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    create_test_file(LOCAL_PATH, data, strlen(data));

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = SEND_WAIT_FOREVER;

    mock_info->connector_initiate_send_data_info.in.chunk_size = strlen(data);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, strlen(data)));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testChunkSizeSmall)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    create_test_file(LOCAL_PATH, data, strlen(data));

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = SEND_WAIT_FOREVER;

    mock_info->connector_initiate_send_data_info.in.chunk_size = strlen(data) / 4 - 1; /* Don't allocate enough space so data callback is called several times */

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, strlen(data)));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testChunkSizeSmallBinary)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    #define TEST_SIZE 600
    static uint8_t data[TEST_SIZE];
    unsigned int i;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    for( i=0 ; i < TEST_SIZE ; i++)
        data[i] = i;

    create_test_file(LOCAL_PATH, data, TEST_SIZE);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = SEND_WAIT_FOREVER;

    mock_info->connector_initiate_send_data_info.in.chunk_size = TEST_SIZE / 10 - 3; /* Don't allocate enough space so data callback is called several times */

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, TEST_SIZE));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testSEND_ERROR_STATUS_CANCEL)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, TEST_SIZE);

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_cancel;
    }

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_CANCEL, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testSEND_ERROR_STATUS_TIMEOUT)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, TEST_SIZE);

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_timeout;
    }

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_TIMEOUT, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_no_reply, testSEND_ERROR_STATUS_SESSION_ERROR)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, TEST_SIZE);

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_session_error;
    }

    error = ccapi_send_file(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_SESSION_ERROR, error);

    destroy_test_file(LOCAL_PATH);
}
