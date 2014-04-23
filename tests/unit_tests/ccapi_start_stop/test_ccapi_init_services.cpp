#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_init_services)
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

TEST(test_ccapi_init_services, testServicesNotSupported)
{
    th_start_ccapi();
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_FALSE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_FALSE);
}

TEST(test_ccapi_init_services, testServicesSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_filesystem_service_t fs_service = {NULL, NULL};
    void * pointer = pointer; /* Not-NULL */

    th_fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    start.service.receive = &pointer;
    start.service.firmware = &pointer;
    start.service.rci = &pointer;
    start.service.file_system = &fs_service;

    error = ccapi_start(&start);

    CHECK(error == CCAPI_START_ERROR_NONE);
    CHECK(ccapi_data_single_instance->config.cli_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.receive_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.firmware_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.rci_supported == CCAPI_TRUE);
    CHECK(ccapi_data_single_instance->config.filesystem_supported == CCAPI_TRUE);
    CHECK_EQUAL(fs_service.access_cb, ccapi_data_single_instance->service.file_system.user_callbacks.access_cb);
    CHECK_EQUAL(fs_service.changed_cb, ccapi_data_single_instance->service.file_system.user_callbacks.changed_cb);
    CHECK(NULL == ccapi_data_single_instance->service.file_system.imp_context);

}
