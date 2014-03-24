#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_init_threading_test)
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

TEST(ccapi_init_threading_test, testInitErrorThreadNullPointer)
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
    Mock_ccimp_free_notExpected();

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
TEST(ccapi_init_threading_test, testInitErrorBadCcapiSignature)
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
    Mock_ccimp_free_notExpected();

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

        CHECK((*spy_ccapi_data)->thread.connector_run->status == CCAPI_THREAD_REQUEST_START);
        stop_aux_thread(aux_thread);
    }

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}

/* This test makes layer1 run thread return connector_init_error.
*/
TEST(ccapi_init_threading_test, testInitErrorBadConnectorSignature)
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
    Mock_connector_run_returnInNextLoop(connector_init_error);

    expected_create_thread_connector_run.argument = malloc_for_ccapi_data;
    expected_create_thread_connector_run.type = CCIMP_THREAD_CONNECTOR_RUN;
    Mock_ccimp_create_thread_expectAndReturn(&expected_create_thread_connector_run, MOCK_THREAD_ENABLED_NORMAL, CCIMP_STATUS_OK);

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    ASSERT_WAIT(1);
    ASSERT_IF_NOT_HIT_DO ("Bad connector_signature", FAIL_TEST("Bad connector_signature not hitted"));

    CHECK((*spy_ccapi_data)->thread.connector_run->status == CCAPI_THREAD_RUNNING);

    free(malloc_for_device_cloud_url);
    free(malloc_for_device_type);
    free(malloc_for_ccapi_data);
}
