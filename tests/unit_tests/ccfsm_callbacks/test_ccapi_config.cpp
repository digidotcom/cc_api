#include "test_helper_functions.h"

TEST_GROUP(ccapi_config_test_basic)
{
    void setup()
    {
        Mock_create_all();
        th_start_ccapi();
        th_setup_mock_info_single_instance();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);
        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_basic, testDeviceID)
{
    connector_request_id_t request;
    connector_config_pointer_data_t device_id = {NULL, sizeof ccapi_data_single_instance->config.device_id};


    request.config_request = connector_request_id_config_device_id;
    ccapi_connector_callback(connector_class_id_config, request, &device_id, ccapi_data_single_instance);
    CHECK_EQUAL(device_id.data, ccapi_data_single_instance->config.device_id);
}

TEST(ccapi_config_test_basic, testCloudURL)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_cloud_url = {0};

    request.config_request = connector_request_id_config_device_cloud_url;
    ccapi_connector_callback(connector_class_id_config, request, &device_cloud_url, ccapi_data_single_instance);
    STRCMP_EQUAL(device_cloud_url.string, ccapi_data_single_instance->config.device_cloud_url);
    CHECK(strlen(ccapi_data_single_instance->config.device_cloud_url) == device_cloud_url.length);
}

TEST(ccapi_config_test_basic, testVendorID)
{
    connector_request_id_t request;
    connector_config_vendor_id_t vendor_id = {0};

    request.config_request = connector_request_id_config_vendor_id;
    ccapi_connector_callback(connector_class_id_config, request, &vendor_id, ccapi_data_single_instance);
    CHECK(vendor_id.id == ccapi_data_single_instance->config.vendor_id);
}

TEST(ccapi_config_test_basic, testDeviceType)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_type = {0};

    request.config_request = connector_request_id_config_device_type;
    ccapi_connector_callback(connector_class_id_config, request, &device_type, ccapi_data_single_instance);
    STRCMP_EQUAL(device_type.string, ccapi_data_single_instance->config.device_type);
    CHECK(strlen(ccapi_data_single_instance->config.device_type) == device_type.length);
}

TEST(ccapi_config_test_basic, testFirmwareSupport)
{
    connector_request_id_t request;
    connector_config_supported_t firmware_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_firmware_facility;
    ccapi_connector_callback(connector_class_id_config, request, &firmware_supported, ccapi_data_single_instance);
    CHECK(firmware_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testFileSystemSupport)
{
    connector_request_id_t request;
    connector_config_supported_t filesystem_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_file_system;
    ccapi_connector_callback(connector_class_id_config, request, &filesystem_supported, ccapi_data_single_instance);
    CHECK(filesystem_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testRCISupport)
{
    connector_request_id_t request;
    connector_config_supported_t rci_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_remote_configuration;
    ccapi_connector_callback(connector_class_id_config, request, &rci_supported, ccapi_data_single_instance);
    CHECK(rci_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testDataServiceSupport)
{
    connector_request_id_t request;
    connector_config_supported_t dataservice_supported = {connector_false}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_data_service;
    ccapi_connector_callback(connector_class_id_config, request, &dataservice_supported, ccapi_data_single_instance);
    CHECK(dataservice_supported.supported == connector_true);
}
