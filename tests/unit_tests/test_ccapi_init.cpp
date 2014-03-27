#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccapi/ccapi.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_init_test)
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

TEST(ccapi_init_test, testParamNULL)
{
    ccapi_start_error_t error;
    ccapi_start_t * start = NULL;

    error = ccapi_start(start);
    CHECK(error == CCAPI_START_ERROR_NULL_PARAMETER);
}

TEST(ccapi_init_test, testVendorIdZero)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.vendor_id = 0;
    error = ccapi_start(&start);

    CHECK_EQUAL(error, CCAPI_START_ERROR_INVALID_VENDORID);
}

TEST(ccapi_init_test, testInvalidDeviceId)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0};
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    memcpy(start.device_id, device_id, sizeof start.device_id);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_DEVICEID);
}

TEST(ccapi_init_test, testNullDeviceCloudURL)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_cloud_url = NULL;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_URL);
}

TEST(ccapi_init_test, testInvalidDeviceCloudURL)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_cloud_url = "";
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_URL);
}

TEST(ccapi_init_test, testNullDeviceType)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_type = NULL;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_DEVICETYPE);
}

TEST(ccapi_init_test, testInvalidDeviceType)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_type = "";
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INVALID_DEVICETYPE);
}

TEST(ccapi_init_test, testNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data = NULL;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_free_notExpected();
    fill_start_structure_with_good_parameters(&start);

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testDeviceTypeNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = NULL;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);

    /* No need to call free on malloc_for_device_type because the malloc failed */
    Mock_ccimp_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testDeviceCloudURLNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = NULL;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);

    /* No need to call free on malloc_for_device_cloud_url because the malloc failed */
    Mock_ccimp_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testConnectorInitNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    connector_handle_t handle = NULL;

    Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle, (*spy_ccapi_data));

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testStartOk)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;
    ccimp_create_thread_info_t expected_create_thread_connector_run;

    connector_handle_t handle = &handle; /* Not-NULL */

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);
    Mock_ccimp_free_notExpected();

    Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle, (*spy_ccapi_data));
    Mock_connector_run_returnInNextLoop(connector_success);

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    /* expected_create_thread_connector_run.start */
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_NORMAL, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(start.vendor_id == (*spy_ccapi_data)->config.vendor_id);
    CHECK(memcmp(start.device_id, (*spy_ccapi_data)->config.device_id, sizeof start.device_id) == 0);
    STRCMP_EQUAL(start.device_type, (*spy_ccapi_data)->config.device_type);
    STRCMP_EQUAL(start.device_cloud_url, (*spy_ccapi_data)->config.device_cloud_url);
    CHECK((*spy_ccapi_data)->thread.connector_run->status == CCAPI_THREAD_RUNNING);
}

TEST(ccapi_init_test, testStartThreadNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data= malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    void * mem_for_thread_connector_run = NULL;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), mem_for_thread_connector_run);

    /* No need to call free on mem_for_thread_connector_run because the malloc failed */
    Mock_ccimp_free_expectAndReturn(malloc_for_device_cloud_url, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testStartThreadFail)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * malloc_for_ccapi_data= malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;
    ccimp_create_thread_info_t expected_create_thread_connector_run;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);

    Mock_ccimp_free_expectAndReturn(&mem_for_thread_connector_run, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_device_cloud_url, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_device_type, CCIMP_STATUS_OK);
    Mock_ccimp_free_expectAndReturn(malloc_for_ccapi_data, CCIMP_STATUS_OK);

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_DONT_CREATE_THREAD, CCIMP_STATUS_ABORT);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_THREAD_FAILED);
}

TEST(ccapi_init_test, testStartTwiceFails)
{
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    fill_start_structure_with_good_parameters(&start);

    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_NONE);

    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_ALREADY_STARTED);
}
