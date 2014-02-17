#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"
#include "mocks/mock_ccimp_os.h"
#include "mocks/mock_connector_api.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "internal/ccapi_definitions.h"
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

TEST_GROUP(ccapi_init_services_test)
{
    void setup()
    {
        void * malloc_for_ccapi_config = malloc(sizeof (ccapi_config_t));
        void * malloc_for_device_type = malloc(sizeof DEVICE_TYPE_STRING);
        void * malloc_for_device_cloud_url = malloc(sizeof DEVICE_CLOUD_URL_STRING);
        connector_handle_t handle = &handle; /* Not-NULL */

        Mock_ccimp_malloc_expectAndReturn(sizeof(ccapi_config_t), malloc_for_ccapi_config);
        Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_TYPE_STRING), malloc_for_device_type);
        Mock_ccimp_malloc_expectAndReturn(sizeof(DEVICE_CLOUD_URL_STRING), malloc_for_device_cloud_url);
        Mock_connector_init_expectAndReturn(ccapi_connector_callback, handle);
        Mock_ccimp_create_thread_expectAndReturn(NULL, CCAPI_TRUE);
    }

    void teardown()
    {
        Mock_ccimp_malloc_destroy();
    }
};

TEST(ccapi_init_services_test, testCliNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->cli_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testCliSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->cli_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testReceiveSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.receive = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->receive_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testReceiveNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->receive_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testFirmwareSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->firmware_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testFirmwareNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->firmware_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testRciSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.rci = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->rci_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testRciNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->rci_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testFileSysSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.file_system = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->filesystem_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testFileSysNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    CHECK(ccapi_config->filesystem_supported == CCAPI_FALSE);
}
