#include "test_helper_functions.h"

#define STREAM_ID   "stream_id"

TEST_GROUP(test_ccapi_dp_send_collection)
{
    ccapi_dp_collection_handle_t dp_collection;

    void setup()
    {
        ccapi_dp_error_t dp_error;
        Mock_create_all();

        dp_error = ccapi_dp_create_collection(&dp_collection);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

        dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, STREAM_ID, CCAPI_DP_KEY_DATA_INT32);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

        dp_error = ccapi_dp_add(dp_collection, STREAM_ID, 0);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
        dp_error = ccapi_dp_add(dp_collection, STREAM_ID, 1);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
        dp_error = ccapi_dp_add(dp_collection, STREAM_ID, 2);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
        dp_error = ccapi_dp_add(dp_collection, STREAM_ID, 3);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_send_collection, testSendCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_send_collection(NULL, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_CCAPI_NOT_RUNNING, dp_error);

    th_start_ccapi();
    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_TRANSPORT_NOT_STARTED, dp_error);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_UDP);
    CHECK_EQUAL(CCAPI_DP_ERROR_TRANSPORT_NOT_STARTED, dp_error);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_SMS);
    CHECK_EQUAL(CCAPI_DP_ERROR_TRANSPORT_NOT_STARTED, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendEmptyCollection)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_collection_handle_t empty_collection;

    dp_error = ccapi_dp_create_collection(&empty_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_send_collection(empty_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionNoMemory4Transaction)
{
    ccapi_dp_error_t dp_error;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NULL, false);
    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionCreateSyncrFailed)
{
    ccapi_dp_error_t dp_error;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);
    Mock_ccimp_os_syncr_create_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionCCFSMFailure)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.user_context = malloc_for_transaction;
    ccfsm_status.user_context = malloc_for_transaction;

    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_unavailable);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_INITIATE_ACTION_FAILED, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPOk)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_success;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_complete;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPResponseCloudError)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_cloud_error;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_complete;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_RESPONSE_CLOUD_ERROR, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPResponseErrorUnavailable)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_unavailable;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_complete;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_RESPONSE_UNAVAILABLE, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPResponseErrorBadRequest)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_bad_request;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_cancel; /* Just for checking that response error has priority */
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_RESPONSE_BAD_REQUEST, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPStatusCancel)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_success;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_cancel;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_STATUS_CANCEL, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPStatusInvalidData)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_success;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_invalid_data;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_STATUS_INVALID_DATA, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPStatusTimeout)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_success;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_timeout;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_STATUS_TIMEOUT, dp_error);
}

TEST(test_ccapi_dp_send_collection, testSendCollectionTCPStatusSessionError)
{
    ccapi_dp_error_t dp_error;
    connector_request_data_point_multiple_t ccfsm_request;

    th_start_ccapi();
    th_start_tcp_lan_ipv4();

    void * malloc_for_transaction = th_expect_malloc(sizeof (ccapi_dp_transaction_info_t), TH_MALLOC_RETURN_NORMAL, true);

    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(ccapi_data_single_instance->connector_handle);
    connector_data_point_response_t ccfsm_response;
    connector_data_point_status_t ccfsm_status;

    ccfsm_response.hint = NULL;
    ccfsm_response.transport = connector_transport_tcp;
    ccfsm_response.response = connector_data_point_response_t::connector_data_point_response_success;
    ccfsm_response.user_context = malloc_for_transaction;

    ccfsm_status.transport = connector_transport_tcp;
    ccfsm_status.status = connector_data_point_status_t::connector_data_point_status_session_error;
    ccfsm_status.user_context = malloc_for_transaction;
    mock_info->connector_initiate_data_point_multiple.ccfsm_response = &ccfsm_response;
    mock_info->connector_initiate_data_point_multiple.ccfsm_status = &ccfsm_status;

    ccfsm_request.request_id = NULL;
    ccfsm_request.response_required = connector_false;
    ccfsm_request.timeout_in_seconds = OS_SYNCR_ACQUIRE_INFINITE;
    ccfsm_request.transport = connector_transport_tcp;
    ccfsm_request.user_context = malloc_for_transaction;
    ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_data_point_multiple, &ccfsm_request, connector_success);

    dp_error = ccapi_dp_send_collection(dp_collection, CCAPI_TRANSPORT_TCP);
    CHECK_EQUAL(CCAPI_DP_ERROR_STATUS_SESSION_ERROR, dp_error);
}
