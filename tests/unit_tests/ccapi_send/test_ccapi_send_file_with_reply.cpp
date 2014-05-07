#include "test_helper_functions.h"

#define CLOUD_PATH   "test/test.txt"
#define CONTENT_TYPE "text/plain"
#define DATA         "CCAPI send data sample\n"
#define LOCAL_PATH   "./send_file_with_reply.txt"

TEST_GROUP(test_ccapi_send_file_with_reply)
{
    static ccapi_send_error_t error;

    void setup()
    {
        Mock_create_all();

        th_start_ccapi();

        th_start_tcp_lan_ipv4();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_send_file_with_reply, testTimeoutOkNoHint)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;
    unsigned long timeout = 10;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, data, strlen(data));

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_true;
    header.timeout_in_seconds = timeout;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE, timeout, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, strlen(data)));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testHint)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;
    char const hint_check[] = "hello man";
    unsigned long timeout = SEND_WAIT_FOREVER;

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, data, strlen(data));

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_true;
    header.timeout_in_seconds = timeout;

    hint.length = 10;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE, timeout, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, strlen(data)));

    CHECK(0 == strcmp(hint.string, hint_check));

    destroy_test_file(LOCAL_PATH);
}

/* This test compared with 'testHint': Forces ccfsm to call response with a hint but the user has not provided a hint buffer */
TEST(test_ccapi_send_file_with_reply, testHintCanBeNull)
{
    ccapi_send_error_t error;

    char const hint_check[] = "hello man";

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testHintNoEnoughtRoom)
{
    ccapi_send_error_t error;

    char const hint_check[] = "testHintNoEnoughtRoom";

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    hint.length = 10;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(hint.string, hint_check,  hint.length - 1));
    CHECK('\0' == hint.string[hint.length - 1]);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testHintJustEnoughtRoom)
{
    ccapi_send_error_t error;

    char const hint_check[] = "testHintJustEnoughtRoom";

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    hint.length = strlen(hint_check) + 1;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(hint.string, hint_check,  hint.length - 1));
    CHECK('\0' == hint.string[hint.length - 1]);
    CHECK(0 == strcmp(hint.string, hint_check));

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_RESPONSE_BAD_REQUEST)
{
    ccapi_send_error_t error;


    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_bad_request;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_RESPONSE_UNAVAILABLE)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_unavailable;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_RESPONSE_CLOUD_ERROR)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_cloud_error;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testResponseErrorHasPriorityOverStatusError)
{
    ccapi_send_error_t error;


    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_cloud_error;
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_timeout;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR, error);

    destroy_test_file(LOCAL_PATH);
}


TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_STATUS_CANCEL)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_cancel;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_CANCEL, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_STATUS_TIMEOUT)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_timeout;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_TIMEOUT, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testSEND_ERROR_STATUS_SESSION_ERROR)
{
    ccapi_send_error_t error;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_session_error;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, CLOUD_PATH, CONTENT_TYPE, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_SESSION_ERROR, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testOpenSEND_ERROR_ACCESSING_FILE)
{
    ccapi_send_error_t error;

    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    create_test_file(LOCAL_PATH, DATA, strlen(DATA));

    {
        ccimp_fs_file_open_t ccimp_open_data;

        ccimp_open_data.errnum.pointer = NULL;
        ccimp_open_data.imp_context = NULL;
        ccimp_open_data.handle.pointer = NULL;
        ccimp_open_data.flags = CCIMP_FILE_O_RDONLY;
        ccimp_open_data.path = LOCAL_PATH;

        Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_ERROR);

        if (ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE)
            ccapi_data_single_instance->service.file_system.imp_context = NULL;
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_ACCESSING_FILE, error);

    destroy_test_file(LOCAL_PATH);
}

TEST(test_ccapi_send_file_with_reply, testReadSEND_ERROR_ACCESSING_FILE)
{
    ccapi_send_error_t error;

    char const data[] = DATA;
    char const cloud_path[] = CLOUD_PATH;
    char const content_type[] = CONTENT_TYPE;

    create_test_file(LOCAL_PATH, data, strlen(data));

    {
        connector_request_data_service_send_t header;

        header.transport = connector_transport_tcp;
        header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
        header.path  = cloud_path;
        header.content_type = content_type;
        header.response_required = connector_true;
        header.timeout_in_seconds = SEND_WAIT_FOREVER;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

        if (ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE)
            ccapi_data_single_instance->service.file_system.imp_context = NULL;
    }

    {
        ccimp_fs_file_open_t ccimp_open_data;

        ccimp_open_data.errnum.pointer = NULL;
        ccimp_open_data.imp_context = ccapi_data_single_instance->service.file_system.imp_context;
        ccimp_open_data.handle.pointer = NULL;
        ccimp_open_data.flags = CCIMP_FILE_O_RDONLY;
        ccimp_open_data.path = LOCAL_PATH;

        Mock_ccimp_fs_file_open_expectAndReturn(&ccimp_open_data, CCIMP_STATUS_OK);
    }

    {
        ccimp_fs_file_read_t ccimp_read_data;

        ccimp_read_data.errnum.pointer = NULL;
        ccimp_read_data.imp_context = &my_fs_context;
        ccimp_read_data.handle.value = 5;
        ccimp_read_data.bytes_available = strlen(data);
        ccimp_read_data.bytes_used = 0;

        Mock_ccimp_fs_file_read_expectAndReturn(&ccimp_read_data, CCIMP_STATUS_ERROR);
    }

    error = ccapi_send_file_with_reply(CCAPI_TRANSPORT_TCP, LOCAL_PATH, cloud_path, content_type, CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_ACCESSING_FILE, error);

    destroy_test_file(LOCAL_PATH);
}
