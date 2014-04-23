#include "test_helper_functions.h"

#define CLOUD_PATH   "test/test.txt"
#define CONTENT_TYPE "text/plain"
#define DATA         "CCAPI send data sample\n"

/* This group doesn't call ccapi_start/stop functions */
TEST_GROUP(ccapi_send_with_no_ccapi) 
{
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(ccapi_send_with_no_ccapi, testCcapiNotStarted)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING, error);
}

/* This group doesn't call ccapi_transport_tcp_start function */
TEST_GROUP(ccapi_send_with_no_transport_start) 
{
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(ccapi_send_with_no_transport_start, testTransportNotStarted)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED, error);
}

TEST_GROUP(test_ccapi_send_common_sanity_checks)
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

TEST(test_ccapi_send_common_sanity_checks, testInvalidCloudPath)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, NULL, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_INVALID_CLOUD_PATH, error);
}

TEST(test_ccapi_send_common_sanity_checks, testNullContentTypeIsValid)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, NULL, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);
}

TEST_GROUP(test_ccapi_send_data_sanity_checks)
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

TEST(test_ccapi_send_data_sanity_checks, testInvalidData)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, NULL, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_INVALID_DATA, error);
}

TEST(test_ccapi_send_data_sanity_checks, testInvalidBytes)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, 0, CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_INVALID_DATA, error);
}

TEST(test_ccapi_send_data_sanity_checks, testOK)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_TCP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_NONE, error);
}

TEST(test_ccapi_send_data_sanity_checks, testUdpTransportNotStarted)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_UDP, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED, error);
}

TEST(test_ccapi_send_data_sanity_checks, testSmsTransportNotStarted)
{
    ccapi_send_error_t error;

    error = ccapi_send_data(CCAPI_TRANSPORT_SMS, CLOUD_PATH, CONTENT_TYPE, DATA, strlen(DATA), CCAPI_SEND_BEHAVIOR_OVERWRITE);
    CHECK_EQUAL(CCAPI_SEND_ERROR_TRANSPORT_NOT_STARTED, error);
}
