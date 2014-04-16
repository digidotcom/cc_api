#include "test_helper_functions.h"

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

TEST(ccapi_send_with_no_ccapi, testNotStarted)
{
    ccapi_send_error_t error;
    ccapi_transport_t transport = CCAPI_TRANSPORT_TCP;
    char const cloud_path[] = "test/test.txt";
    char const content_type[] = "text/plain";
    char const data[] = "CCAPI send data sample\n";
    ccapi_send_behavior_t behavior = CCAPI_SEND_OVERWRITE;

    error = ccapi_send_data(transport, cloud_path, content_type, data, strlen(data), behavior);
    CHECK_EQUAL(CCAPI_SEND_ERROR_CCAPI_NOT_RUNNING, error);
}

