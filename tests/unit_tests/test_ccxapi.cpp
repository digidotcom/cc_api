#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include <unistd.h>

#include "mocks/mocks.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccxapi/ccxapi.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

extern mock_connector_api_info_t mock_info[MAX_INFO];

uint8_t device_id1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
uint8_t device_id2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0x44};

TEST_GROUP(ccxapi_test)
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


TEST(ccxapi_test, testStartOneInstance)
{
    ccapi_start_t start1 = {0};
    ccapi_start_error_t start1_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop1_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle1 = NULL;
    mock_connector_api_info_t * mock_info1 = NULL; 

    ccapi_data_t * ccapi_data = NULL;

    mock_info1 = alloc_mock_connector_api_info();
    CHECK(mock_info1 != NULL);

    fill_start_structure_with_good_parameters(&start1);

    CHECK(*spy_ccapi_data == NULL);
    start1_error = ccxapi_start(&ccapi_handle1, &start1);
    CHECK(start1_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle1 != NULL);
    CHECK(*spy_ccapi_data == NULL);

    /* sleep(5); */

    ccapi_data = (ccapi_data_t *)ccapi_handle1;
    mock_info1->ccapi_handle = ccapi_handle1;
    mock_info1->connector_handle = ccapi_data->connector_handle;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop1_error = ccxapi_stop(ccapi_handle1, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop1_error == CCAPI_STOP_ERROR_NONE);

    CHECK(*spy_ccapi_data == NULL);

    free_mock_connector_api_info(mock_info);
}

TEST(ccxapi_test, testStartTwoInstances)
{
    ccapi_data_t * ccapi_data = NULL;

    ccapi_start_t start1 = {0};
    ccapi_start_error_t start1_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop1_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle1 = NULL;
    mock_connector_api_info_t * mock_info1 = NULL; 

    ccapi_start_t start2 = {0};
    ccapi_start_error_t start2_error = CCAPI_START_ERROR_NONE;
    ccapi_stop_error_t stop2_error = CCAPI_STOP_ERROR_NONE;
    ccapi_handle_t ccapi_handle2 = NULL;
    mock_connector_api_info_t * mock_info2 = NULL; 

    mock_info1 = alloc_mock_connector_api_info();
    CHECK(mock_info1 != NULL);

    fill_start_structure_with_good_parameters(&start1);
    memcpy(start1.device_id, device_id1, sizeof start1.device_id);

    CHECK(*spy_ccapi_data == NULL);
    start1_error = ccxapi_start(&ccapi_handle1, &start1);
    CHECK(start1_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle1 != NULL);
    CHECK(*spy_ccapi_data == NULL);

    mock_info2 = alloc_mock_connector_api_info();
    CHECK(mock_info2 != NULL);

    fill_start_structure_with_good_parameters(&start2);
    memcpy(start2.device_id, device_id2, sizeof start2.device_id);

    CHECK(*spy_ccapi_data == NULL);
    start2_error = ccxapi_start(&ccapi_handle2, &start2);
    CHECK(start2_error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_handle2 != NULL);
    CHECK(*spy_ccapi_data == NULL);

    CHECK(ccapi_handle1 != ccapi_handle2);

    /* sleep(5); */

    ccapi_data = (ccapi_data_t *)ccapi_handle1;
    mock_info1->ccapi_handle = ccapi_handle1;
    mock_info1->connector_handle = ccapi_data->connector_handle;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop1_error = ccxapi_stop(ccapi_handle1, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop1_error == CCAPI_STOP_ERROR_NONE);
    CHECK(*spy_ccapi_data == NULL);

    ccapi_data = (ccapi_data_t *)ccapi_handle2;
    mock_info2->ccapi_handle = ccapi_handle2;
    mock_info2->connector_handle = ccapi_data->connector_handle;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop2_error = ccxapi_stop(ccapi_handle2, CCAPI_STOP_IMMEDIATELY);
    CHECK(stop2_error == CCAPI_STOP_ERROR_NONE);
    CHECK(*spy_ccapi_data == NULL);

    free_mock_connector_api_info(mock_info1);
    free_mock_connector_api_info(mock_info2);
}
