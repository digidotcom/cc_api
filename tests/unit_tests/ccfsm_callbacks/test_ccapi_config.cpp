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

TEST_GROUP(ccapi_config_test_tcp_start_LAN_1)
{
    /* This groups starts with LAN and IPv4, No password */
    void setup()
    {

        Mock_create_all();

        th_start_ccapi();
        th_setup_mock_info_single_instance();
        th_start_tcp_lan_ipv4();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);
        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigConnectionType)
{
    connector_request_id_t request;
    connector_config_connection_type_t connection_type = { connector_connection_type_wan };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_connection_type;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connection_type, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connection_type.type, connector_connection_type_lan);
}

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigMAC)
{
    connector_request_id_t request;
    connector_config_pointer_data_t connector_mac_addr = {NULL, sizeof ccapi_data_single_instance->transport_tcp.info->connection.info.lan.mac_address};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_mac_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_mac_addr, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(0, memcmp(connector_mac_addr.data, ccapi_data_single_instance->transport_tcp.info->connection.info.lan.mac_address, sizeof ccapi_data_single_instance->transport_tcp.info->connection.info.lan.mac_address));
}

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigIPv4)
{
    connector_request_id_t request;
    connector_config_ip_address_t connector_ip_addr = {NULL, connector_ip_address_ipv6};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_ip_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_ip_addr, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_ip_address_ipv4, connector_ip_addr.ip_address_type);
    CHECK_EQUAL(0, memcmp(connector_ip_addr.address, &ccapi_data_single_instance->transport_tcp.info->connection.ip.address.ipv4, sizeof ccapi_data_single_instance->transport_tcp.info->connection.ip.address.ipv4));
}

TEST(ccapi_config_test_tcp_start_LAN_1, testIdVerificationSimple)
{
    connector_request_id_t request;
    connector_config_identity_verification_t connector_id_verification = {connector_identity_verification_password};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_identity_verification;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_id_verification, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_identity_verification_simple, connector_id_verification.type);
}

TEST_GROUP(ccapi_config_test_tcp_start_LAN_2)
{
    /* This groups starts with LAN and IPv6, Password enabled, max transactions = 10 and Keepalives RX=90, TX=100, WC=10 */
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
        th_setup_mock_info_single_instance();
        th_start_tcp_lan_ipv6_password_keepalives();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_LAN_2, testConfigIPv6)
{
    connector_request_id_t request;
    connector_config_ip_address_t connector_ip_addr = {NULL, connector_ip_address_ipv4};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_ip_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_ip_addr, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_ip_address_ipv6, connector_ip_addr.ip_address_type);
    CHECK_EQUAL(0, memcmp(connector_ip_addr.address, &ccapi_data_single_instance->transport_tcp.info->connection.ip.address.ipv6, sizeof ccapi_data_single_instance->transport_tcp.info->connection.ip.address.ipv6));
}

TEST(ccapi_config_test_tcp_start_LAN_2, testIdVerificationPassword)
{
    connector_request_id_t request;
    connector_config_identity_verification_t connector_id_verification = {connector_identity_verification_simple};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_identity_verification;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_id_verification, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_identity_verification_password, connector_id_verification.type);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testPassword)
{
    connector_request_id_t request;
    connector_config_pointer_string_t password = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_password;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &password, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen(ccapi_data_single_instance->transport_tcp.info->connection.password), password.length);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.password, password.string);
    STRCMP_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.password, password.string);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testMaxTransactions)
{
    connector_request_id_t request;
    connector_config_max_transaction_t max_transaction = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_max_transaction;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &max_transaction, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.max_transactions, max_transaction.count);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testRxKeepalives)
{
    connector_request_id_t request;
    connector_config_keepalive_t rx_keepalive = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_rx_keepalive;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &rx_keepalive, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->keepalives.rx, rx_keepalive.interval_in_seconds);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testTxKeepalives)
{
    connector_request_id_t request;
    connector_config_keepalive_t tx_keepalive = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_tx_keepalive;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &tx_keepalive, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->keepalives.tx, tx_keepalive.interval_in_seconds);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testWcKeepalives)
{
    connector_request_id_t request;
    connector_config_wait_count_t wait_count = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_wait_count;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &wait_count, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->keepalives.wait_count, wait_count.count);
}

TEST_GROUP(ccapi_config_test_tcp_start_WAN)
{
    /* This groups starts with WAN, linkspeed = 1000 and phone number != "", password disabled, max transactions = 10 and Keepalives RX=90, TX=100, WC=10 */
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
        th_setup_mock_info_single_instance();
        th_start_tcp_wan_ipv4_with_callbacks();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_WAN, testLinkSpeed)
{
    connector_request_id_t request;
    connector_config_link_speed_t link_speed = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_link_speed;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &link_speed, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.link_speed, link_speed.speed);
}

TEST(ccapi_config_test_tcp_start_WAN, testPhoneNumber)
{
    connector_request_id_t request;
    connector_config_pointer_string_t phone_number = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_phone_number;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &phone_number, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number), phone_number.length);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number, phone_number.string);
    STRCMP_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number, phone_number.string);
}
