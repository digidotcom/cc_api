#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

using namespace std;

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

static void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x12345678; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

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

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);

    free(malloc_for_ccapi_data);
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

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);

    free(malloc_for_ccapi_data);
    free(malloc_for_device_type);
}

TEST(ccapi_init_test, testConnectorInitNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    connector_handle_t handle = NULL;

    Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle, ccapi_data_single_instance);

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
    Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle, ccapi_data_single_instance);
    Mock_connector_run_expectAndReturn(handle ,connector_success);

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    /* expected_create_thread_connector_run.start */
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_NORMAL, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(start.vendor_id == ccapi_data_single_instance->config.vendor_id);
    CHECK(memcmp(start.device_id, ccapi_data_single_instance->config.device_id, sizeof start.device_id) == 0);
    STRCMP_EQUAL(start.device_type, ccapi_data_single_instance->config.device_type);
    STRCMP_EQUAL(start.device_cloud_url, ccapi_data_single_instance->config.device_cloud_url);
    CHECK(ccapi_data_single_instance->thread.connector_run->status == CCAPI_THREAD_RUNNING);

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
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

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_INSUFFICIENT_MEMORY);

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
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

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_DONT_CREATE_THREAD, CCIMP_STATUS_ABORT);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_THREAD_FAILED);

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}

static void * thread_wrapper(void * argument)
{

    ccapi_start((ccapi_start_t *)argument);

    return NULL;
}

pthread_t aux_ccapi_start(void * argument)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, argument);

    if (ccode != 0)
    {
        printf("aux_ccapi_start() error %d\n", ccode);
    }

    return pthread;
}

int stop_aux_thread(pthread_t pthread)
{
    int error;

    error = pthread_cancel(pthread);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }

    error = pthread_join(pthread, NULL);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }
done:
    return error;
}

/* This test sets to null the argument passed to the layer2 run thread.
*/
TEST(ccapi_init_test, testInitError_thread_null_pointer)
{
    ccapi_start_t start = {0};
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;
    ccimp_create_thread_info_t expected_create_thread_connector_run;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);

    /* corrupt the argument created by the handle */

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED3_ARGUMENT_NULL, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    {
        /* call ccapi_start in a sepatare thread as it won't return */
        pthread_t const aux_thread = aux_ccapi_start(&start);

        ASSERT_WAIT(1);
        ASSERT_IF_NOT_HIT_DO ("NULL Pointer on CCIMP_THREAD_CONNECTOR_RUN", FAIL_TEST("NULL Pointer on CCIMP_THREAD_CONNECTOR_RUN not hitted"));

        stop_aux_thread(aux_thread);
    }
    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}

/* This test corrupts the argument passed to the layer2 run thread so ccapi_signature check fails.
*/
TEST(ccapi_init_test, testInitError_bad_ccapi_signature)
{
    ccapi_start_t start = {0};
    void * malloc_for_ccapi_data = malloc(sizeof (ccapi_data_t));
    void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
    void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
    static ccimp_create_thread_info_t mem_for_thread_connector_run;
    ccimp_create_thread_info_t expected_create_thread_connector_run;

    Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_data_t), malloc_for_ccapi_data);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
    Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_thread_info_t), (void*)&mem_for_thread_connector_run);

    /* corrupt the argument created by the handle */

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED2_ARGUMENT_CORRUPT, CCIMP_STATUS_ABORT);

    fill_start_structure_with_good_parameters(&start);
    {
        /* call ccapi_start in a sepatare thread as it won't return */
        pthread_t const aux_thread = aux_ccapi_start(&start);

        ASSERT_WAIT(1);
        ASSERT_IF_NOT_HIT_DO ("Bad ccapi_signature", FAIL_TEST("Bad ccapi_signature not hitted"));

        CHECK(ccapi_data_single_instance->thread.connector_run->status == CCAPI_THREAD_REQUEST_START);
        stop_aux_thread(aux_thread);
    }

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}

/* This test makes layer1 run thread return connector_init_error.
*/
TEST(ccapi_init_test, testInitError_bad_connector_signature)
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
    Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle, ccapi_data_single_instance);
    Mock_connector_run_expectAndReturn(handle, connector_init_error);
    
    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_NORMAL, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    ASSERT_WAIT(01);
    ASSERT_IF_NOT_HIT_DO ("Bad connector_signature", FAIL_TEST("Bad connector_signature not hitted"));

    CHECK(ccapi_data_single_instance->thread.connector_run->status == CCAPI_THREAD_RUNNING);

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}
