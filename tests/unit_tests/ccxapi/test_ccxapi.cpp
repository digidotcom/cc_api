#include "test_helper_functions.h"

using namespace std;

static const uint8_t device_id1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
static const uint8_t device_id2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0x44};

TEST_GROUP(test_ccxapi)
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

TEST(test_ccxapi, testNULLHandle)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    th_fill_start_structure_with_good_parameters(&start);
    error = ccxapi_start(NULL, &start);

    CHECK_EQUAL(error, CCAPI_START_ERROR_NULL_PARAMETER);
}

TEST(test_ccxapi, testStartTwiceSameHandlerFails)
{
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};
    ccapi_handle_t ccapi_handle = NULL;

    th_fill_start_structure_with_good_parameters(&start);

    start_error = ccxapi_start(&ccapi_handle, &start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_NONE);

    start_error = ccxapi_start(&ccapi_handle, &start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_ALREADY_STARTED);
}

TEST(test_ccxapi, testStartOneInstance)
{
    ccapi_start_t start1 = {0};
    ccapi_start_error_t start1_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop1_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle1 = NULL;

    th_fill_start_structure_with_good_parameters(&start1);

    CHECK(ccapi_data_single_instance == NULL);
    start1_error = ccxapi_start(&ccapi_handle1, &start1);
    CHECK(start1_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle1 != NULL);
    CHECK(ccapi_data_single_instance == NULL);

    Mock_connector_initiate_action_expectAndReturn(((ccapi_data_t *)ccapi_handle1)->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop1_error = ccxapi_stop(ccapi_handle1, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop1_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}

TEST(test_ccxapi, testStartTwoInstances)
{
    ccapi_start_t start1 = {0};
    ccapi_start_error_t start1_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop1_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle1 = NULL;

    ccapi_start_t start2 = {0};
    ccapi_start_error_t start2_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop2_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle2 = NULL;

    th_fill_start_structure_with_good_parameters(&start1);
    memcpy(start1.device_id, device_id1, sizeof start1.device_id);

    CHECK(ccapi_data_single_instance == NULL);

    start1_error = ccxapi_start(&ccapi_handle1, &start1);
    CHECK(start1_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle1 != NULL);
    CHECK(ccapi_data_single_instance == NULL);

    th_fill_start_structure_with_good_parameters(&start2);
    memcpy(start2.device_id, device_id2, sizeof start2.device_id);

    CHECK(ccapi_data_single_instance == NULL);
    start2_error = ccxapi_start(&ccapi_handle2, &start2);
    CHECK(start2_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle2 != NULL);
    CHECK(ccapi_data_single_instance == NULL);

    CHECK(ccapi_handle1 != ccapi_handle2);

    Mock_connector_initiate_action_expectAndReturn(((ccapi_data_t *)ccapi_handle1)->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop1_error = ccxapi_stop(ccapi_handle1, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop1_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);

    Mock_connector_initiate_action_expectAndReturn(((ccapi_data_t *)ccapi_handle2)->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop2_error = ccxapi_stop(ccapi_handle2, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop2_error == CCAPI_STOP_ERROR_NONE);
    CHECK(ccapi_data_single_instance == NULL);
}
