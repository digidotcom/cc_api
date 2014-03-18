#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

#include "test_helper_functions.h"

using namespace std;

TEST_GROUP(ccapi_init_services_test)
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

TEST(ccapi_init_services_test, testCliNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testCliSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testReceiveSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.receive = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testReceiveNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testFirmwareSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testFirmwareNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testRciSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.rci = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testRciNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testFileSysSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.file_system = &pointer;
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_TRUE);
}

TEST(ccapi_init_services_test, testFileSysNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);

    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE);
}
