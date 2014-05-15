#include "test_helper_functions.h"

#define TEST_TARGET "my_target"
#define DATA  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, \
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }

static char const * ccapi_receive_data_expected_target = NULL;
static ccapi_transport_t ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
static ccapi_bool_t ccapi_receive_data_cb_called = CCAPI_FALSE;
static ccapi_bool_t ccapi_receive_data_expected_request_not_null = CCAPI_FALSE;

static ccapi_buffer_info_t * ccapi_receive_data_expected_request = NULL;
static ccapi_buffer_info_t * ccapi_receive_data_expected_response = NULL;
static ccapi_receive_error_t ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;


static void test_receive_data_cb(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error)
{
    STRCMP_EQUAL(ccapi_receive_data_expected_target, target);
    CHECK_EQUAL(ccapi_receive_data_expected_transport, transport);
    CHECK_EQUAL(ccapi_receive_data_expected_request, request_buffer_info);
    CHECK_EQUAL(ccapi_receive_data_expected_response, response_buffer_info);
    CHECK_EQUAL(ccapi_receive_data_expected_receive_error, receive_error);

    ccapi_receive_data_cb_called = CCAPI_TRUE;

    return;
}

TEST_GROUP(test_ccapi_receive_data_callback_NoReceiveSupport)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);

        ccapi_receive_data_expected_target = NULL;
        ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
        ccapi_receive_data_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback_NoReceiveSupport, testNoReceiveSupport)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    CHECK(ccfsm_receive_data_data.user_context != NULL);
    {
        ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)ccfsm_receive_data_data.user_context;
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NO_RECEIVE_SUPPORT);
    }

    CHECK_EQUAL(CCAPI_FALSE, ccapi_receive_data_cb_called);
}

TEST_GROUP(test_ccapi_receive_data_callback_MissingDataCallback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_receive_service_t receive_service = {NULL, NULL, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.receive = &receive_service;

        ccapi_receive_data_expected_target = NULL;
        ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
        ccapi_receive_data_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_INVALID_RECEIVE_CALLBACKS);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback_MissingDataCallback, testWeNotAllowNullDataCallback)
{
}

TEST_GROUP(test_ccapi_receive_data_callback)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_receive_service_t receive_service = {NULL, test_receive_data_cb, NULL};
        Mock_create_all();

        th_fill_start_structure_with_good_parameters(&start);
        start.service.receive = &receive_service;

        ccapi_receive_data_expected_target = NULL;
        ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
        ccapi_receive_data_expected_request = NULL;
        ccapi_receive_data_expected_response = NULL;
        ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;
        ccapi_receive_data_cb_called = CCAPI_FALSE;

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
        CHECK_EQUAL(receive_service.data_cb, ccapi_data_single_instance->service.receive.user_callbacks.data_cb);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_receive_data_callback, testUserContextNull)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = NULL;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_error, status);

    CHECK(ccfsm_receive_data_data.user_context == NULL);

    CHECK_EQUAL(CCAPI_FALSE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_BufferNULL)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.response_required = CCAPI_TRUE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request_not_null = CCAPI_TRUE;
    ccapi_receive_data_expected_request = &svc_receive.request_buffer_info;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = NULL;
    ccfsm_receive_data_data.bytes_used = 0;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        ccapi_svc_receive_t * svc_receive = (ccapi_svc_receive_t *)ccfsm_receive_data_data.user_context;
        CHECK_EQUAL(svc_receive->receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive->request_buffer_info.buffer != NULL); /* Should be? */
        CHECK(svc_receive->request_buffer_info.length == 0);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_OneCall)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.response_required = CCAPI_TRUE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request_not_null = CCAPI_TRUE;
    ccapi_receive_data_expected_request = &svc_receive.request_buffer_info;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);
        CHECK(memcmp(svc_receive.request_buffer_info.buffer, data, sizeof data) == 0);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_TwoCall)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.response_required = CCAPI_TRUE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request_not_null = CCAPI_TRUE;
    ccapi_receive_data_expected_request = &svc_receive.request_buffer_info;
    ccapi_receive_data_expected_response = &svc_receive.response_buffer_info;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_true;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);

    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == 2 * sizeof data);
        CHECK(memcmp(svc_receive.request_buffer_info.buffer, data, sizeof data) == 0);
        CHECK(memcmp(((uint8_t*)svc_receive.request_buffer_info.buffer) + sizeof data, data, sizeof data) == 0);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}

TEST(test_ccapi_receive_data_callback, testOK_ResponseNotRequired)
{
    connector_request_id_t request;
    connector_data_service_receive_data_t ccfsm_receive_data_data;
    connector_callback_status_t status;

    uint8_t const data[] = DATA;

    ccapi_svc_receive_t svc_receive = {0};
    const char target[] = TEST_TARGET;
    svc_receive.target = (char*)target;
    svc_receive.response_required = CCAPI_FALSE;
    
    ccapi_receive_data_expected_target = TEST_TARGET;
    ccapi_receive_data_expected_transport = CCAPI_TRANSPORT_TCP;
    ccapi_receive_data_expected_request_not_null = CCAPI_TRUE;
    ccapi_receive_data_expected_request = &svc_receive.request_buffer_info;
    ccapi_receive_data_expected_response = NULL;
    ccapi_receive_data_expected_receive_error = CCAPI_RECEIVE_ERROR_NONE;

    ccfsm_receive_data_data.transport = connector_transport_tcp;
    ccfsm_receive_data_data.user_context = &svc_receive;
    ccfsm_receive_data_data.buffer = data;
    ccfsm_receive_data_data.bytes_used = sizeof data;
    ccfsm_receive_data_data.more_data = connector_false;

    request.data_service_request = connector_request_id_data_service_receive_data;
    status = ccapi_connector_callback(connector_class_id_data_service, request, &ccfsm_receive_data_data, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, status);

    CHECK(ccfsm_receive_data_data.user_context == &svc_receive);
    {
        CHECK_EQUAL(svc_receive.receive_error, CCAPI_RECEIVE_ERROR_NONE);
        CHECK(svc_receive.request_buffer_info.buffer != NULL);
        CHECK(svc_receive.request_buffer_info.length == sizeof data);
        CHECK(memcmp(svc_receive.request_buffer_info.buffer, data, sizeof data) == 0);
    }

    CHECK_EQUAL(CCAPI_TRUE, ccapi_receive_data_cb_called);
}