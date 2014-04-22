#include "test_helper_functions.h"

#define CLOUD_PATH   "test/test.txt"
#define CONTENT_TYPE "text/plain"
#define DATA         "CCAPI send data sample\n"

TEST_GROUP(test_ccapi_send_data_no_reply)
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

TEST(test_ccapi_send_data_no_reply, testSEND_ERROR_NONE)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    static char const data[] = DATA;
    static char const cloud_path[] = CLOUD_PATH;
    static char const content_type[] = CONTENT_TYPE;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = 0; /* TODO: SEND_WAIT_FOREVER;*/

    mock_info->connector_initiate_send_data_info.bytes = strlen(data);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, cloud_path, content_type, data, strlen(data), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(mock_info->connector_initiate_send_data_info.bytes == mock_info->connector_initiate_send_data_info.bytes_used);
    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.data, mock_info->connector_initiate_send_data_info.bytes));
    CHECK(mock_info->connector_initiate_send_data_info.more_data == connector_false);
}

TEST(test_ccapi_send_data_no_reply, testMoreData)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    static char const data[] = DATA;
    static char const cloud_path[] = CLOUD_PATH;
    static char const content_type[] = CONTENT_TYPE;

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = 0; /* TODO: SEND_WAIT_FOREVER;*/

    mock_info->connector_initiate_send_data_info.bytes = strlen(data) - 10; /* Don't allocate enough space so more_data remain true */

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, cloud_path, content_type, data, strlen(data), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);

    CHECK(mock_info->connector_initiate_send_data_info.bytes == mock_info->connector_initiate_send_data_info.bytes_used);
    CHECK(0 == memcmp(data, mock_info->connector_initiate_send_data_info.data, mock_info->connector_initiate_send_data_info.bytes));
    CHECK(mock_info->connector_initiate_send_data_info.more_data == connector_true);
}

TEST(test_ccapi_send_data_no_reply, testSEND_ERROR_CCFSM_ERROR)
{
    ccapi_send_error_t error;

    connector_request_data_service_send_t header;
    static char const data[] = DATA;
    static char const cloud_path[] = CLOUD_PATH;
    static char const content_type[] = CONTENT_TYPE;

    header.transport = connector_transport_tcp;
    header.option = connector_request_data_service_send_t::connector_data_service_send_option_overwrite;
    header.path  = cloud_path;
    header.content_type = content_type;
    header.response_required = connector_false;
    header.timeout_in_seconds = 0; /* TODO: SEND_WAIT_FOREVER;*/

    {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
        mock_info->connector_initiate_send_data_info.status = connector_data_service_status_t::connector_data_service_status_session_error;
    }

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_send_data, &header, connector_success);

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, cloud_path, content_type, data, strlen(data), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_CCFSM_ERROR, error);
}
