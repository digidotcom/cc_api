#include "test_helper_functions.h"

#define CLOUD_PATH   "test/test.txt"
#define CONTENT_TYPE "text/plain"
#define DATA         "CCAPI send data sample\n"

TEST_GROUP(test_ccapi_send_data_with_reply)
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

TEST(test_ccapi_send_data_with_reply, testTimeoutOkNoHint)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    static char const data[] = DATA;
    static char const cloud_path[] = CLOUD_PATH;
    static char const content_type[] = CONTENT_TYPE;
    unsigned long timeout = 10;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_true;
    header.timeout_in_seconds = timeout;

    mock_info->connector_initiate_send_data_info.in.bytes = strlen(data);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, cloud_path, content_type, data, strlen(data), CCAPI_SEND_BEHAVIOR_OVERWRITE, timeout, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(mock_info->connector_initiate_send_data_info.in.bytes == mock_info->connector_initiate_send_data_info.out.bytes_used);
    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, mock_info->connector_initiate_send_data_info.in.bytes));
    CHECK(mock_info->connector_initiate_send_data_info.out.more_data == connector_false);
}

TEST(test_ccapi_send_data_with_reply, testHint)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    static char const data[] = DATA;
    static char const cloud_path[] = CLOUD_PATH;
    static char const content_type[] = CONTENT_TYPE;
    static char const hint_check[] = "hello man";
    unsigned long timeout = SEND_WAIT_FOREVER;

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_true;
    header.timeout_in_seconds = timeout;

    hint.length = 10;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.bytes = strlen(data);
    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, cloud_path, content_type, data, strlen(data), CCAPI_SEND_BEHAVIOR_OVERWRITE, timeout, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(mock_info->connector_initiate_send_data_info.in.bytes == mock_info->connector_initiate_send_data_info.out.bytes_used);
    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.out.data, mock_info->connector_initiate_send_data_info.in.bytes));
    CHECK(mock_info->connector_initiate_send_data_info.out.more_data == connector_false);

    CHECK(0 == strcmp(hint.string, hint_check));
}

/* This test compared with 'testHint': Forces ccfsm to call response with a hint but the user has not provided a hint buffer */
TEST(test_ccapi_send_data_with_reply, testHintCanBeNull)
{
    ccapi_send_error_t error;

    static char const hint_check[] = "hello man";

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

}

TEST(test_ccapi_send_data_with_reply, testHintNoEnoughtRoom)
{
    ccapi_send_error_t error;

    static char const hint_check[] = "testHintNoEnoughtRoom";

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    hint.length = 10;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(hint.string, hint_check,  hint.length - 1));
    CHECK('\0' == hint.string[hint.length - 1]);
}

TEST(test_ccapi_send_data_with_reply, testHintJustEnoughtRoom)
{
    ccapi_send_error_t error;

    static char const hint_check[] = "testHintJustEnoughtRoom";

    ccapi_string_info_t hint;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    hint.length = strlen(hint_check) + 1;
    hint.string = (char*)malloc(hint.length);

    mock_info->connector_initiate_send_data_info.in.hint = hint_check;

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, &hint);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(0 == memcmp(hint.string, hint_check,  hint.length - 1));
    CHECK('\0' == hint.string[hint.length - 1]);
    CHECK(0 == strcmp(hint.string, hint_check));
}

TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_RESPONSE_BAD_REQUEST)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_bad_request;
    }

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST, error);
}

TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_RESPONSE_UNAVAILABLE)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_unavailable;
    }

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE, error);
}

TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_RESPONSE_CLOUD_ERROR)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_cloud_error;
    }

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR, error);
}

TEST(test_ccapi_send_data_with_reply, testResponseErrorHasPriorityOverStatusError)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.response = connector_data_service_send_response_t::connector_data_service_send_response_cloud_error;
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_timeout;
    }

    error = ccapi_send_data_with_reply(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE, 0, NULL);
    CHECK_EQUAL(CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR, error);
}


TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_STATUS_CANCEL)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_cancel;
    }

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_CANCEL, error);
}

TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_STATUS_TIMEOUT)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_timeout;
    }

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_TIMEOUT, error);
}

TEST(test_ccapi_send_data_with_reply, testSEND_ERROR_STATUS_SESSION_ERROR)
{
    ccapi_send_error_t error;

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.in.status = connector_data_service_status_t::connector_data_service_status_session_error;
    }

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_STATUS_SESSION_ERROR, error);
}