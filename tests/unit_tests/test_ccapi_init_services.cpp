#include "test_helper_functions.h"

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

TEST(ccapi_init_services_test, testServicesNotSupported)
{
    th_start_ccapi();
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE);
}

TEST(ccapi_init_services_test, testServicesSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    void * pointer = pointer; /* Not-NULL */

    th_fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    start.service.receive = &pointer;
    start.service.firmware = &pointer;
    start.service.rci = &pointer;
    start.service.file_system = &pointer;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_TRUE);

}
