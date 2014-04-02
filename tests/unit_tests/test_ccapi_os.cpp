#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_os_test)
{
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;

        Mock_create_all();

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(ccapi_os_test, testMalloc)
{
    connector_request_id_t request;
    connector_os_malloc_t malloc_structure = {1024, NULL};
    connector_callback_status_t status;
    void * pre_allocated_buffer = malloc(1024);

    Mock_ccimp_malloc_expectAndReturn(malloc_structure.size, pre_allocated_buffer);

    request.os_request = connector_request_id_os_malloc;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &malloc_structure, *spy_ccapi_data);

    CHECK(status == connector_callback_continue);
    CHECK(malloc_structure.ptr == pre_allocated_buffer);
}

TEST(ccapi_os_test, testFreeOk)
{
    connector_request_id_t request;
    void * pre_allocated_buffer = malloc(1024);
    connector_os_free_t free_structure = {pre_allocated_buffer};
    connector_callback_status_t status;

    Mock_ccimp_free_expectAndReturn(pre_allocated_buffer, CCIMP_STATUS_OK);

    request.os_request = connector_request_id_os_free;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &free_structure, *spy_ccapi_data);
    CHECK(status == connector_callback_continue);
}

TEST(ccapi_os_test, testFreeAbort)
{
    connector_request_id_t request;
    void * pre_allocated_buffer = malloc(1024);
    connector_os_free_t free_structure = {pre_allocated_buffer};
    connector_callback_status_t status;

    Mock_ccimp_free_expectAndReturn(pre_allocated_buffer, CCIMP_STATUS_ABORT);

    request.os_request = connector_request_id_os_free;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &free_structure, *spy_ccapi_data);
    CHECK(status == connector_callback_abort);
}

TEST(ccapi_os_test, testYield)
{
    /* Trying to mock ccimp_os_yield() was a complete failure, the function is being called from connector_thread()
     * making it quite difficult to check expectations */
    connector_request_id_t request;
    connector_callback_status_t status;

    request.os_request = connector_request_id_os_yield;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, NULL, *spy_ccapi_data);
    CHECK(status == connector_callback_continue);
}

TEST(ccapi_os_test, testSystemUptime)
{
    connector_request_id_t request;
    connector_os_system_up_time_t uptime;
    connector_callback_status_t status;

    Mock_ccimp_os_get_system_time_return(0);
    request.os_request = connector_request_id_os_system_up_time;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &uptime, *spy_ccapi_data);
    CHECK(status == connector_callback_continue);
}

TEST(ccapi_os_test, testReboot)
{
    /* This is not entirely OK, reboot would never return */
    connector_request_id_t request;
    connector_callback_status_t status;

    request.os_request = connector_request_id_os_reboot;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, NULL, *spy_ccapi_data);
    CHECK(status == connector_callback_continue);
}

TEST(ccapi_os_test, testRealloc)
{
    /* This is not entirely OK, reboot would never return */
    connector_request_id_t request;
    connector_callback_status_t status;
    void * pointer_to_be_reallocated = malloc(512);
    connector_os_realloc_t realloc_info = {512, 1024, pointer_to_be_reallocated};
    uint8_t aux_buffer[1024] = {'A'};

    memcpy(realloc_info.ptr, aux_buffer, 512);
    request.os_request = connector_request_id_os_realloc;
    status = ccapi_connector_callback(connector_class_id_operating_system, request, &realloc_info, *spy_ccapi_data);
    CHECK(status == connector_callback_continue);
    memcpy(realloc_info.ptr, aux_buffer, 1024); /* If realloc is not done, this would throw a SIGSEV */
}
